/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Assignments
 *
 */

#include "icg.h"

ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, char *to, Type assignType){

	ICGElmOp *result = newOp(ICGO_IDENT, to);

	char *unqsym = getSymbolUniqueName(to);
	ICGElmOp *resultb = newOpVariable(assignType, unqsym);
	free(unqsym);

	ICGElm *data = icGen(root, prev);
	if(data->typ == ICG_LITERAL){
		freeICGElm(data);
		ICGElmOp *op1 = newOpLiteral(assignType, (char *)root->tok->extra);
		prev = newICGElm(prev, ICG_MOV, (PTree*)root->parent);
		prev->result = result;
		prev->resultb = resultb;
		prev->op1 = op1;
	}else if(data->typ == ICG_IDENT){
		freeICGElm(data);
		ICGElmOp *op1 = newOp(ICGO_IDENT, (char*)root->tok->extra);

		char *unqsym = getSymbolUniqueName((char*)root->tok->extra);
		ICGElmOp *op1b = newOpVariable(assignType, unqsym);
		free(unqsym);

		prev = newICGElm(prev, ICG_MOV, (PTree*)root->parent);
		prev->result = result;
		prev->resultb = resultb;
		prev->op1 = op1;
		prev->op1b = op1b;
	}else{
		prev = data;

		ICGElmOp *op1 = newOpVariable(assignType, prev->result->data);
		prev = newICGElm(prev, ICG_MOV, (PTree*)root->parent);
		prev->result = result;
		prev->resultb = resultb;
		prev->op1 = op1;
	}
	return prev;
}

void icGenAssn_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "mov");
	ICGElmOp *op1 = elm->op1;
	if(op1->typ == ICGO_IDENT)
		op1 = elm->op1b;
	if(op1->typ == ICGO_LIT_INT8 || op1->typ == ICGO_VAR_INT8)       fprintf(f, "i8 ");
	if(op1->typ == ICGO_LIT_INT16 || op1->typ == ICGO_VAR_INT16)     fprintf(f, "i16 ");
	if(op1->typ == ICGO_LIT_INT32 || op1->typ == ICGO_VAR_INT32)     fprintf(f, "i32 ");
	if(op1->typ == ICGO_LIT_INT64 || op1->typ == ICGO_VAR_INT64)     fprintf(f, "i64 ");
	if(op1->typ == ICGO_LIT_FLOAT32 || op1->typ == ICGO_VAR_FLOAT32) fprintf(f, "f32 ");
	if(op1->typ == ICGO_LIT_FLOAT64 || op1->typ == ICGO_VAR_FLOAT64) fprintf(f, "f64 ");
	if(op1->typ == ICGO_VAR_PTR)                                     fprintf(f, " ");

	fprintf(f, "$%s, ", elm->resultb->data);

	if(literalOp(op1)){
		fprintf(f, "%s", op1->data);
	}else{
		fprintf(f, "$%s", op1->data);
	}
}
