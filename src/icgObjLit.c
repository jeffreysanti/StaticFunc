/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgObjLit.c
 *  Intermediate Code Generator : Handle Object Literals (Strings / lists]
 *
 */

#include "icg.h"

ICGElm * icGenStringLit(PTree *root, ICGElm *prev){
	char *roName = newROStringLit((char*)root->tok->extra);
	ICGElmOp *op1 = newOpCopyData(ICGO_RO_ADDR, roName);

	char *tempVar = newTempVariable(root->finalType);
	ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

	ICGElm *ret = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, root);
	ret->result = res;
	ret->op1 = op1;

	return ret;
}

ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, char *reg){
	ICGElmOp *op1 = newOpCopyData(ICGO_REG, reg);

	char *tempVar = newTempVariable(root->finalType);
	ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

	ICGElm *ret = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, root);
	ret->result = res;
	ret->op1 = op1;

	return ret;
}

void icGenObjCpy_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "ocpy");
	fprintf(f, " $%s, ", elm->result->data);
	if(elm->op1->typ == ICGO_REG){
		fprintf(f, "$%s", elm->op1->data);
	}else if(elm->op1->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", elm->op1->data);
	}
}


