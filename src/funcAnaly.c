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

/*
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

*/
// Deduces return types of a function call
TypeDeductions handleFunctCall(PTree *root, int *err)
{
	// fetch function versions
	char *fname = ((PTree*)root->child1)->tok->extra;
	NamedFunctionMapEnt *l = getFunctionVersions(fname);
	if(l == NULL){
		reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
		(*err) ++;
		return singleTypeDeduction(newBasicType(TB_ERROR));
	}
	TypeDeductions ret = newTypeDeductions();

	// deduce parameters
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
		free(pDeds);
		ret = singleTypeDeduction(newBasicType(TB_ERROR));
		return ret;
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
			TypeDeductions sigParam = singleTypeDeduction(((Type*)ver->sig.children)[i]);
			if(!typeDeductionMergeExists(sigParam, pDeds[i-1])){
				success = false;
				freeTypeDeductions(sigParam);
				break;
			}
			freeTypeDeductions(sigParam);
		}
		if(success){
			Type typ = duplicateType(((Type*)ver->sig.children)[0]);
			utarray_push_back(ret.types, &typ);
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
		free(pDeds);
		ret = singleTypeDeduction(newBasicType(TB_ERROR));
		return ret;
	}
	free(pDeds);
	return ret;
}

TypeDeductions deduceTreeType(PTree *root, int *err)
{
	if(root->child1 == NULL && root->child2 == NULL){
		if(root->typ == PTT_INT){
			setTypeDeductions(root,
					expandedTypeDeduction(getLogicalIntegerTypeByLiteral((char*)root->tok)));
			return root->deducedTypes;
		}
		else if(root->typ == PTT_FLOAT){
			setTypeDeductions(root,
					expandedTypeDeduction(getLogicalFloatTypeByLiteral((char*)root->tok)));
			return root->deducedTypes;
		}
		else if(root->typ == PTT_STRING){
			setTypeDeductions(root, expandedTypeDeduction(newBasicType(TB_NATIVE_STRING)));
			return root->deducedTypes;
		}
		else if(root->typ == PTT_IDENTIFIER){
			if(!symbolExists((char*)root->tok->extra)){
				reportError("SA006", "Symbol Does Not Exist %s: Line %d", (char*)root->tok->extra, root->tok->lineNo);
				(*err) ++;
				setTypeDeductions(root, singleTypeDeduction(newBasicType(TB_ERROR)));
			}
			setTypeDeductions(root,
					expandedTypeDeduction(getSymbolType((char*)root->tok->extra, root->tok->lineNo)));
			return root->deducedTypes;
		}
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_EXP || root->typ == PTT_AND || root->typ == PTT_OR || root->typ == PTT_AND ||
			root->typ == PTT_XOR){
		TypeDeductions tInts = expandedTypeDeduction(newBasicType(TB_NATIVE_INT8));
		TypeDeductions tFloats = expandedTypeDeduction(newBasicType(TB_NATIVE_FLOAT32));

		TypeDeductions ch1 = deduceTreeType((PTree*)root->child1, err);
		TypeDeductions ch2 = deduceTreeType((PTree*)root->child2, err);

		TypeDeductions overlap = mergeTypeDeductionsOrErr(ch1, ch2, err, MS_NEITHER);
		if(*err > 0 || !(	typeDeductionMergeExists(overlap, tInts) ||
							typeDeductionMergeExists(overlap, tFloats))){
			reportError("SA011", "Operation (+-/^*&|~) Requires Both Integer Or Both Float Types: Line %d", root->tok->lineNo);
			(*err) ++;
			freeTypeDeductions(overlap);
			overlap = singleTypeDeduction(newBasicType(TB_ERROR));
		}
		freeTypeDeductions(tInts);
		freeTypeDeductions(tFloats);
		setTypeDeductions(root, overlap);
		return overlap;
	}else if(root->typ == PTT_PARAM_CONT){ // function call
		TypeDeductions options = handleFunctCall(root,err);
		setTypeDeductions(root, options);
		return options;
	}else if(root->typ == PTT_EQUAL){
		TypeDeductions ch1 = deduceTreeType((PTree*)root->child1, err);
		TypeDeductions ch2 = deduceTreeType((PTree*)root->child2, err);
		TypeDeductions overlap = mergeTypeDeductionsOrErr(ch1, ch2, err, MS_NEITHER);
		if(*err > 0){
			reportError("SA032", "Both sides of equals must be of same type: Line %d", root->tok->lineNo);
			(*err) ++;
			freeTypeDeductions(overlap);
			overlap = singleTypeDeduction(newBasicType(TB_ERROR));
		}
		setTypeDeductions(root, overlap);
		return overlap;
	}




	reportError("SA010", "Unknown Tree Expression: %s", getParseNodeName(root));
	(*err) ++;
	TypeDeductions d;
	return d;
}


