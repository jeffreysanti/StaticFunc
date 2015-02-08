/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  funcAnaly.c
 *  Function Semantic Analyzer
 *
 */

#include "funcAnaly.h"

UT_icd int_icd = {sizeof(int), NULL, NULL, NULL };


void onFunctionCallDeductionChosen(void * deductionList, int chosen)
{
	TypeDeductions deductions = *((TypeDeductions*)deductionList);
	PTree *callRoot = (PTree*)deductions.extraPtr2;
	NamedFunctionMapEnt *funcMap = (NamedFunctionMapEnt*)deductions.extraPtr1;

	int *iptr = (int*)utarray_eltptr(deductions.extra, chosen);
	int chosenVerno = *iptr;
	Type *chosenRet = (Type*)utarray_eltptr(deductions.types, chosen);

	callRoot->deducedType = *chosenRet;

	int verno = 0;
	FunctionVersion *v = funcMap->V;
	while(verno < chosenVerno){
		v = (FunctionVersion*)v->next;
		verno ++;
	}
	// finalize deductions - last time we did this silently, now we know proper deduction
	int i, err=0;
	PTree *param = (PTree*)callRoot->child2;
	for(i=1; i<v->sig.numchildren; i++){
		findDeductionMatching_multrecv(((Type*)v->sig.children)[i],
				deduceTreeType((PTree*)param->child1, &err));
		param = (PTree*)param->child2;
	}

	// now mark this as used function
	markFunctionVersionUsed(v);
}


// Deduces return types of a function call
TypeDeductions handleFunctCall(PTree *root, int *err)
{
	// fetch function versions
	char *fname = ((PTree*)root->child1)->tok->extra;
	NamedFunctionMapEnt *l = getFunctionVersions(fname);
	if(l == NULL){
		reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
		(*err) ++;
		return singleType(newBasicType(TB_ERROR));
	}
	TypeDeductions ret = newTypeDeductions();
	ret.onChooseDeduction = &onFunctionCallDeductionChosen;
	ret.extraPtr1 = l;
	ret.extraPtr2 = root;
	utarray_new(ret.extra, &int_icd);


	// deduce paramaters
	PTree *param = (PTree*)root->child2;
	int pCount = 0;
	while(param != NULL){
		pCount ++;
		param = (PTree*)param->child2;
	}
	TypeDeductions *pDeds = calloc(pCount, sizeof(TypeDeductions));
	if(pDeds == NULL){
		fatalError("Out of Memory [handleFunctCall]\n");
	}
	param = (PTree*)root->child2;
	int pNum;
	for(pNum=0; pNum<pCount; pNum++){
		pDeds[pNum] = deduceTreeType((PTree*)param->child1, err); // get all deductions for this param
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

	int verno = 0;
	while(ver != NULL){
		if(ver->sig.numchildren != 1+pCount){
			ver = (FunctionVersion*)ver->next;
			verno ++;
			continue;
		}
		int i;
		bool success = true;
		for(i=1; i<ver->sig.numchildren; i++){
			Type t = findDeductionMatching_multrecv_silent_nofree(((Type*)ver->sig.children)[i], pDeds[i-1]);
			if(t.base == TB_ERROR){
				success = false;
				break;
			}
		}
		if(success){
			utarray_push_back(ret.types, (Type*)&((Type*)ver->sig.children)[0]);
			utarray_push_back(ret.extra, &verno);
		}
		ver = (FunctionVersion*)ver->next;
		verno ++;
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
	for(pNum=0; pNum<pCount; pNum++){
		freeTypeDeductions(pDeds[pNum]);
	}
	free(pDeds);

	return ret;
}


TypeDeductions deduceTreeType(PTree *root, int *err)
{
	if(root->child1 == NULL && root->child2 == NULL){
		if(root->typ == PTT_INT){
			return singleType(getLogicalIntegerTypeByLiteral((char*)root->tok));
		}
		else if(root->typ == PTT_FLOAT){
			return singleType(getLogicalFloatTypeByLiteral((char*)root->tok));
		}
		else if(root->typ == PTT_STRING){
			return singleType(newBasicType(TB_NATIVE_STRING));
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
			root->typ == PTT_EXP || root->typ == PTT_AND || root->typ == PTT_OR || root->typ == PTT_AND ||
			root->typ == PTT_XOR){
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
			reportError("SA011", "Operation (+-/^*&|~) Requires Both Integer Or Both Float Types: Line %d", root->tok->lineNo);
			(*err) ++;
			return singleType(newBasicType(TB_ERROR));
		}
	}else if(root->typ == PTT_PARAM_CONT){ // function call
		TypeDeductions options = handleFunctCall(root,err);
		return options;
	}else if(root->typ == PTT_EQUAL){
		// TODO
		return singleType(newBasicType(TB_NATIVE_INT8));
	}




	reportError("SA010", "Unknown Tree Expression: %s", getParseNodeName(root));
	(*err) ++;
	return singleType(newBasicType(TB_ERROR));
}


bool semAnalyStmt(PTree *root, Type sig)
{
	printf("STMT: %s\n", (char*)root->tok->extra);
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
		Type retval = findDeductionMatching_any(handleFunctCall(root, &err));
		if(retval.base == TB_ERROR || err > 0){
			reportError("SA001", "Function Call Failed: Line %d", root->tok->lineNo);
			return false;
		}
		root->deducedType = retval;
	}else if(root->typ == PTT_IF){
		PTree *cond = (PTree*)root->child1;
		PTree *branch = (PTree*)root->child2;

		Type booleanType = newBasicType(TB_ANY_INT);
		Type condtype = findDeductionMatching_multrecv(booleanType,
							deduceTreeType(cond, &err));
		if(condtype.base == TB_ERROR || err > 0){
			reportError("SA025", "If Condition Must Evaluate To Integer Type: Line %d", cond->tok->lineNo);
			return false;
		}
		if(branch->typ == PTT_IFELSE_SWITCH){
			return blockUnit((PTree*)branch->child1, sig) && blockUnit((PTree*)branch->child2, sig);
		}else{
			return blockUnit(branch, sig);
		}
	}else{
		reportError("SA009", "Unknown Statement: %s", getParseNodeName(root));
		return false;
	}

	return (err == 0);
}

bool blockUnit(PTree *root, Type sig)
{
	int errs = 0;
	if(root->typ != PTT_STMTBLOCK){
		if(!semAnalyStmt(root, sig)){
			errs ++;
		}
	}else{
		PTree *body = root;
		while(body != NULL){
			if(!semAnalyStmt((PTree*)body->child1, sig)){
				errs ++;
			}
			body = (PTree*)body->child2;
		}
	}
	return (errs == 0);
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

	blockUnit(body, sig);

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
