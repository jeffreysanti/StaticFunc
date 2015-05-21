/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgArith.c
 *  Intermediate Code Generator : Arithmetic
 *
 */

#include "icg.h"

extern ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, char *reg);


inline ICGElm * processChild(PTree *child1, ICGElm *prev, ICGElmOp **op){
	ICGElm *data1 = icGen(child1, prev);
	*op = NULL;
	if(data1->typ == ICG_LITERAL){
		freeICGElm(data1);
		*op = newOpCopyData(ICGO_NUMERICLIT, (char*)child1->tok->extra);
	}else if(data1->typ == ICG_IDENT){
		freeICGElm(data1);
		if(isTypeNumeric(child1->finalType)){
			*op = newOp(ICGO_NUMERICREG, getSymbolUniqueName((char*)child1->tok->extra));
		}else{
			char *tmpreg = getSymbolUniqueName((char*)child1->tok->extra);
			prev = icGenCopyObject(child1, prev, tmpreg);
			free(tmpreg);
			*op = newOpCopyData(ICGO_NUMERICREG, prev->result->data);
		}
	}else{ // expression (other code before this)
		prev = data1;
		*op = newOpCopyData(ICGO_NUMERICREG, prev->result->data);
	}
	return prev;
}

ICGElm * icGenArith(PTree *root, ICGElm *prev){
	Type d = root->finalType;

	char *tempVar = newTempVariable(d);
	ICGElmOp *res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));

	ICGElmOp *op1, *op2;
	prev = processChild((PTree*)root->child1, prev, &op1);
	prev = processChild((PTree*)root->child2, prev, &op2);

	ICGElmType arithType = ICG_NONE;
	if(root->typ == PTT_ADD) arithType = ICG_ADD;
	if(root->typ == PTT_SUB) arithType = ICG_SUB;
	if(root->typ == PTT_MULT) arithType = ICG_MUL;
	if(root->typ == PTT_DIV) arithType = ICG_DIV;

	prev = newICGElm(prev, arithType, typeToICGDataType(d), root);
	prev->result = res;
	prev->op1 = op1;
	prev->op2 = op2;

	return prev;
}


void icGenArith_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_ADD) fprintf(f, "add");
	if(elm->typ == ICG_MUL) fprintf(f, "mul");
	if(elm->typ == ICG_DIV) fprintf(f, "div");
	if(elm->typ == ICG_SUB) fprintf(f, "sub");
	printICGTypeSuffix(elm, f);

	fprintf(f, " $%s, ", elm->result->data);

	ICGElmOp *op = elm->op1;
	if(op->typ == ICGO_NUMERICLIT){
		fprintf(f, "%s", op->data);
	}else if(op->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", op->data);
	}else{
		fprintf(f, "$%s", op->data);
	}

	fprintf(f, ", ");
	op = elm->op2;
	if(op->typ == ICGO_NUMERICLIT){
		fprintf(f, "%s", op->data);
	}else if(op->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", op->data);
	}else{
		fprintf(f, "$%s", op->data);
	}
}
