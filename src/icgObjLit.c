/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgObjLit.c
 *  Intermediate Code Generator : Handle Object Literals (Strings / lists]
 *
 */

#include "icg.h"

extern ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, char *to, Type assignType);

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

inline ICGElmOp *bitSizeOp(Type t){
	ICGElmOp *op = NULL;
	if(t.base == TB_NATIVE_INT8) op = newOpCopyData(ICGO_LIT, "1");
	else if(t.base == TB_NATIVE_INT16) op = newOpCopyData(ICGO_LIT, "2");
	else if(t.base == TB_NATIVE_INT32 || t.base == TB_NATIVE_FLOAT32)
		op = newOpCopyData(ICGO_LIT, "4");
	else if(t.base == TB_NATIVE_INT64 || t.base == TB_NATIVE_FLOAT64)
		op = newOpCopyData(ICGO_LIT, "8");
	else
		op = newOpCopyData(ICGO_LIT, "PTR");
	return op;
}

ICGElm * icGenArray(PTree *root, ICGElm *prev){
	int elmCnt = 0;
	PTree *ptr = root;
	while(ptr != NULL){
		elmCnt ++;
		ptr = (PTree*)ptr->child2;
	}

	if(root->finalType.base == TB_VECTOR){
		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

		Type childType = ((Type*)root->finalType.children)[0];
		ICGElmOp *op1 = bitSizeOp(childType);

		ICGElmOp *op2 = newOpInt(ICGO_LIT, elmCnt);

		prev = newICGElm(prev, ICG_NEWVEC, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;
			char *elmTemp = newTempVariable(root->finalType);
			prev = icGenAssnToX(elm, prev, elmTemp, elm->finalType);

			ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

			ICGElmOp *op1 = newOpInt(ICGO_LIT, i);

			ICGElmOp *op2 = newOp(ICGO_IDENT, elmTemp);
			ICGElmOp *op2b = newOp(ICGO_REG, getSymbolUniqueName(elmTemp));

			prev = newICGElm(prev, ICG_VECSTORE, typeToICGDataType(elm->finalType), elm);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;
			prev->op2b = op2b;

			ptr = (PTree*)ptr->child2;
		}
	}else if(root->finalType.base == TB_DICT){
		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

		Type childTypeKey = ((Type*)root->finalType.children)[0];
		ICGElmOp *op1 = bitSizeOp(childTypeKey);

		Type childTypeVal = ((Type*)root->finalType.children)[1];
		ICGElmOp *op2 = bitSizeOp(childTypeVal);

		ICGElmOp *op3 = newOpInt(ICGO_LIT, elmCnt);

		prev = newICGElm(prev, ICG_NEWDICT, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;
		prev->op3 = op3;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;

			PTree *key = (PTree*)elm->child1;
			char *elmKeyTemp = newTempVariable(key->finalType);
			prev = icGenAssnToX(key, prev, elmKeyTemp, key->finalType);

			PTree *val = (PTree*)elm->child2;
			char *elmValTemp = newTempVariable(val->finalType);
			prev = icGenAssnToX(val, prev, elmValTemp, val->finalType);

			ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

			ICGElmOp *op1 = newOp(ICGO_IDENT, elmKeyTemp);
			ICGElmOp *op1b = newOp(ICGO_REG, getSymbolUniqueName(elmKeyTemp));

			ICGElmOp *op2 = newOp(ICGO_IDENT, elmValTemp);
			ICGElmOp *op2b = newOp(ICGO_REG, getSymbolUniqueName(elmValTemp));

			prev = newICGElm(prev, ICG_DICTSTORE, typeToICGDataType(elm->finalType), elm);
			prev->result = res;
			prev->op1 = op1;
			prev->op1b = op1b;
			prev->op2 = op2;
			prev->op2b = op2b;

			ptr = (PTree*)ptr->child2;
		}
	}
	return prev;
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

void icGenArray_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_NEWVEC){
		fprintf(f, "newvec $%s, %s, %s", elm->result->data, elm->op1->data, elm->op2->data);
	}else if(elm->typ == ICG_NEWDICT){
		fprintf(f, "newdict $%s, %s, %s, %s", elm->result->data, elm->op1->data, elm->op2->data,
				elm->op3->data);
	}else if(elm->typ == ICG_VECSTORE){
		fprintf(f, "vst");
		printICGTypeSuffix(elm, f);
		fprintf(f, " $%s, %s, $%s", elm->result->data, elm->op1->data, elm->op2b->data);
	}else if(elm->typ == ICG_DICTSTORE){
		fprintf(f, "dst");
		printICGTypeSuffix(elm, f);
		fprintf(f, " $%s, $%s, $%s", elm->result->data, elm->op1b->data, elm->op2b->data);
	}
}


