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
	char *tempVar = newTempVariable(root->finalType);

	ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));
	ICGElmOp *op1 = newOpCopyData(ICGO_RO_ADDR, roName);

	prev = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, root);
	prev->result = res;
	prev->op1 = op1;
	return prev;
}

static inline ICGElmOp *bitSizeTupleOp(Type t){
	char *dta = calloc(t.numchildren*2 + 1, 1);
	char *origDta = dta;
	int i;
	for(i=0; i<t.numchildren; i++){
		Type child = ((Type*)t.children)[i];
		if(child.base == TB_NATIVE_INT8){
			sprintf(dta, "1;");
			dta += 2;
		}
		else if(child.base == TB_NATIVE_INT16){
			sprintf(dta, "2;");
			dta += 2;
		}
		else if(child.base == TB_NATIVE_INT32 || child.base == TB_NATIVE_FLOAT32){
			sprintf(dta, "4;");
			dta += 2;
		}
		else if(child.base == TB_NATIVE_INT64 || child.base == TB_NATIVE_FLOAT64){
			sprintf(dta, "8;");
			dta += 2;
		}
		else{
			sprintf(dta, "P;");
			dta += 2;
		}
	}
	return newOp(ICGO_NUMERICLIT, origDta);
}

static inline ICGElmOp *bitSizeOp(Type t){
	ICGElmOp *op = NULL;
	if(t.base == TB_NATIVE_INT8) op = newOpCopyData(ICGO_NUMERICLIT, "1");
	else if(t.base == TB_NATIVE_INT16) op = newOpCopyData(ICGO_NUMERICLIT, "2");
	else if(t.base == TB_NATIVE_INT32 || t.base == TB_NATIVE_FLOAT32)
		op = newOpCopyData(ICGO_NUMERICLIT, "4");
	else if(t.base == TB_NATIVE_INT64 || t.base == TB_NATIVE_FLOAT64)
		op = newOpCopyData(ICGO_NUMERICLIT, "8");
	else
		op = newOpCopyData(ICGO_NUMERICLIT, "PTR");
	return op;
}

