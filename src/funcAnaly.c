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


static inline bool declaration(PTree *root, int *err, Variable **symbol){
	char *sname = (char*)root->tok->extra;
	if(variableExistsCurrentScope(sname)){
		reportError("SA002", "Second Declaration of %s: Line %d", sname, root->tok->lineNo);
		return false;
	}if(variableExists(sname)){
		reportError("#SA003", "Warning: Hiding Previous Declaration of %s: Line %d", sname, root->tok->lineNo);
	}
	Type styp = deduceTypeDeclType((PTree*)root->child1);
	*symbol = defineVariable(sname, styp);
	setTypeDeductions(root, singleTypeDeduction(styp));
	if(root->child2 != NULL){
		TypeDeductions assignment = deduceTreeType((PTree*)root->child2, err, CAST_UP);
		if(*err > 0 || !typeDeductionMergeExists(root->deducedTypes, assignment)){
			showTypeDeductionMergeError(root->deducedTypes, assignment);
			reportError("SA001", "Declaration Assignment Type Invalid: Line %d", root->tok->lineNo);
			return false;
		}
	}
	freeType(styp);
	return true;
}

static inline bool tryFuncSig(Type sig, int pCount, TypeDeductions *pDeds, TypeDeductions *ret){
	if(sig.numchildren != 1+pCount){
		return false;
	}
	int i;
	bool success = true;
	for(i=1; i<sig.numchildren; i++){
		TypeDeductions sigParam = singleTypeDeduction(((Type*)sig.children)[i]);
		if(!typeDeductionMergeExists(sigParam, pDeds[i-1])){
			success = false;
			freeTypeDeductions(sigParam);
			break;
		}
		freeTypeDeductions(sigParam);
	}
	if(success){
		Type typ = duplicateType(((Type*)sig.children)[0]);
		addTypeDeductionsType(ret, typ);
	}
	return success;
}

