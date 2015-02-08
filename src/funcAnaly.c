/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  funcAnaly.c
 *  Function Semantic Analyzer
 *
 */

#include "funcAnaly.h"

TypeDeductions handleFunctCall(PTree *root, int *err)
{
	char *fname = ((PTree*)root->child1)->tok->extra;
	NamedFunctionMapEnt *l = getFunctionVersions(fname);
	if(l == NULL){
		reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
		(*err) ++;
		return singleType(newBasicType(TB_ERROR));
	}
	TypeDeductions ret = newTypeDeductions();

	PTree *param = (PTree*)root->child2;
	int pCount = 0;
	while(param != NULL){
		pCount ++;
		param = (PTree*)param->child2;
	}
	TypeDeductions *pDeds = malloc(sizeof(TypeDeductions) * pCount);
	if(pDeds == NULL){
		fatalError("Out of Memory [handleFunctCall]\n");
	}
	param = (PTree*)root->child2;
	int pNum;
	TypeDeductions deduct;
	for(pNum=0; pNum<pCount; pNum++){
		deduct = deduceTreeType((PTree*)param->child1, err);
		pDeds[pNum] = deduct;
		param = (PTree*)param->child2;
	}
	if(*err != 0){
		freeTypeDeductions(ret);
		for(pNum=0; pNum<pCount; pNum++){
			freeTypeDeductions(pDeds[pNum]);
		}
		free(pDeds);
		return singleType(newBasicType(TB_ERROR));
	}

	FunctionVersion *ver = l->V;
	while(ver != NULL){
		if(ver->sig.numchildren != 1+pCount){
			ver = (FunctionVersion*)ver->next;
			continue;
		}
		int i;
		for(i=1; i<ver->sig.numchildren; i++){
			Type t = findDeductionMatching_multrecv_silent(((Type*)ver->sig.children)[i], pDeds[i-1]);
			if(t.base != TB_ERROR){
				utarray_push_back(ret.types, &t);
			}
		}
		//utarray_push_back(intOrFloat.types, &tInt);
		ver = (FunctionVersion*)ver->next;
	}
	if(utarray_len(ret.types) == 0){
		reportError("SA015", "No Signature Match For Function %s: Line %d", fname, root->tok->lineNo);
		(*err) ++;
		ver = l->V;
		while(ver != NULL){
			errShowType("  SIG: ", &ver->sig);
			ver = (FunctionVersion*)ver->next;
		}
		for(pNum=0; pNum<pCount; pNum++){
			errRaw("\t* PARAM\n");
			showTypeDeductionOption(pDeds[pNum]);
		}
		freeTypeDeductions(ret);
		for(pNum=0; pNum<pCount; pNum++){
			freeTypeDeductions(pDeds[pNum]);
		}
		free(pDeds);
		return singleType(newBasicType(TB_ERROR));
	}
		/*if(fver->next == NULL){ // no attempts left
			char *fname = ((PTree*)root->child1)->tok->extra;
			NamedFunctionMapEnt *l = getFunctionVersions(fname);
			reportError("SA006", "Function %s No Possible Signature Matches: Line %d", fname, root->tok->lineNo);
			FunctionVersion *ver = l->V;
			while(ver != NULL){
				errShowType("  SIG: ", &ver->sig);
				ver = (FunctionVersion*)ver->next;
			}
			(*err) ++;
			return newBasicType(TB_ERROR);
		}
		*lastTry = (FunctionVersion*)fver->next;
		fver = *lastTry;*/
	free(pDeds);

	return ret;
}