ICGElm * icGenArray(PTree *root, ICGElm *prev){
	int elmCnt = 0;
	PTree *ptr = root;
	while(ptr != NULL){
		elmCnt ++;
		ptr = (PTree*)ptr->child2;
	}

	if(root->finalType.base == TB_VECTOR || root->finalType.base == TB_VECTOR){
		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));

		Type childType = ((Type*)root->finalType.children)[0];
		ICGElmOp *op1 = bitSizeOp(childType);

		ICGElmOp *op2 = newOpInt(ICGO_NUMERICLIT, elmCnt);

		if(root->finalType.base == TB_VECTOR){
			prev = newICGElm(prev, ICG_NEWVEC, typeToICGDataType(root->finalType), root);
		}else{
			prev = newICGElm(prev, ICG_NEWSET, typeToICGDataType(root->finalType), root);
		}
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;
			char *elmTemp = newTempVariable(elm->finalType);
			prev = icGenAssnToX(elm, prev, elmTemp, elm->finalType);

			if(root->finalType.base == TB_VECTOR){
				ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));
				ICGElmOp *op1 = newOpInt(ICGO_NUMERICLIT, i);
				ICGElmOp *op2 = newOpCopyData(prev->result->typ, prev->result->data);

				prev = newICGElm(prev, ICG_VECSTORE, typeToICGDataType(elm->finalType), elm);
				prev->result = res;
				prev->op1 = op1;
				prev->op2 = op2;
			}else{
				ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));
				ICGElmOp *op1 = newOpCopyData(prev->result->typ, prev->result->data);

				prev = newICGElm(prev, ICG_SETSTORE, typeToICGDataType(elm->finalType), elm);
				prev->result = res;
				prev->op1 = op1;
			}

			ptr = (PTree*)ptr->child2;
		}
	}else if(root->finalType.base == TB_DICT){
		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));

		Type childTypeKey = ((Type*)root->finalType.children)[0];
		ICGElmOp *op1 = bitSizeOp(childTypeKey);

		Type childTypeVal = ((Type*)root->finalType.children)[1];
		ICGElmOp *op2 = bitSizeOp(childTypeVal);

		ICGElmOp *op3 = newOpInt(ICGO_NUMERICLIT, elmCnt);

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
			ICGElmOpType optkey = prev->result->typ;

			PTree *val = (PTree*)elm->child2;
			char *elmValTemp = newTempVariable(val->finalType);
			prev = icGenAssnToX(val, prev, elmValTemp, val->finalType);
			ICGElmOpType optval = prev->result->typ;

			ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));
			ICGElmOp *op1 = newOp(optkey, getSymbolUniqueName(elmKeyTemp));
			ICGElmOp *op2 = newOp(optval, getSymbolUniqueName(elmValTemp));

			prev = newICGElm(prev, ICG_DICTSTORE, typeToICGDataType(elm->finalType), elm);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;

			ptr = (PTree*)ptr->child2;
		}
	}else if(root->finalType.base == TB_TUPLE){
		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));

		ICGElmOp *op1 = newOpInt(ICGO_NUMERICLIT, elmCnt);
		ICGElmOp *op2 = bitSizeTupleOp(root->finalType);

		prev = newICGElm(prev, ICG_NEWTUPLE, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;

			char *elmTemp = newTempVariable(elm->finalType);
			prev = icGenAssnToX(elm, prev, elmTemp, elm->finalType);

			ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));
			ICGElmOp *op1 = newOpInt(ICGO_NUMERICLIT, i);
			ICGElmOp *op2 = newOpCopyData(prev->result->typ, prev->result->data);

			prev = newICGElm(prev, ICG_TPLSTORE, typeToICGDataType(elm->finalType), elm);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;

			ptr = (PTree*)ptr->child2;
		}
	}
	return prev;
}

ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, char *reg){

	if(prev->result->typ == ICGO_OBJREFNEW){
		fprintf(stderr, "icGenCopyObject called on new object!\n");
	}

	ICGElmOp *op1 = newOpCopyData(ICGO_OBJREF, reg);

	char *tempVar = newTempVariable(root->finalType);
	ICGElmOp *res = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(tempVar));

	ICGElm *ret = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, root);
	ret->result = res;
	ret->op1 = op1;

	return ret;
}

void icGenObjCpy_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "ocpy");
	fprintf(f, " $%s, ", elm->result->data);
	if(elm->op1->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", elm->op1->data);
	}else{
		fprintf(f, "$%s", elm->op1->data);
	}
}

void icGenArray_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_NEWVEC){
		fprintf(f, "newvec $%s, %s, %s", elm->result->data, elm->op1->data, elm->op2->data);
	}if(elm->typ == ICG_NEWSET){
		fprintf(f, "newset $%s, %s, %s", elm->result->data, elm->op1->data, elm->op2->data);
	}else if(elm->typ == ICG_NEWDICT){
		fprintf(f, "newdict $%s, %s, %s, %s", elm->result->data, elm->op1->data, elm->op2->data,
				elm->op3->data);
	}else if(elm->typ == ICG_NEWTUPLE){
		fprintf(f, "newtuple $%s, %s, %s", elm->result->data, elm->op1->data, elm->op2->data);
	}else if(elm->typ == ICG_VECSTORE){
		fprintf(f, "vst");
		printICGTypeSuffix(elm, f);
		fprintf(f, " $%s, %s, $%s", elm->result->data, elm->op1->data, elm->op2->data);
	}else if(elm->typ == ICG_SETSTORE){
		fprintf(f, "sst");
		printICGTypeSuffix(elm, f);
		fprintf(f, " $%s, $%s", elm->result->data, elm->op1->data);
	}else if(elm->typ == ICG_DICTSTORE){
		fprintf(f, "dst");
		printICGTypeSuffix(elm, f);
		fprintf(f, " $%s, $%s, $%s", elm->result->data, elm->op1->data, elm->op2->data);
	}else if(elm->typ == ICG_TPLSTORE){
		fprintf(f, "tst");
		printICGTypeSuffix(elm, f);
		ICGElmOp *op2 = elm->op2;
		fprintf(f, " $%s, %s, $%s", elm->result->data, elm->op1->data, op2->data);
	}
}