// Deduces return types of a function call
TypeDeductions handleFunctCall(PTree *root, int *err)
{
	// deduce parameters
	TypeDeductions ret = newTypeDeductions();
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
		pDeds[pNum] = deduceTreeType((PTree*)param->child1, err, CAST_UP); // get all deductions for this param
		param = (PTree*)param->child2;
	}
	if(*err != 0){
		freeTypeDeductions(ret);
		free(pDeds);
		ret = singleTypeDeduction(newBasicType(TB_ERROR));
		return ret;
	}

	// first check temp object
	PTree *id = (PTree*)root->child1;
	TypeDeductions callee = deduceTreeType(id, err, CAST_UP);
	if(*err > 0){
		reportError("SA005", "Function Call Does Not Deduce: Line %d", root->tok->lineNo);
	}

	Type *sig = NULL;
	while((sig=(Type*)utarray_next(callee._types,sig))){
		if(sig->base == TB_FUNCTION){
			tryFuncSig(*sig, pCount, pDeds, &ret);
		}
	}

	if(utarray_len(ret._types) == 0){
		reportError("SA015", "No Signature Match For Function Call: Line %d", root->tok->lineNo);
		(*err) ++;
		sig = NULL;
		while((sig=(Type*)utarray_next(callee._types,sig))){
			errShowType("  SIG: ", sig);
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
	// check if this is a local function call (ie: lambda)
	/*if(symbolExists((char*)id->tok->extra)){
		Type sig = getSymbolType((char*)id->tok->extra, id->tok->lineNo);
		if(sig.base == TB_FUNCTION && tryFuncSig(sig, pCount, pDeds, &ret)){
			Type typ = duplicateType(((Type*)sig.children)[0]);
			utarray_push_back(ret.types, &typ);
			free(pDeds);
			return ret;
		}
	}

	// check if this is a global function version
	char *fname = ((PTree*)root->child1)->tok->extra;
	NamedFunctionMapEnt *l = getFunctionVersions(fname);
	if(l == NULL){
		if(!symbolExists((char*)id->tok->extra)){
			reportError("SA005", "Function Does Not Exist %s: Line %d", fname, root->tok->lineNo);
		}else{
			reportError("SA109", "Local Function Type Mismatch %s: Line %d", fname, root->tok->lineNo);
			Type t = getSymbolType((char*)id->tok->extra, id->tok->lineNo);
			errShowType("SIG: ", &t);
		}
		(*err) ++;
		freeTypeDeductions(ret);
		free(pDeds);
		return singleTypeDeduction(newBasicType(TB_ERROR));
	}

	FunctionVersion *ver = l->V;
	while(ver != NULL){
		tryFuncSig(ver->sig, pCount, pDeds, &ret);
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
		free(pDeds);
		ret = singleTypeDeduction(newBasicType(TB_ERROR));
		return ret;
	}*/
	free(pDeds);
	return ret;
}

TypeDeductions handleLambdaCreation(PTree *root, int *err){
	PTree *funcRoot = root;
	PTree *typeTree = (PTree*)funcRoot->child1;
	PTree *retTypeTree = (PTree*)typeTree->child1;
	PTree *paramTypeTree = (PTree*)typeTree->child2;

	Type Treturn = deduceTypeDeclType(retTypeTree); // return type
	if(Treturn.base == TB_ERROR || Treturn.base == TB_TYPELIST){
		reportError("SA105", "Lambda has invalid return type (or invalid template): Line %ld", funcRoot->tok->lineNo);
		TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
		freeType(Treturn);
		return ret;
	}

	// paramaters
	int paramCnt = 0;
	bool failed = false;
	while(paramTypeTree != NULL){
		paramCnt ++;
		Type tempParamType = deduceTypeDeclType((PTree*)((PTree*)paramTypeTree->child1)->child1);
		if(tempParamType.base == TB_ERROR || tempParamType.base == TB_TYPELIST){
			reportError("SA106", "Lambda has invalid param %d type (or is invalid template): Line %ld", paramCnt,
					funcRoot->tok->lineNo);
			failed = true;
			(*err) ++;
			break;
		}
		freeType(tempParamType);
		paramTypeTree = (PTree*)paramTypeTree->child2;
	}
	if(failed){
		TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
		freeType(Treturn);
		return ret;
	}
	Type lambdaSig = newBasicType(TB_FUNCTION);
	allocTypeChildren(&lambdaSig, paramCnt + 1);
	((Type*)lambdaSig.children)[0] = Treturn;
	paramTypeTree = (PTree*)typeTree->child2;
	int i;
	for(i=1; i<=paramCnt; i++){
		PTree *pTempRoot = (PTree*)((PTree*)paramTypeTree->child1)->child1;
		((Type*)lambdaSig.children)[i] = deduceTypeDeclType(pTempRoot);
		paramTypeTree = (PTree*)paramTypeTree->child2;
	}

	// now we have the signature :: time to analyze the lambda function
	if(!semAnalyFunc(funcRoot, false, lambdaSig)){
		reportError("SA107", "Lambda body compile failed: Line %ld", funcRoot->tok->lineNo);
		TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
		freeType(lambdaSig);
		return ret;
	}

	// finally our return type deduction
	TypeDeductions ret = singleTypeDeduction(lambdaSig);
	freeType(lambdaSig);
	return ret;
}

TypeDeductions deduceTreeType(PTree *root, int *err, CastDirection cd)
{
	if(root->child1 == NULL && root->child2 == NULL){
		if(root->typ == PTT_INT){
			setTypeDeductions(root,
					expandedTypeDeduction(getLogicalIntegerTypeByLiteral((char*)root->tok), cd));
			return root->deducedTypes;
		}
		else if(root->typ == PTT_FLOAT){
			setTypeDeductions(root,
					expandedTypeDeduction(getLogicalFloatTypeByLiteral((char*)root->tok), cd));
			return root->deducedTypes;
		}
		else if(root->typ == PTT_STRING){
			setTypeDeductions(root, expandedTypeDeduction(newBasicType(TB_NATIVE_STRING), cd));
			return root->deducedTypes;
		}
		else if(root->typ == PTT_IDENTIFIER){
			if(!variableExists((char*)root->tok->extra)){
				// perhaps it is a global function:
				NamedFunctionMapEnt *l = getFunctionVersions((char*)root->tok->extra);
				if(l == NULL){
					reportError("SA006", "Symbol Does Not Exist %s: Line %d", (char*)root->tok->extra, root->tok->lineNo);
					(*err) ++;
					setTypeDeductions(root, singleTypeDeduction(newBasicType(TB_ERROR)));
				}else{
					TypeDeductions ret = newTypeDeductions();
					FunctionVersion *ver = l->V;
					while(ver != NULL){
						Type typ = duplicateType(ver->sig);
						addTypeDeductionsType(&ret, typ);
						ver = (FunctionVersion*)ver->next;
					}
					setTypeDeductions(root, ret);
				}
			}else{
				setTypeDeductions(root,
					expandedTypeDeduction(getNearbyVariableTypeOrErr((char*)root->tok->extra, root->tok->lineNo), cd));
			}
			return root->deducedTypes;
		}
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_SHR || root->typ == PTT_SHL ||
			root->typ == PTT_EXP || root->typ == PTT_AND || root->typ == PTT_OR || root->typ == PTT_AND || root->typ == PTT_NOT ||
			root->typ == PTT_XOR || root->typ == PTT_MOD){
		TypeDeductions tInts = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
		TypeDeductions tFloats = expandedTypeDeduction(newBasicType(TB_NATIVE_FLOAT), CAST_UP);

		TypeDeductions ch1 = deduceTreeType((PTree*)root->child1, err, cd);

		TypeDeductions overlap;
		if(root->typ != PTT_NOT){
			TypeDeductions ch2 = deduceTreeType((PTree*)root->child2, err, cd);
			overlap = mergeTypeDeductionsOrErr(ch1, ch2, err);
		}else{
			overlap = duplicateTypeDeductions(ch1);
		}

		if(*err > 0 || !(	typeDeductionMergeExists(overlap, tInts) ||
							typeDeductionMergeExists(overlap, tFloats))){
			reportError("SA011", "Operation (+-/^*&|~%%) Requires Both Integer Or Both Float Types: Line %d", root->tok->lineNo);
			(*err) ++;
			freeTypeDeductions(overlap);
			overlap = singleTypeDeduction(newBasicType(TB_ERROR));
		}
		if((root->typ == PTT_MOD || root->typ == PTT_SHR || root->typ == PTT_SHL) &&
					!typeDeductionMergeExists(overlap, tInts)){
			reportError("SA011b", "Operations (%%, shr/shl) Requires Both Integer Types: Line %d", root->tok->lineNo);
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
	}else if(root->typ == PTT_EQUAL || root->typ == PTT_NOT_EQUAL || root->typ == PTT_GT || root->typ == PTT_LT ||
							root->typ == PTT_GTE || root->typ == PTT_LTE){
		TypeDeductions ch1 = deduceTreeType((PTree*)root->child1, err, cd);
		TypeDeductions ch2 = deduceTreeType((PTree*)root->child2, err, cd);
		TypeDeductions overlap = mergeTypeDeductionsOrErr(ch1, ch2, err);
		if(*err > 0){
			reportError("SA032", "Both sides of equals/inequalities must be of same type: Line %d", root->tok->lineNo);
			(*err) ++;
			freeTypeDeductions(overlap);
			overlap = singleTypeDeduction(newBasicType(TB_ERROR));
		}

		if(root->typ == PTT_GT || root->typ == PTT_LT || root->typ == PTT_GTE || root->typ == PTT_LTE){
			TypeDeductions tInts = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
			TypeDeductions tFloats = expandedTypeDeduction(newBasicType(TB_NATIVE_FLOAT), CAST_UP);

			if(!(	typeDeductionMergeExists(overlap, tInts) ||
								typeDeductionMergeExists(overlap, tFloats))){
				reportError("SA011", "Ineuality Operations Requires Both Integer Or Both Float Types: Line %d", root->tok->lineNo);
				(*err) ++;
				freeTypeDeductions(overlap);
				overlap = singleTypeDeduction(newBasicType(TB_ERROR));
			}
			freeTypeDeductions(tInts);
			freeTypeDeductions(tFloats);
		}

		// If these are objects, we must make a call to PTT_OBJECT_EQUAL_CHECK
		TypeDeductions booleanType = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
		appendToTypeDeductionAndFree(&booleanType, expandedTypeDeduction(newBasicType(TB_NATIVE_FLOAT), CAST_UP));
		TypeDeductions res = mergeTypeDeductions(overlap, booleanType);
		freeTypeDeductions(booleanType);
		if(utarray_len(res._types) == 0){ // non-boolean
			PTree *checkNode = newParseTree(PTT_OBJECT_EQUAL_CHECK);
			checkNode->child1 = root->child1;
			checkNode->child2 = root->child2;
			((PTree*)checkNode->child1)->parent = (void*)checkNode;
			((PTree*)checkNode->child2)->parent = (void*)checkNode;
			checkNode->parent = (void*)root;
			root->child1 = (void*)checkNode;
			root->child2 = NULL;
			setTypeDeductions(checkNode, overlap);
			overlap = singleTypeDeduction(newBasicType(TB_NATIVE_INT)); // boolean type now
		}
		freeTypeDeductions(res);

		setTypeDeductions(root, overlap);

		if(root->typ == PTT_NOT_EQUAL){
			root->typ = PTT_EQUAL;
		  PTree *notNode = newParseTree(PTT_NOT);
		  notNode->child1 = (void*)root;
			notNode->parent = root->parent;
			root->parent = (void*)notNode;
			if((PTree*)((PTree*)notNode->parent)->child1 == root){
				((PTree*)notNode->parent)->child1 = (void*)notNode;
			}else{
				((PTree*)notNode->parent)->child2 = (void*)notNode;
			}
			setTypeDeductions(notNode, duplicateTypeDeductions(overlap));
		}
		return overlap;
	}else if(root->typ == PTT_ARRAY_ELM){
		PTree *node = root;
		bool paired = false;
		TypeDeductions tdPair;
		TypeDeductions tdElms;

		PTree *param = root;
		int pCount = 0;
		while(param != NULL){
			pCount ++;
			param = (PTree*)param->child2;
		}
		TypeDeductions *elmDeds = calloc(pCount, sizeof(TypeDeductions));
		if(elmDeds == NULL){
			fatalError("Out of Memory [handleFunctCall]\n");
		}
		int i = 0;
		for(i=0; i<pCount; i++){
			elmDeds[i] = newTypeDeductions();
		}
		i = 0;
		while(node != NULL){
			PTree *elm = (PTree*)node->child1;
			if(i == 0){
				freeTypeDeductions(elmDeds[0]);
				if(elm->typ == PTT_ARRAY_ELM_PAIR){
					paired = true;
					tdPair = duplicateTypeDeductions(deduceTreeType((PTree*)elm->child1, err, cd));
					elmDeds[0] = deduceTreeType((PTree*)elm->child2, err, cd);
					tdElms = duplicateTypeDeductions(elmDeds[0]);
				}else{
					elmDeds[0] = deduceTreeType(elm, err, cd);
					tdElms = duplicateTypeDeductions(elmDeds[0]);
				}
			}else{
				if((paired && elm->typ != PTT_ARRAY_ELM_PAIR) ||
						(!paired && elm->typ == PTT_ARRAY_ELM_PAIR)){
					reportError("SA037", "Cannot mix paired/unpaired elements in list: Line %d", root->tok->lineNo);
					(*err) ++;
					continue;
				}
				if(paired){
					TypeDeductions tmp = deduceTreeType((PTree*)elm->child1, err, cd);
					TypeDeductions tmp2 = mergeTypeDeductions(tdPair, tmp);
					freeTypeDeductions(tdPair);
					tdPair = tmp2;
					freeTypeDeductions(elmDeds[i]);
					elmDeds[i] = deduceTreeType((PTree*)elm->child2, err, cd);
					tmp = mergeTypeDeductions(tdElms, elmDeds[i]);
					freeTypeDeductions(tdElms);
					tdElms = tmp;
				}else{
					freeTypeDeductions(elmDeds[i]);
					elmDeds[i] = deduceTreeType(elm, err, cd);
					TypeDeductions tmp = mergeTypeDeductions(tdElms, elmDeds[i]);
					freeTypeDeductions(tdElms);
					tdElms = tmp;
				}
			}
			node = (PTree*)node->child2;
			i++;
		}
		// couple different possible types: tuple, dictionary, vector
		TypeDeductions ret = newTypeDeductions();
		if(!paired){ // tuple or vector or set
			if(utarray_len(tdElms._types) > 0){ // could be a vector
				addVectorsOfTypeDeduction(&ret, tdElms);
			}
			addAllTuplesOfTypeDeductions(&ret, elmDeds, pCount);
		}else{ // it's a dictionary
			if(utarray_len(tdElms._types) == 0 || utarray_len(tdPair._types) == 0){
				reportError("SA038", "Must have single type key and value for dictionary: Line %d", root->tok->lineNo);
				(*err) ++;
			}else{
				addDictsOfTypeDeduction(&ret, tdPair, tdElms);
			}
		}
		setTypeDeductions(root, ret);

		if(paired){
			freeTypeDeductions(tdPair);
		}
		freeTypeDeductions(tdElms);
		free(elmDeds);

		if(*err > 0){
			reportError("SA039", "Could not deduce list type: Line %d", root->tok->lineNo);
			(*err) ++;
			freeTypeDeductions(ret);
			ret = singleTypeDeduction(newBasicType(TB_ERROR));
		}
		return ret;
	}else if(root->typ == PTT_ARRAY_COMP){

		// input data
		PTree *dataIn = (PTree*)root->child1;
		PTree *tmpVar = (PTree*)dataIn->child1;
		PTree *actualData = (PTree*)dataIn->child2;
		TypeDeductions dataDeduction = deduceTreeType(actualData, err, cd);
		if(*err != 0){
			reportError("SA080", "Comprehension source deduction failed: Line %d", root->tok->lineNo);
			TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
			setTypeDeductions(root, ret);
			return ret;
		}
		enterNewScope();
		Variable *lastAddedSymb = NULL;
		if(!declaration(tmpVar, err, &lastAddedSymb) || *err != 0){
			exitScope();
			reportError("SA081", "Comprehension variable declaration failed: Line %d", root->tok->lineNo);
			TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
			setTypeDeductions(root, ret);
			return ret;
		}
		// get single elements out of vector input of comprehension
		TypeDeductions dataSingles = newTypeDeductions();
		singlesOfVectorsTypeDeduction(&dataSingles, dataDeduction);

		// check compadibility
		TypeDeductions finalElm = mergeTypeDeductionsOrErr(tmpVar->deducedTypes, dataSingles, err);
		freeTypeDeductions(dataSingles);
		if(*err > 0){
			exitScope();
			reportError("SA082", "Comprehension variable does not match data: Line %d", root->tok->lineNo);
			freeTypeDeductions(finalElm);
			TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
			setTypeDeductions(root, ret);
			return ret;
		}
		setTypeDeductions(tmpVar, finalElm);
		finalElm = newTypeDeductions();
		addVectorsOfTypeDeduction(&finalElm, tmpVar->deducedTypes);
		setTypeDeductions(actualData, finalElm);

		// output data
		PTree *dataOut = (PTree*)root->child2;
		PTree *outVar = (PTree*)dataOut->child1;
		PTree *cond = (PTree*)dataOut->child2;

		if(cond != NULL){
			TypeDeductions booleanType = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
			TypeDeductions deduced = deduceTreeType(cond, err, cd);

			TypeDeductions res = mergeTypeDeductionsOrErr(deduced, booleanType, err);
			freeTypeDeductions(booleanType);
			setTypeDeductions(cond, res);
			if(*err > 0){
				exitScope();
				reportError("SA083", "Comprehension Condition Must Evaluate To Integer Type: Line %d", root->tok->lineNo);
				TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
				setTypeDeductions(root, ret);
				return ret;
			}
		}

		// finally we can typecheck the output
		TypeDeductions finalRet = newTypeDeductions();
		if(outVar->typ == PTT_ARRAY_ELM_PAIR){
			TypeDeductions deducedKey = deduceTreeType((PTree*)outVar->child1, err, cd);
			TypeDeductions deducedVal = deduceTreeType((PTree*)outVar->child2, err, cd);
			if(*err > 0){
				exitScope();
				reportError("SA084", "Comprehension Eval (for dict) Failed: Line %d", root->tok->lineNo);
				freeTypeDeductions(finalRet);
				finalRet = singleTypeDeduction(newBasicType(TB_ERROR));
				setTypeDeductions(root, finalRet);
				return finalRet;
			}
			addDictsOfTypeDeduction(&finalRet, deducedKey, deducedVal);
		}else{
			TypeDeductions deduced = deduceTreeType(outVar, err, cd);
			if(*err > 0){
				exitScope();
				reportError("SA084", "Comprehension Eval Failed: Line %d", root->tok->lineNo);
				freeTypeDeductions(finalRet);
				finalRet = singleTypeDeduction(newBasicType(TB_ERROR));
				setTypeDeductions(root, finalRet);
				return finalRet;
			}
			addVectorsOfTypeDeduction(&finalRet, deduced);
		}

		exitScope();

		setTypeDeductions(root, finalRet);
		return finalRet;
	}else if(root->typ == PTT_LAMBDA){ // function call
		TypeDeductions options = handleLambdaCreation(root,err);
		setTypeDeductions(root, options);
		return options;
	}else if(root->typ == PTT_ARR_ACCESS){
		// ...lhs[rhs]...
		TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd);
		TypeDeductions rhs = deduceTreeType((PTree*)root->child2, err, CAST_UP);

		TypeDeductions tmpret = newTypeDeductions();
		if(*err > 0){
			reportError("SA120", "Array Access Failed (previous error): Line %d", root->tok->lineNo);
			freeTypeDeductions(tmpret);
			tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
			setTypeDeductions(root, tmpret);
			return tmpret;
		}

		// need to determine structure type
		// integer => vector
		// anything => dictionary
		TypeDeductions tInts = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
		int i = 0;
		if(typeDeductionMergeExists(rhs, tInts)){ // lhs might be a vector
			TypeDeductions tmptd = newTypeDeductions();
			singlesOfVectorsTypeDeduction(&tmptd, lhs);
			addVectorsOfTypeDeduction(&tmpret, tmptd);
			freeTypeDeductions(tmptd);
			i++;
		}
		freeTypeDeductions(tInts);
		Type *p = NULL;
		while((p=(Type*)utarray_next(lhs._types,p))){ // dictionaries
			if(p->base != TB_DICT){
				continue;
			}
			TypeDeductions key = singleTypeDeduction(((Type*)p->children)[0]);
			if(typeDeductionMergeExists(key, rhs)){ // needs to have matching key type
				Type val = newDictionaryType(duplicateType(((Type*)p->children)[0]),
						duplicateType(((Type*)p->children)[1]));
				addTypeDeductionsType(&tmpret, val);
				i++;
			}
			freeTypeDeductions(key);
		}

		TypeDeductions ret = mergeTypeDeductionsOrErr(lhs, tmpret, err);
		freeTypeDeductions(tmpret);
		tmpret = newTypeDeductions();
		if(*err > 0){
			(*err) ++;
			reportError("SA121", "No Array Access Deduction Found: Line %d", root->tok->lineNo);
			freeTypeDeductions(tmpret);
			tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
		}

		p = NULL;
		while((p=(Type*)utarray_next(ret._types,p))){
			if(p->base == TB_DICT){
				appendToTypeDeductionAndFree(&tmpret, expandedTypeDeduction(((Type*)p->children)[1], cd));
			}else{
				appendToTypeDeductionAndFree(&tmpret, expandedTypeDeduction(((Type*)p->children)[0], cd));
			}
		}

		setTypeDeductions((PTree*)root->child1, ret); // perserve it for propagate
		setTypeDeductions(root, tmpret);
		return tmpret;
	}else if(root->typ == PTT_DOT){
		TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd);

		PTree *rhs = (PTree*)root->child2;
		if(*err > 0){
			reportError("SA121", "Tuple Dot Access Failed (Prev Error): Line %d", root->tok->lineNo);
			TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
			setTypeDeductions(root, tmpret);
			return tmpret;
		}

		TypeDeductions ret = newTypeDeductions();
		TypeDeductions tuples = newTypeDeductions();

		Type *p = NULL;
		int cnt = 0;
		while((p=(Type*)utarray_next(lhs._types,p))){
			int i;
			for(i=0; i<p->numchildren; i++){
				if(((Type*)p->children)[i].altName == NULL)
					continue;
				if(strcmp(((Type*)p->children)[i].altName, (char*)rhs->tok->extra)==0){
					Type t = duplicateType(((Type*)p->children)[i]);
					Type p2 = duplicateType(*p);
					addTypeDeductionsType(&ret, t);
					addTypeDeductionsType(&tuples, p2);
					cnt++;
					break;
				}
			}
		}
		if(cnt == 0){
			reportError("SA122", "Tuple Dot Access Failed: No Element '%s': Line %d",
					rhs->tok->extra, root->tok->lineNo);
			freeTypeDeductions(ret);
			freeTypeDeductions(tuples);
			TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
			setTypeDeductions(root, tmpret);
			return tmpret;
		}
		setTypeDeductions(root, ret);
		setTypeDeductions((PTree*)root->child1, tuples);
		return ret;
	}else if(root->typ == PTT_PUSH || root->typ == PTT_QUEUE){
	  TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd); // container
	  TypeDeductions rhs = deduceTreeType((PTree*)((PTree*)root->child2)->child2, err, cd); // element being added

	  TypeDeductions matchedRHS = newTypeDeductions();
	  addVectorsOfTypeDeduction(&matchedRHS, rhs);
	  
	  if(*err > 0){
	    reportError("SA121", "Push/Queue Failed (Prev Error): Line %d", root->tok->lineNo);
	    TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
	    setTypeDeductions(root, tmpret);
	    return tmpret;
	  }

	  TypeDeductions res = mergeTypeDeductionsOrErr(lhs, matchedRHS, err);
	  freeTypeDeductions(matchedRHS);
	  if(*err > 0){
		  reportError("SA083", "Push/Queue must be applied to vector of added type. Line %d", root->tok->lineNo);
		  TypeDeductions ret = singleTypeDeduction(newBasicType(TB_ERROR));
		  setTypeDeductions(root, ret);
		  return ret;
	  }
	  matchedRHS = newTypeDeductions();
	  singlesOfVectorsTypeDeduction(&matchedRHS, res); // extract original types for parameter
	  setTypeDeductions((PTree*)((PTree*)root->child2)->child2, matchedRHS);
	  freeTypeDeductions(res);

	  TypeDeductions ret = singleTypeDeduction(newBasicType(TB_NATIVE_VOID));
	  setTypeDeductions(root, ret);
	  return ret;
	}else if(root->typ == PTT_REMOVE || root->typ == PTT_CONTAINS){
	  TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd); // container
	  TypeDeductions rhs = deduceTreeType((PTree*)((PTree*)root->child2)->child2, err, cd); // element being removed or checked
	  
	  TypeDeductions chosenLHS = newTypeDeductions();
	  TypeDeductions chosenRHS = newTypeDeductions();
	  Type *p = NULL;
	  Type *q = NULL;
	  while((p=(Type*)utarray_next(lhs._types,p))){
	    if(p->base != TB_VECTOR && p->base != TB_DICT){
	      continue;
	    }
	    while((q=(Type*)utarray_next(rhs._types,q))){
	      if(typesEqualMostly(*q, ((Type*)p->children)[0])){
		Type pc = duplicateType(*p);
		Type qc = duplicateType(*q);
		utarray_push_back(chosenLHS._types, &pc);
		utarray_push_back(chosenRHS._types, &qc);
	      }
	    }
	  }
	  
	  if(utarray_len(chosenLHS._types) == 0){
		(*err) ++;
		reportError("SA087", "Contains/Remove must be applied to vector/dict type of given key: Line %d", root->tok->lineNo);
		reportTypeDeductions("LHS", lhs);
		reportTypeDeductions("RHS", rhs);
		TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
	        setTypeDeductions(root, tmpret);
	        return tmpret;
	  }
	  setTypeDeductions((PTree*)root->child1, chosenLHS);
	  setTypeDeductions((PTree*)((PTree*)root->child2)->child2, chosenRHS);
	  
	  TypeDeductions innerType = singleTypeDeduction(newBasicType(TB_NATIVE_INT));
	  setTypeDeductions(root, innerType);
	  return innerType;
	}else if(root->typ == PTT_POP || root->typ == PTT_DEQUEUE){
	  TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd); // container
	  TypeDeductions innerType = newTypeDeductions();
	  singlesOfVectorsTypeDeduction(&innerType, lhs);

	  if(utarray_len(innerType._types) == 0){
		(*err) ++;
		reportError("SA087", "Cannot Pop/Dequeue Non Vector Type: Line %d", root->tok->lineNo);
		reportTypeDeductions("FOUND", lhs);
		TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
	        setTypeDeductions(root, tmpret);
		freeTypeDeductions(innerType);
	        return tmpret;
	  }

	  setTypeDeductions(root, innerType);
	  return innerType;
	}
	else if(root->typ == PTT_SIZE){
	  TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd); // container
	  TypeDeductions chosen = newTypeDeductions();
	  filterVectorAndDictTypes(&chosen, lhs);
	  setTypeDeductions((PTree*)root->child1, chosen);
	  
	  if(utarray_len(chosen._types) == 0){
		(*err) ++;
		reportError("SA087", "Size must be applied to vector/dict type: Line %d", root->tok->lineNo);
		reportTypeDeductions("FOUND", lhs);
		TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
	        setTypeDeductions(root, tmpret);
	        return tmpret;
	  }
	  TypeDeductions innerType = singleTypeDeduction(newBasicType(TB_NATIVE_INT));
	  setTypeDeductions(root, innerType);
	  return innerType;
	}
	else if(root->typ == PTT_KEYS || root->typ == PTT_VALUES){
	  TypeDeductions lhs = deduceTreeType((PTree*)root->child1, err, cd); // container
	  TypeDeductions chosen = newTypeDeductions();
	  filterDictTypes(&chosen, lhs);
	  setTypeDeductions((PTree*)root->child1, chosen);
	  
	  if(utarray_len(chosen._types) == 0){
		(*err) ++;
		reportError("SA087", "Keys/Values must be applied to dict type: Line %d", root->tok->lineNo);
		reportTypeDeductions("FOUND", lhs);
		TypeDeductions tmpret = singleTypeDeduction(newBasicType(TB_ERROR));
	        setTypeDeductions(root, tmpret);
	        return tmpret;
	  }
	  TypeDeductions tmp = newTypeDeductions();
	  if(root->typ == PTT_KEYS){
	    keysOfDictTypeDeductions(&tmp, chosen);
	  }
	  else{
	    valuesOfDictTypeDeductions(&tmp, chosen);
	  }
	  TypeDeductions ret = newTypeDeductions();
	  addVectorsOfTypeDeduction(&ret, tmp);
	  freeTypeDeductions(tmp);
	  setTypeDeductions(root, ret);
	  return ret;
	}
	


	reportError("SA010", "Unknown Tree Expression: %s", getParseNodeName(root));
	(*err) ++;
	TypeDeductions d = singleTypeDeduction(newBasicType(TB_ERROR));
	setTypeDeductions(root, d);
	return d;
}