/*bool handleFunctCall(PTree *root, Type expect)
{
	char *fname = ((PTree*)root->child1)->tok->extra;
	NamedFunctionMapEnt *l = getFunctionVersions(fname);
	if(l == NULL){
		reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
		return false;
	}
	FunctionVersion *ver = l->V;
	while(ver != NULL){
		if(expect.base == TB_NATIVE_VOID || typesEqualMostly(((Type*)ver->sig.children)[0], expect)){ // return type correct
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
	return false;
}


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
	else if(root->typ == PTT_FLOAT){
		if(floatingType(expect)){
			return true;
		}
		if(!silent){
			errShowType("EXPECTED: ",&expect);
			errShowTypeStr("FOUND: ","Float");
		}
		return false;
	}
	else if(root->typ == PTT_IDENTIFIER){
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
	else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_EXP){
		return (integralType(expect) || floatingType(expect)) &&
				semAnalyExpr((PTree*)root->child1, expect, silent) &&
				semAnalyExpr((PTree*)root->child2, expect, silent);
	}
	else if(root->typ == PTT_MOD || root->typ == PTT_XOR || root->typ == PTT_AND || root->typ == PTT_OR ||
			root->typ == PTT_NOT){
		return (integralType(expect)) &&
				semAnalyExpr((PTree*)root->child1, expect, silent) &&
				semAnalyExpr((PTree*)root->child2, expect, silent);
	}
	else if(root->typ == PTT_PARAM_CONT){
		return handleFunctCall(root, expect);
	}
	else{
		if(!silent){
			reportError("SA008", "Type Expression Deduction Failed: Line %d", root->tok->lineNo);
			errShowType("EXPECTED: ",&expect);
		}
	}
	return false;
}

*/


/*Type deduceTreeTypeFunctionCall(PTree *root, int *err, FunctionVersion **lastTry)
{
	FunctionVersion *fver = *lastTry;
	if(fver == NULL){
		char *fname = ((PTree*)root->child1)->tok->extra;
		NamedFunctionMapEnt *l = getFunctionVersions(fname);
		if(l == NULL){
			reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
			(*err) ++;
			return newBasicType(TB_ERROR);
		}
		*lastTry = l->V;
		fver = l->V;
	}else{

		if(fver->next == NULL){ // no attempts left
			char *fname = ((PTree*)root->child1)->tok->extra;
			NamedFunctionMapEnt *l = getFunctionVersions(fname);
			reportError("SA006", "Function %s No Possible Signature Matches: Line %d", fname, root->tok->lineNo);
			FunctionVersion *ver = l->V;
			while(ver != NULL){
				errShowType("  SIG: ", &ver->sig);
				ver = (FunctionVersion*)ver->next;
			}
			(*err) ++;
			return newBasicType(TB_ERROR);
		}
		*lastTry = (FunctionVersion*)fver->next;
		fver = *lastTry;
	}



}*/