bool finalizeSingleDeduction(PTree *root)
{
	if(utarray_len(root->deducedTypes.types) < 1){
		reportError("SA053", "No Single Deduction Found... Giving Up!");
		return false;
	}
	if(utarray_len(root->deducedTypes.types) > 1){
		reportError("#SA053", "Warning: Multiple sDeduction Found, Choosing First");
		showTypeDeductionOption(root->deducedTypes);
	}
	propagateTreeType(root);
	return true;
}

// The called tree has it's final type deduced, so now job is to propagate this down the tree
void propagateTreeType(PTree *root){
	TypeDeductions final = root->deducedTypes;
	if(root->child1 == NULL && root->child2 == NULL){
		// nothing to do here
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_EXP || root->typ == PTT_AND || root->typ == PTT_OR || root->typ == PTT_AND ||
			root->typ == PTT_XOR || root->typ == PTT_EQUAL || root->typ == PTT_ASSIGN){
		// both sides need be same type
		setTypeDeductions((PTree*)root->child1, duplicateTypeDeductions(final));
		setTypeDeductions((PTree*)root->child2, duplicateTypeDeductions(final));
		propagateTreeType((PTree*)root->child1);
		propagateTreeType((PTree*)root->child2);
	}else if(root->typ == PTT_PARAM_CONT){ // function call

		char *fname = ((PTree*)root->child1)->tok->extra;
		NamedFunctionMapEnt *l = getFunctionVersions(fname);

		PTree *param = (PTree*)root->child2;
		int pCount = 0;
		while(param != NULL){
			pCount ++;
			param = (PTree*)param->child2;
		}

		FunctionVersion *ver = l->V;
		while(ver != NULL){
			if(ver->sig.numchildren != 1+pCount){
				ver = (FunctionVersion*)ver->next;
				continue;
			}
			// check return type match
			TypeDeductions tmp = singleTypeDeduction(((Type*)ver->sig.children)[0]);
			if(!typeDeductionMergeExists(tmp, root->deducedTypes)){
				ver = (FunctionVersion*)ver->next;
				freeTypeDeductions(tmp);
				continue;
			}
			freeTypeDeductions(tmp);
			// parameter type match
			int i;
			bool success = true;
			param = (PTree*)root->child2;
			for(i=1; i<ver->sig.numchildren; i++){
				TypeDeductions tmp = singleTypeDeduction(((Type*)ver->sig.children)[i]);
				if(!typeDeductionMergeExists(tmp, ((PTree*)param->child1)->deducedTypes)){
					success = false;
					freeTypeDeductions(tmp);
					break;
				}
				freeTypeDeductions(tmp);
				param = (PTree*)param->child2;
			}
			if(success){ // propagate tree to each parameter
				param = (PTree*)root->child2;
				for(i=1; i<ver->sig.numchildren; i++){
					setTypeDeductions((PTree*)param->child1, singleTypeDeduction(((Type*)ver->sig.children)[i]));
					propagateTreeType((PTree*)param->child1);
					param = (PTree*)param->child2;
				}
				markFunctionVersionUsed(ver);
				break;
			}
			ver = (FunctionVersion*)ver->next;
		}
	}else if(root->typ == PTT_RETURN){
		setTypeDeductions((PTree*)root->child1, duplicateTypeDeductions(final));
		propagateTreeType((PTree*)root->child1);
	}else if(root->typ == PTT_DECL){
		if(root->child2 != NULL){
			setTypeDeductions((PTree*)root->child2, duplicateTypeDeductions(final));
			propagateTreeType((PTree*)root->child2);
		}
	}else{
		fatalError("No propagateTreeType for tree node!");
	}
}



inline bool declaration(PTree *root, int *err){
	char *sname = (char*)root->tok->extra;
	if(symbolExistsCurrentLevel(sname)){
		reportError("SA002", "Second Declaration of %s: Line %d", sname, root->tok->lineNo);
		return false;
	}if(symbolExists(sname)){
		reportError("#SA003", "Warning: Hiding Previous Declaration of %s: Line %d", sname, root->tok->lineNo);
	}
	Type styp = deduceTypeDeclType((PTree*)root->child1);
	addSymbol(sname, styp);
	setTypeDeductions(root, singleTypeDeduction(styp));
	if(root->child2 != NULL){
		TypeDeductions assignment = deduceTreeType((PTree*)root->child2, err);
		if(*err > 0 || !typeDeductionMergeExists(root->deducedTypes, assignment)){
			reportError("SA001", "Declaration Assignment Type Invalid: Line %d", root->tok->lineNo);
			return false;
		}
	}
	return true;
}