bool finalizeSingleDeduction(PTree *root)
{
	if(utarray_len(root->deducedTypes._types) < 1){
		reportError("SA053", "No Single Deduction Found... Giving Up!");
		return false;
	}
	if(utarray_len(root->deducedTypes._types) > 1){
		if(root->tok != NULL)
			reportError("#SA053", "Warning: Multiple Deduction Found, Choosing First: Line %d", root->tok->lineNo);
		else
			reportError("#SA053", "Warning: Multiple Deduction Found, Choosing First");
		showTypeDeductionOption(root->deducedTypes);
	}
	propagateTreeType(root);
	return true;
}

static inline bool tryPropogateFuncSig(Type sig, int pCount, PTree *root){
	if(sig.numchildren != 1+pCount){
		return false;
	}
	if(typesEqualMostly(((Type*)sig.children)[0], root->finalType)){
		int i;
		bool success = true;
		PTree *param = (PTree*)root->child2;
		for(i=1; i<sig.numchildren; i++){
			TypeDeductions tmp = singleTypeDeduction(((Type*)sig.children)[i]);
			if(!typeDeductionMergeExists(tmp, ((PTree*)param->child1)->deducedTypes)){
				success = false;
				freeTypeDeductions(tmp);
				break;
			}
			freeTypeDeductions(tmp);
			param = (PTree*)param->child2;
		}
		if(success){
			param = (PTree*)root->child2;
			for(i=1; i<sig.numchildren; i++){
				setFinalTypeDeduction((PTree*)param->child1, duplicateType(((Type*)sig.children)[i]));
				propagateTreeType((PTree*)param->child1);
				param = (PTree*)param->child2;
			}
			return true;
		}
	}
	return false;
}

