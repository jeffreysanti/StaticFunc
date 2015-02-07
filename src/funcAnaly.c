/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  funcAnaly.c
 *  Function Semantic Analyzer
 *
 */

#include "funcAnaly.h"

bool semAnalyExpr(PTree *root, Type expect, bool silent)
{
	if(root->typ == PTT_INT){
		if(integralType(expect) || floatingType(expect)){
			return true;
		}
		if(!silent){
			errShowType("EXPECTED: ",&expect);
			errShowTypeStr("FOUND: ","Numeric");
		}
		return false;
	}
	if(root->typ == PTT_FLOAT){
		if(floatingType(expect)){
			return true;
		}
		if(!silent){
			errShowType("EXPECTED: ",&expect);
			errShowTypeStr("FOUND: ","Float");
		}
		return false;
	}
	if(root->typ == PTT_IDENTIFIER){
		if(!symbolExists((char*)root->tok->extra)){
			reportError("SA006", "Symbol Does Not Exist %s: Line %d", (char*)root->tok->extra, root->tok->lineNo);
			return false;
		}
		Type tsym = getSymbolType((char*)root->tok->extra, root->tok->lineNo);
		if(typesEqualMostly(tsym, expect)){
			return true;
		}
		if(!silent){
			reportError("SA007", "Symbol of Wrong Type %s: Line %d", (char*)root->tok->extra, root->tok->lineNo);
			errShowType("EXPECTED: ",&expect);
			errShowType("FOUND: ", &tsym);
		}
	}

	if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_EXP){
		return (integralType(expect) || floatingType(expect)) &&
				semAnalyExpr((PTree*)root->child1, expect, silent) &&
				semAnalyExpr((PTree*)root->child2, expect, silent);
	}
	if(root->typ == PTT_MOD || root->typ == PTT_XOR || root->typ == PTT_AND || root->typ == PTT_OR ||
			root->typ == PTT_NOT){
		return (integralType(expect)) &&
				semAnalyExpr((PTree*)root->child1, expect, silent) &&
				semAnalyExpr((PTree*)root->child2, expect, silent);
	}


	if(root->typ == PTT_PARAM_CONT){
		char *fname = ((PTree*)root->child1)->tok->extra;
		NamedFunctionMapEnt *l = getFunctionVersions(fname);
		if(l == NULL){
			reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
			return false;
		}
		FunctionVersion *ver = l->V;
		while(ver != NULL){
			if(typesEqualMostly(((Type*)ver->sig.children)[0], expect)){ // return type correct
				// test this version
				PTree *param = (PTree*)root->child2;
				int pNo = 0;
				bool found = true;
				while(param != NULL){
					pNo ++;
					if(pNo >= ver->sig.numchildren){
						found = false;
						break;
					}
					PTree *p = (PTree*)param->child1;
					if(!semAnalyExpr(p, ((Type*)ver->sig.children)[pNo], true)){
						found = false;
						break;
					}
					param = (PTree*)param->child2;
				}
				if(found && pNo == ver->sig.numchildren - 1){
					markFunctionVersionUsed(ver); // mark used
					return true;
				}
			}
			ver = (FunctionVersion*)ver->next;
		}
		reportError("SA006", "Function %s Signature Does Not Match: Line %d", fname, root->tok->lineNo);
		errShowType("RET-NEEDED: ", &expect);
		ver = l->V;
		while(ver != NULL){
			errShowType("  SIG: ", &ver->sig);
			ver = (FunctionVersion*)ver->next;
		}
	}

	if(!silent){
		reportError("SA008", "Type Expression Deduction Failed: Line %d", root->tok->lineNo);
		errShowType("EXPECTED: ",&expect);
	}
	return false;
}


bool semAnalyStmt(PTree *root, Type sig)
{
	printf("STMT: %d\n", root->typ);

	if(root->typ == PTT_RETURN){
		if(!semAnalyExpr((PTree*)root->child1, ((Type*)sig.children)[0], false)){
			reportError("SA004", "Return Type Invalid: Line %d", root->tok->lineNo);
			return false;
		}
	}else if(root->typ == PTT_DECL){
		char *sname = (char*)root->tok->extra;
		if(symbolExistsCurrentLevel(sname)){
			reportError("SA002", "Second Declaration of %s: Line %d", sname, root->tok->lineNo);
			return false;
		}if(symbolExists(sname)){
			reportError("#SA003", "Warning: Hiding Previous Declaration of %s: Line %d", sname, root->tok->lineNo);
		}
		Type styp = deduceTypeDeclType((PTree*)root->child1);
		addSymbol(sname, styp);
		if(root->child2 != NULL){
			if(!semAnalyExpr((PTree*)root->child2, styp, false)){
				reportError("SA001", "Declaration Assignment Type Invalid: Line %d", root->tok->lineNo);
				return false;
			}
		}
	}
	return true;
}


bool semAnalyFunc(PTree *root, bool global, Type sig)
{
	//PTree *defn = NULL;
	int errs = 0;
	PTree *body = root;
	if(!global){
		enterScope();
		// need to push locals onto stack
		dumpParseTreeDet(root, 0);
		int i;
		for(i=1; i<sig.numchildren; i++){

		}


	}else{
		enterGlobalSpace();
	}

	while(body != NULL){
		if(!semAnalyStmt((PTree*)body->child1, sig)){
			errs ++;
		}
		body = (PTree*)body->child2;
	}

	if(!global){
		exitScope();
	}
	if(errs > 0)
		return false;

	if(global){
		bool ret = true;
		while(true){
			FunctionVersion *v = markFirstUsedVersionChecked();
			if(v == NULL)
				return ret;
			semAnalyFunc(v->defRoot, false, v->sig);
		}
		printf("DONE!");
	}
	return true;
}
