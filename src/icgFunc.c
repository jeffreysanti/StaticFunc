/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
*  icgFunc.c
*  Intermediate Code Generator : Function Calls & Function Creation
*
*/

#include "icg.h"
#include "symbols.h"
#include "scopeprocessor.h"



ICGElm *icGenPackLambdaContainer(ICGElm *prev, PackedLambdaData *packedLambda, Variable *executerAddr, Scope *holdingMethodScope, Variable **objPtrRet){
	
	// Allocate memory for lambda holder
	Variable *objPtr = defineUnattachedVariable(newBasicType(TB_FUNCTION));
	prev = newICGElm(prev, ICG_ALLOC, ICGDT_PTR, NULL);
	prev->op1 = newOpInt(packedLambda->size);
	prev->result = newOp(ICGO_REG, objPtr);

	// first load the executer address (offset 0)
	if(executerAddr != NULL){
		prev = newICGElm(prev, ICG_STOREH, ICGDT_PTR, NULL);
		prev->op1 = newOp(ICGO_REG, executerAddr);
		prev->op2 = newOpInt(0);
		prev->result = newOp(ICGO_REG, objPtr);
	}

	// now leave space for every variable
	// [Intentionally Left Blank]

	// === now pack addresses of scopes accessed ===

	// load address of current methodscope
	// Note: Non-Lambdas don't have an executer address and have no previous scope, so there's nothing to pack here.
	if(executerAddr != NULL){
		Variable *currentScopeAddr = defineUnattachedVariable(newBasicType(TB_FUNCTION));
		prev = newICGElm(prev, ICG_METHOD_PTR, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, currentScopeAddr);

		int scopesPacked = 0;
		while(scopesPacked < utarray_len(packedLambda->spaces)){
			Scope **s;

			// Determine if this scope needs to be packed. If so, we have the address
			for(s=(Scope**)utarray_front(packedLambda->spaces); s!=NULL; s=(Scope**)utarray_next(packedLambda->spaces,s)) {
				if(*s == holdingMethodScope){
					int idx = utarray_eltidx(packedLambda->spaces, s);
					int offset = *((int*)utarray_eltptr(packedLambda->spaces_offsets, idx));

					// Pack it
					prev = newICGElm(prev, ICG_STOREH, ICGDT_PTR, NULL);
					prev->op1 = newOp(ICGO_REG, currentScopeAddr);
					prev->op2 = newOpInt(offset);
					prev->result = newOp(ICGO_REG, objPtr);

					scopesPacked ++;
				}
			}

			if(scopesPacked >= utarray_len(packedLambda->spaces)){
				break; // don't want to load more memory..
			}

			// now get the next scope address
			Variable *tmp = defineUnattachedVariable(newBasicType(TB_FUNCTION));
			prev = newICGElm(prev, ICG_LOADH, ICGDT_PTR, NULL);
			prev->result = newOp(ICGO_REG, currentScopeAddr);
			prev->op1 = newOp(ICGO_REG, currentScopeAddr);
			prev->op2 = newOpInt(8); // 8 bytes into it, is the previous entry offset
		}
	}

	*objPtrRet = objPtr;
	return prev;
}