// The called tree has it's final type deduced, so now job is to propagate this down the tree
void propagateTreeType(PTree *root){
	Type final = root->finalType;
	if(final.base == TB_ERROR){
		setFinalTypeDeduction(root, duplicateType(*(Type*)utarray_front(root->deducedTypes._types)));
		final = root->finalType;
	}

	if(root->child1 == NULL && root->child2 == NULL){
		// nothing to do here
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT || root->typ == PTT_DIV ||
			root->typ == PTT_SHR || root->typ == PTT_SHL ||
			root->typ == PTT_EXP || root->typ == PTT_AND || root->typ == PTT_OR || root->typ == PTT_AND || root->typ == PTT_NOT ||
			root->typ == PTT_XOR || root->typ == PTT_MOD  || root->typ == PTT_GT || root->typ == PTT_LT ||
      root->typ == PTT_GTE || root->typ == PTT_LTE || root->typ == PTT_ASSIGN){
		// both sides need be same type
		setFinalTypeDeduction((PTree*)root->child1, duplicateType(final));
		propagateTreeType((PTree*)root->child1);
		if(root->typ != PTT_NOT){
			setFinalTypeDeduction((PTree*)root->child2, duplicateType(final));
			propagateTreeType((PTree*)root->child2);
		}
	}else if(root->typ == PTT_EQUAL){
		if(root->child2 == NULL){
			propagateTreeType((PTree*)root->child1);
		}else{
			// both sides need be same type
			setFinalTypeDeduction((PTree*)root->child1, duplicateType(final));
			setFinalTypeDeduction((PTree*)root->child2, duplicateType(final));
			propagateTreeType((PTree*)root->child1);
			propagateTreeType((PTree*)root->child2);
		}

		// need to check if this was a not equals and if so, if not node recevied same type deduction
		if(root->parent != NULL && ((PTree*)root->parent)->typ == PTT_NOT){
			if(!typesEqualMostly(((PTree*)root->parent)->finalType, root->finalType)){
				setFinalTypeDeduction((PTree*)root->parent, duplicateType(root->finalType));
			}
		}
	}else if(root->typ == PTT_OBJECT_EQUAL_CHECK){
		// both sides need be same type
		setFinalTypeDeduction((PTree*)root->child1, duplicateType(final));
		setFinalTypeDeduction((PTree*)root->child2, duplicateType(final));
		propagateTreeType((PTree*)root->child1);
		propagateTreeType((PTree*)root->child2);
	}else if(root->typ == PTT_PARAM_CONT){ // function call
		PTree *param = (PTree*)root->child2;
		int pCount = 0;
		while(param != NULL){
			pCount ++;
			param = (PTree*)param->child2;
		}

		// first check globals
		PTree *id = (PTree*)root->child1;
		if(id->typ == PTT_IDENTIFIER && getFunctionVersions((char*)id->tok->extra) != NULL){
			NamedFunctionMapEnt *l = getFunctionVersions((char*)id->tok->extra);
			FunctionVersion *ver = l->V;
			while(ver != NULL){
				if(tryPropogateFuncSig(ver->sig, pCount, root)){
					markFunctionVersionUsed(ver);
					return;
				}
				ver = (FunctionVersion*)ver->next;
			}
		}

		// lambdas and temps:
		TypeDeductions callee = id->deducedTypes;
		Type *sig = NULL;
		while((sig=(Type*)utarray_next(callee._types,sig))){
			if(sig->base == TB_FUNCTION){
				if(tryPropogateFuncSig(*sig, pCount, root)){
					setFinalTypeDeduction(id, duplicateType(*sig));
					propagateTreeType(id);
					return;
				}
			}
		}
	}else if(root->typ == PTT_RETURN){
		setFinalTypeDeduction((PTree*)root->child1, duplicateType(final));
		propagateTreeType((PTree*)root->child1);
	}else if(root->typ == PTT_DECL){
		if(root->child2 != NULL){
			setFinalTypeDeduction((PTree*)root->child2, duplicateType(final));
			propagateTreeType((PTree*)root->child2);
		}
	}else if(root->typ == PTT_ARRAY_ELM){
		PTree *node = root;
		int i = 0;
		while(node != NULL){
			PTree *elm = (PTree*)node->child1;
			if(final.base == TB_DICT){
				Type tkey = duplicateType(((Type*)final.children)[0]);
				Type tval = duplicateType(((Type*)final.children)[1]);
				PTree *key = (PTree*)elm->child1;
				PTree *val = (PTree*)elm->child2;
				setFinalTypeDeduction(key, tkey);
				propagateTreeType(key);
				setFinalTypeDeduction(val, tval);
				propagateTreeType(val);
			}else if(final.base == TB_TUPLE){
				Type t = duplicateType(((Type*)final.children)[i]);
				setFinalTypeDeduction(elm, t);
				propagateTreeType(elm);
			}else{ // vector
				Type t = duplicateType(((Type*)final.children)[0]);
				setFinalTypeDeduction(elm, t);
				propagateTreeType(elm);
			}
			node = (PTree*)node->child2;
			i++;
		}
	}else if(root->typ == PTT_ARRAY_COMP){
		// we need to propagate source and eval expressions
		propagateTreeType((PTree*)((PTree*)root->child1)->child2);

		if(((PTree*)(PTree*)((PTree*)root->child2)->child1)->typ == PTT_ARRAY_ELM_PAIR){
		  propagateTreeType((PTree*)((PTree*)((PTree*)((PTree*)root)->child2)->child1)->child1);
		  propagateTreeType((PTree*)((PTree*)((PTree*)((PTree*)root)->child2)->child1)->child2);
		}else{
		  propagateTreeType((PTree*)((PTree*)root->child2)->child1);
		}

		if(((PTree*)root->child2)->child2 != NULL){ // condition
			propagateTreeType((PTree*)((PTree*)root->child2)->child2);
		}
	}else if(root->typ == PTT_ARRAY_ELM_PAIR){
		Type tkey = duplicateType(((Type*)final.children)[0]);
		Type tval = duplicateType(((Type*)final.children)[1]);
		setFinalTypeDeduction((PTree*)root->child1, tkey);
		propagateTreeType((PTree*)root->child1);
		setFinalTypeDeduction((PTree*)root->child2, tval);
		propagateTreeType((PTree*)root->child2);
	}else if(root->typ == PTT_LAMBDA){ // lambda function
		// nothing to do here
	}else if(root->typ == PTT_ARR_ACCESS){
		TypeDeductions lhs = ((PTree*)root->child1)->deducedTypes;

		Type *p = NULL;
		while((p=(Type*)utarray_next(lhs._types,p))){ // all posibilities
			if(p->base == TB_DICT){
				Type tmp = ((Type*)p->children)[1];
				if(typesEqualMostly(tmp, final)){ // success -- here's the key & value
					PTree *treeL = (PTree*)root->child1;
					PTree *treeR = (PTree*)root->child2;
					setFinalTypeDeduction(treeR, duplicateType(((Type*)p->children)[0]));
					setFinalTypeDeduction(treeL, duplicateType(*p));
					break;
				}
			}else{ // vector
				Type tmp = ((Type*)p->children)[0];
				if(typesEqualMostly(tmp, final)){ // success -- here's the key & value
					PTree *treeL = (PTree*)root->child1;
					PTree *treeR = (PTree*)root->child2;
					setFinalTypeDeduction(treeR, newBasicType(TB_NATIVE_INT));
					setFinalTypeDeduction(treeL, duplicateType(*p));
					break;
				}
			}
		}
		propagateTreeType((PTree*)root->child1);
		propagateTreeType((PTree*)root->child2);
	}else if(root->typ == PTT_DOT){
		// the correct return value has been chosen, so pick right key-value
		TypeDeductions lhs = ((PTree*)root->child1)->deducedTypes;
		PTree *rhs = (PTree*)root->child2;
		Type *p=NULL;
		while((p=(Type*)utarray_next(lhs._types,p))){
			int i;
			bool done=false;
			for(i=0; i<p->numchildren; i++){
				if(((Type*)p->children)[i].altName == NULL)
					continue;
				if(strcmp(((Type*)p->children)[i].altName, (char*)rhs->tok->extra)==0){
					if(typesEqualMostly(((Type*)p->children)[i], final)){ // found
						done = true;
						setFinalTypeDeduction((PTree*)root->child1, duplicateType(*p));
						break;
					}
				}
			}
			if(done)
				break;
		}
		propagateTreeType((PTree*)root->child1);
		// nothing on RHS except token
	}else if(root->typ == PTT_PUSH || root->typ == PTT_QUEUE || root->typ == PTT_CONTAINS || root->typ == PTT_REMOVE){
	  propagateTreeType((PTree*)root->child1);
	  propagateTreeType((PTree*)((PTree*)root->child2)->child2);
	}else if(root->typ == PTT_POP || root->typ == PTT_DEQUEUE || root->typ == PTT_SIZE ||
		 root->typ == PTT_KEYS || root->typ == PTT_VALUES){
	  propagateTreeType((PTree*)root->child1);
	}else{
		fatalError("No propagateTreeType for tree node!");
	}
}