bool semAnalyStmt(PTree *root, Type sig)
{
	if(root == NULL)
		return false;
	if(root->tok != NULL && root->tok->extra != NULL)
		printf("STMT: %s\n", (char*)root->tok->extra);
	int err = 0;
	if(root->typ == PTT_RETURN){
		TypeDeductions deduced = deduceTreeType((PTree*)root->child1, &err);
		TypeDeductions required = singleTypeDeduction(((Type*)sig.children)[0]);
		TypeDeductions res = mergeTypeDeductionsOrErr(deduced, required, &err, MS_NEITHER);
		freeTypeDeductions(required);
		setTypeDeductions(root, res);
		if(err > 0){
			reportError("SA004", "Return Type Invalid: Line %d", root->tok->lineNo);
			return false;
		}
		return finalizeSingleDeduction(root);
	}else if(root->typ == PTT_DECL){
		if(!declaration(root, &err))
			return false;
		return finalizeSingleDeduction(root);
	}else if(root->typ == PTT_PARAM_CONT){
		TypeDeductions deducts = handleFunctCall(root, &err);
		setTypeDeductions(root, deducts);
		if(err > 0){
			reportError("SA043", "Function Call Failed: Line %d", root->tok->lineNo);
			return false;
		}
		return finalizeSingleDeduction(root);
	}else if(root->typ == PTT_IF){
		PTree *cond = (PTree*)root->child1;
		PTree *branch = (PTree*)root->child2;

		TypeDeductions booleanType = expandedTypeDeduction(newBasicType(TB_NATIVE_INT8));
		TypeDeductions deduced = deduceTreeType(cond, &err);

		TypeDeductions res = mergeTypeDeductionsOrErr(deduced, booleanType, &err, MS_NEITHER);
		freeTypeDeductions(booleanType);
		setTypeDeductions(root, res);
		setTypeDeductions(cond, duplicateTypeDeductions(res));
		if(err > 0 || !finalizeSingleDeduction(cond)){
			reportError("SA025", "If Condition Must Evaluate To Integer Type: Line %d", cond->tok->lineNo);
			return false;
		}

		bool ret;
		if(branch->typ == PTT_IFELSE_SWITCH){
			ret = blockUnit((PTree*)branch->child1, sig, false) && blockUnit((PTree*)branch->child2, sig, false);
		}else{
			ret = blockUnit(branch, sig, false);
		}
		return ret;
	}else if(root->typ == PTT_FOR){
		PTree *cond = (PTree*)root->child1;
		PTree *branch = (PTree*)root->child2;
		PTree *decl = (PTree*)cond->child1;
		PTree *arr = (PTree*)cond->child2;

		// Declaration:
		enterScope();
		if(!declaration(decl, &err) || !finalizeSingleDeduction(decl))
			return false;

		// Find out what type we are iterating/expected object to iterate
		Type typ = duplicateType(lastSymbol()->sig);
		Type typExpected = newVectorType(typ);
		TypeDeductions expected = singleTypeDeduction(typExpected);
		freeType(typExpected);

		// Iterated Object
		TypeDeductions iter = deduceTreeType(arr, &err);

		TypeDeductions res = mergeTypeDeductionsOrErr(expected, iter, &err, MS_NEITHER);
		freeTypeDeductions(expected);
		setTypeDeductions(root, res);
		setTypeDeductions(arr, duplicateTypeDeductions(res));
		if(err > 0 || !finalizeSingleDeduction(arr)){
			reportError("SA051", "Array Iteration Type Invalid: Line %d", arr->tok->lineNo);
			return false;
		}
		return blockUnit(branch, sig, false);
	}else if(root->typ == PTT_ASSIGN){
		TypeDeductions rhs = deduceTreeType((PTree*)root->child1, &err);
		TypeDeductions lhs = deduceTreeType((PTree*)root->child2, &err);
		TypeDeductions res = mergeTypeDeductionsOrErr(lhs, rhs, &err, MS_LHS);
		setTypeDeductions(root, res);
		if(err > 0){
			reportError("SA052", "Assignment Invalid: Line %d", root->tok->lineNo);
			return false;
		}
		if(!((Type*)utarray_front(res.types))->mutable){
			reportError("SA053", "Cannot assign to immutable type: Line %d", root->tok->lineNo);
			return false;
		}
		return finalizeSingleDeduction(root);
	}else{
		reportError("SA009", "Unknown Statement: %s", getParseNodeName(root));
		return false;
	}
	return (err == 0);
}

bool blockUnit(PTree *root, Type sig, bool global)
{
	if(!global)
		enterScope();
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
	if(!global)
		exitScope();
	return (errs == 0);
}

bool semAnalyFunc(PTree *root, bool global, Type sig)
{
	//PTree *defn = NULL;
	int errs = 0;
	PTree *body = root;
	//dumpParseTreeDet(root, 0);
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

	if(!blockUnit(body, sig, global))
		errs ++;

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
			printf("Inside function: %s \n", v->funcName);
			semAnalyFunc(v->defRoot, false, v->sig);
		}
		printf("DONE!");
	}
	return (errs == 0);

}