ICGElm *icGenCall(PTree *root, ICGElm *prev){

	// obtain all params
	UT_array *pushList;
	utarray_new(pushList, &ut_ptr_icd);
	PTree *param = (PTree*)root->child2;
	while(param != NULL){
		prev = icGen((PTree*)param->child1, prev);

		ICGElmOp *result = prev->result;
		utarray_push_back(pushList, &result);

		param = (PTree*)param->child2;
	}

	// we need to determine if we are calling a lambda or a method
	if(root->funcCallVer != NULL){
		// call a non-lambda function
		FunctionVersion *func = root->funcCallVer;

		// We need to build a lambda holder for this call
		// unlike for lambdas which are created in the execution context, these are created at call time.
		// so, we don't really need to use the 8 byte code pointer
		// now build the lambda pointer
		PackedLambdaData *packedLambda = getNodeScope(func->defRoot)->packedLambdaSpace;
		Variable *packedLambdaVar = NULL;
		prev = icGenPackLambdaContainer(prev, packedLambda, NULL, NULL, &packedLambdaVar);

		// mark this lambda container as active
		prev = newICGElm(prev, ICG_WRITE_METHOD_PTR, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, packedLambdaVar);

		// now output the param vars
		ICGElmOp **v;
		int idx = -1;
		for(v=(ICGElmOp**)utarray_front(pushList); v!=NULL; v=(ICGElmOp**)utarray_next(pushList,v)) {
			ICGElmOp *dup = newOpCopy(*v);
			prev = newICGElm(prev, ICG_PUSH, ICGDT_NONE, NULL);
			prev->result = dup;
		}

		// and jump
		prev = newICGElm(prev, ICG_JAL, ICGDT_NONE, root);
		prev->result = newOpLabel_c(func->icgEntryLabel);
	}else{
		// lambda function
		// identifier contains a pointer to the code
		prev = icGen((PTree*)root->child1, prev);
		ICGElmOp *varop = newOpCopy(prev->result);

		// load execution pointer
		Variable *execPtr = defineUnattachedVariable(newBasicType(TB_FUNCTION));
		prev = newICGElm(prev, ICG_LOADH, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, execPtr);
		prev->op1 = varop;
		prev->op2 = newOpInt(0);

		prev = newICGElm(prev, ICG_WRITE_METHOD_PTR, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, execPtr);

		// now output the param vars
		ICGElmOp **v;
		int idx = -1;
		for(v=(ICGElmOp**)utarray_front(pushList); v!=NULL; v=(ICGElmOp**)utarray_next(pushList,v)) {
			ICGElmOp *dup = newOpCopy(*v);
			prev = newICGElm(prev, ICG_PUSH, ICGDT_NONE, NULL);
			prev->result = dup;
		}

		// and jump
		prev = newICGElm(prev, ICG_JAL, ICGDT_NONE, root);
		prev->result = newOp(ICGO_REG, execPtr);
	}

	// Return : If we got data back load it
	Variable *funcRet = defineUnattachedVariable(root->finalType);
	prev = newICGElm(prev, ICG_LOADRET, typeToICGDataType(root->finalType), NULL);
	prev->result = newOp(ICGO_REG, funcRet);

	utarray_free(pushList);
	return prev;
}




ICGElm *icGenLambda(PTree *root, ICGElm *prev){
	char *execLabel = icRunGenFunction(root, FUNCNAME_LAMBDA, NULL);

	// First load the executer address: We'll need this to pack later
	Variable *executerAddr = defineUnattachedVariable(newBasicType(TB_FUNCTION));
	prev = newICGElm(prev, ICG_LABEL_ADDR, ICGDT_PTR, root);
	prev->op1 = newOpLabel_c(execLabel);
	prev->result = newOp(ICGO_REG, executerAddr);

	// now build the lambda pointer
	Scope *holdingMethodScope = getMethodScope(getNodeScope((PTree*)root->parent));
	PackedLambdaData *packedLambda = getMethodScope(getNodeScope(root))->packedLambdaSpace;

	Variable *packedLambdaVar = NULL;
	prev = icGenPackLambdaContainer(prev, packedLambda, executerAddr, holdingMethodScope, &packedLambdaVar);
	
	// leave the address for the next instruction
	prev = newICGElm(prev, ICG_NONE, ICGDT_PTR, NULL);
	prev->result = newOp(ICGO_REG, packedLambdaVar);

	return prev;
}

ICGElm *icGenRet(PTree *root, ICGElm *prev){
	if (root->child1 != NULL)
	{
		prev = icGen((PTree *)root->child1, prev);
		ICGElmOp *op = newOpCopy(prev->result);

		prev = newICGElm(prev, ICG_RET, typeToICGDataType(root->finalType), root);
		prev->result = op;
	}
	else
	{
		prev = newICGElm(prev, ICG_RET, ICGDT_NONE, root);
	}
	return prev;
}

void icGenRetCall_print(ICGElm *elm, FILE *f){

// elm->typ == ICG_RET || elm->typ == ICG_CALL || elm->typ == ICG_LABEL_ADDR || elm->typ == ICG_METHOD_PTR


	if(elm->typ == ICG_RET){
		fprintf(f, "ret");
	}else if(elm->typ == ICG_METHOD_PTR){
		fprintf(f, "mptr");
	}else if(elm->typ == ICG_WRITE_METHOD_PTR){
		fprintf(f, "wmptr");
	}else if(elm->typ == ICG_LABEL_ADDR){
		fprintf(f, "laddr");
	}else if(elm->typ == ICG_LOADRET){
		fprintf(f, "lret");
	}else if(elm->typ == ICG_PUSH){
		fprintf(f, "push");
	}else if(elm->typ == ICG_POP){
		fprintf(f, "pop");
	}

	if(elm->result != NULL){
		fprintf(f, " ");
		printOp(f, elm->result);
	}
	if(elm->op1 != NULL){
		fprintf(f, ", ");
		printOp(f, elm->op1);
	}
}