bool semAnalyStmt(PTree *root, Type sig)
{
	if(root == NULL)
		return false;
	if(root->tok != NULL && root->tok->extra != NULL)
		printf("STMT: %s\n", (char*)root->tok->extra);
	int err = 0;
	if(root->typ == PTT_RETURN){
		TypeDeductions deduced = deduceTreeType((PTree*)root->child1, &err, CAST_UP);
		TypeDeductions required = singleTypeDeduction(((Type*)sig.children)[0]);
		TypeDeductions res = mergeTypeDeductionsOrErr(deduced, required, &err);
		freeTypeDeductions(required);
		setTypeDeductions(root, res);
		if(err > 0){
			reportError("SA004", "Return Type Invalid: Line %d", root->tok->lineNo);
			return false;
		}
		return finalizeSingleDeduction(root);
	}else if(root->typ == PTT_DECL){
	        Variable *addedSymb;
		if(!declaration(root, &err, &addedSymb))
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

		TypeDeductions booleanType = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
		TypeDeductions deduced = deduceTreeType(cond, &err, CAST_UP);

		TypeDeductions res = mergeTypeDeductionsOrErr(deduced, booleanType, &err);
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
	}else if(root->typ == PTT_WHILE){
		PTree *cond = (PTree*)root->child1;
		PTree *branch = (PTree*)root->child2;

		TypeDeductions booleanType = expandedTypeDeduction(newBasicType(TB_NATIVE_INT), CAST_UP);
		TypeDeductions deduced = deduceTreeType(cond, &err, CAST_UP);

		TypeDeductions res = mergeTypeDeductionsOrErr(deduced, booleanType, &err);
		freeTypeDeductions(booleanType);
		setTypeDeductions(root, res);
		setTypeDeductions(cond, duplicateTypeDeductions(res));
		if(err > 0 || !finalizeSingleDeduction(cond)){
			reportError("SA174", "While Condition Must Evaluate To Integer Type: Line %d", cond->tok->lineNo);
			return false;
		}

		bool ret = blockUnit(branch, sig, false);
		return ret;
	}else if(root->typ == PTT_FOR){
		PTree *cond = (PTree*)root->child1;
		PTree *branch = (PTree*)root->child2;
		PTree *decl = (PTree*)cond->child1;
		PTree *arr = (PTree*)cond->child2;

		// Declaration:
		enterNewScope();
		Variable *definedSymb = NULL;
		if(!declaration(decl, &err, &definedSymb) || !finalizeSingleDeduction(decl))
			return false;

		// Find out what type we are iterating/expected object to iterate
		Type typ = duplicateType(definedSymb->sig);
		Type typExpected = newVectorType(typ);
		TypeDeductions expected = singleTypeDeduction(typExpected);
		freeType(typExpected);

		// Iterated Object
		TypeDeductions iter = deduceTreeType(arr, &err, CAST_UP);

		TypeDeductions res = mergeTypeDeductionsOrErr(expected, iter, &err);
		freeTypeDeductions(expected);
		setTypeDeductions(root, res);
		setTypeDeductions(arr, duplicateTypeDeductions(res));
		if(err > 0 || !finalizeSingleDeduction(arr)){
			reportError("SA051", "Array Iteration Type Invalid: Line %d", root->tok->lineNo);
			return false;
		}
		return blockUnit(branch, sig, false);
	}else if(root->typ == PTT_ASSIGN){
		TypeDeductions lhs = deduceTreeType((PTree*)root->child1, &err, CAST_DOWN);
		TypeDeductions rhs = deduceTreeType((PTree*)root->child2, &err, CAST_UP);
		TypeDeductions res = mergeTypeDeductionsOrErr(lhs, rhs, &err);
		setTypeDeductions(root, res);
		if(err > 0){
			reportError("SA052", "Assignment Invalid: Line %d", root->tok->lineNo);
			return false;
		}
		return finalizeSingleDeduction(root);
	}else if(root->typ == PTT_FUNCTION){
		// functions here are named essentially lambdas mixed into a declaration
		// so we reorder the syntax tree as such

		// store original function type for later
		int paramCnt = 0;
		PTree *paramTypeTree = (PTree*)((PTree*)root->child1)->child2;
		while(paramTypeTree != NULL){
			paramCnt ++;
			paramTypeTree = (PTree*)paramTypeTree->child2;
		}

		Type sig = newBasicType(TB_FUNCTION);
		allocTypeChildren(&sig, paramCnt + 1);
		((Type*)sig.children)[0] = deduceTypeDeclType((PTree*)((PTree*)root->child1)->child1);

		paramTypeTree = (PTree*)((PTree*)root->child1)->child2;
		int i;
		for(i=1; i<=paramCnt; i++){
			PTree *pTempRoot = (PTree*)((PTree*)paramTypeTree->child1)->child1;
			((Type*)sig.children)[i] = deduceTypeDeclType(pTempRoot);
			paramTypeTree = (PTree*)paramTypeTree->child2;
		}

		PTree *decl = newParseTree(PTT_DECL);
		decl->parent = root->parent;
		((PTree*)decl->parent)->child1 = (void*)decl;

		decl->tok = root->tok; // move declaration symbol name to new node

		// old root is now the default value for declaration
		root->tok = duplicateAndPlaceAfterToken(decl->tok);
		root->typ = PTT_LAMBDA;
		decl->child2 = (void*)root;
		root->parent = (void*)decl;

		// now we need deduce the type of declaration
		PTree *declType = newParseTree(PTT_DECL_TYPE_DEDUCED);
		declType->parent = (void*)decl;
		decl->child1 = (void*)declType;
		setTypeDeductions(declType, singleTypeDeduction(sig));
		freeType(sig);

		return semAnalyStmt(decl, sig); // not start over with declaration & lambda
	}else if(root->typ == PTT_PUSH || root->typ == PTT_QUEUE || root->typ == PTT_POP || root->typ == PTT_DEQUEUE
		 || root->typ == PTT_REMOVE){ // these can be statements or expressions
		TypeDeductions td = deduceTreeType(root, &err, CAST_DOWN);
		if(err > 0){
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
		enterNewScope();
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
	if(!global){
	        enterGlobalSpace();
		enterNewScope();
		// need to push locals onto stack
		PTree *paramList = (PTree*)((PTree*)root->child1)->child2;
		int i;
		for(i=1; i<sig.numchildren; i++){
			defineVariable((char*)((PTree*)paramList->child1)->tok->extra, duplicateType(((Type*)sig.children)[i]));
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