TypeDeductions deduceTreeType(PTree *root, int *err)
{
	if(root->child1 == NULL && root->child2 == NULL){
		if(root->typ == PTT_INT){
			return singleType(getLogicalIntegerTypeByLiteral((char*)root->tok));
		}
		else if(root->typ == PTT_FLOAT){
			return singleType(getLogicalFloatTypeByLiteral((char*)root->tok));
		}
		else if(root->typ == PTT_IDENTIFIER){
			if(!symbolExists((char*)root->tok->extra)){
				reportError("SA006", "Symbol Does Not Exist %s: Line %d", (char*)root->tok->extra, root->tok->lineNo);
				(*err) ++;
				return singleType(newBasicType(TB_ERROR));
			}
			return singleType(getSymbolType((char*)root->tok->extra, root->tok->lineNo));
		}
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_EXP){
		Type tInt = newBasicType(TB_ANY_INT);
		Type tFloat = newBasicType(TB_ANY_FLOAT);

		TypeDeductions intOrFloat = newTypeDeductions();
		utarray_push_back(intOrFloat.types, &tInt);
		utarray_push_back(intOrFloat.types, &tFloat);
		Type child1 = findDeductionMatching_multboth(intOrFloat, deduceTreeType((PTree*)root->child1, err));

		intOrFloat = newTypeDeductions();
		utarray_push_back(intOrFloat.types, &tInt);
		utarray_push_back(intOrFloat.types, &tFloat);
		Type child2 = findDeductionMatching_multboth(intOrFloat, deduceTreeType((PTree*)root->child2, err));

		if(((integralType(child1) && integralType(child2)) || (floatingType(child1) && floatingType(child2))) && *err == 0){
			return singleType(getMostGeneralType(child1, child2));
		}else{
			reportError("SA011", "Operation (+-/^*) Requires Both Integer Or Both Float Types: Line %d", root->tok->lineNo);
			(*err) ++;
			return singleType(newBasicType(TB_ERROR));
		}
	}else if(root->typ == PTT_PARAM_CONT){ // function call
		TypeDeductions options = handleFunctCall(root,err);
		return options;
	}


	/*else if(root->child1 != NULL && root->child2 == NULL){
		Type child = deduceTreeType((PTree*)root->child1, err);
		if((*err) > 0)
			return singleType(newBasicType(TB_ERROR));
		((PTree*)root->child1)->deducedType = child;
	}else if(root->child2 != NULL && root->child1 == NULL){
		Type child = deduceTreeType((PTree*)root->child2, err);
		if((*err) > 0)
			return singleType(newBasicType(TB_ERROR));
		((PTree*)root->child2)->deducedType = child;
	}else{
		if(root->typ == PTT_PARAM_CONT){ // function call

		}else{
			// all binary operators
			Type child1 = deduceTreeType((PTree*)root->child1, err);
			Type child2 = deduceTreeType((PTree*)root->child2, err);
			if((*err) > 0){
				freeType(child1); // in case one was successful
				freeType(child2);
				return singleType(newBasicType(TB_ERROR));
			}
			((PTree*)root->child1)->deducedType = child1;
			((PTree*)root->child2)->deducedType = child2;
			if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
						root->typ == PTT_EXP){
				if((integralType(child1) && integralType(child2)) || (floatingType(child1) && floatingType(child2))){
					return singleType(getMostGeneralType(child1, child2));
				}else{
					reportError("SA011", "Operation (+-/^*) Requires Both Integer Or Both Float Types: Line %d", root->tok->lineNo);
					(*err) ++;
					return singleType(newBasicType(TB_ERROR));
				}
			}
		}
	}*/




	reportError("SA010", "Unknown Tree Expression: %s", getParseNodeName(root));
	(*err) ++;
	return singleType(newBasicType(TB_ERROR));
}


bool semAnalyStmt(PTree *root, Type sig)
{
	printf("STMT: %d\n", root->typ);
	int err = 0;
	if(root->typ == PTT_RETURN){
		Type ret = findDeductionMatching_multrecv(((Type*)sig.children)[0],
				deduceTreeType((PTree*)root->child1, &err));

		if(ret.base == TB_ERROR || err > 0){
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
			Type initalizerType = findDeductionMatching_multrecv(styp,
					deduceTreeType((PTree*)root->child2, &err));
			if(initalizerType.base == TB_ERROR || err > 0){
				reportError("SA001", "Declaration Assignment Type Invalid: Line %d", root->tok->lineNo);
				return false;
			}
		}
	}else if(root->typ == PTT_PARAM_CONT){
		TypeDeductions deduct = handleFunctCall(root, &err);
		Type *first = (Type*)utarray_front(deduct.types);
		if((*first).base == TB_ERROR || err > 0){
			freeTypeDeductions(deduct);
			reportError("SA001", "Function Call Failed: Line %d", root->tok->lineNo);
			return false;
		}
		root->deducedType = *first;
	}else{
		reportError("SA009", "Unknown Statement: %s", getParseNodeName(root));
		return false;
	}

	return (err == 0);
}


bool semAnalyFunc(PTree *root, bool global, Type sig)
{
	//PTree *defn = NULL;
	int errs = 0;
	PTree *body = root;
	dumpParseTreeDet(root, 0);
	if(!global){


		enterScope();
		// need to push locals onto stack
		PTree *paramList = (PTree*)((PTree*)root->child1)->child2;
		int i;
		for(i=1; i<sig.numchildren; i++){
			addSymbol((char*)((PTree*)paramList->child1)->tok->extra, ((Type*)sig.children)[i]);
			paramList = (PTree*)paramList->child2;
		}
		body = (PTree*)root->child2;
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
