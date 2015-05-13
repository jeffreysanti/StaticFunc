/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgArith.c
 *  Intermediate Code Generator : Arithmetic
 *
 */

#include "icg.h"
/*
typedef enum{
	DT_LIT,
	DT_IDENT,
	DT_EXPR
}DataType;

ICGElm * icGenArith(PTree *root, ICGElm *prev){
	Type d = root->finalType;
	char *tempVar = newTempVariable(d);
	char *dest = getSymbolUniqueName(tempVar);

	DataType op1 = DT_LIT;
	DataType op2 = DT_LIT;

	ICGElm *data1 = icGen((PTree*)root->child1, prev);
	if(data1->typ == ICG_LITERAL || data1->typ == ICG_IDENT){
		freeICGElm(data1);
		if(data1->typ == ICG_IDENT)
			op1 = DT_IDENT;
	}else{
		prev = data1;
		op1 = DT_EXPR;
	}

	ICGElm *data2 = icGen((PTree*)root->child2, prev);
	if(data2->typ == ICG_LITERAL || data2->typ == ICG_IDENT){
		freeICGElm(data2);
		if(data2->typ == ICG_IDENT)
			op2 = DT_IDENT;
	}else{
		prev = data2;
		op2 = DT_EXPR;
	}

	addSymbol((char*)root->tok->extra, duplicateType(d));
	char *unq_nm = getSymbolUniqueName((char*)root->tok->extra);
	if(d.base == TB_NATIVE_INT8 || d.base == TB_NATIVE_BOOL){
		prev = newICGElm(prev, ICG_DEFINE_INT8, root);
		prev->result = unq_nm;
	}else if(d.base == TB_NATIVE_INT16){
		prev = newICGElm(prev, ICG_DEFINE_INT16, root);
		prev->result = unq_nm;
	}else if(d.base == TB_NATIVE_INT32){
		prev = newICGElm(prev, ICG_DEFINE_INT32, root);
		prev->result = unq_nm;
	}else if(d.base == TB_NATIVE_INT64){
		prev = newICGElm(prev, ICG_DEFINE_INT64, root);
		prev->result = unq_nm;
	}else if(d.base == TB_NATIVE_FLOAT32){
		prev = newICGElm(prev, ICG_DEFINE_FLOAT32, root);
		prev->result = unq_nm;
	}else if(d.base == TB_NATIVE_FLOAT64){
		prev = newICGElm(prev, ICG_DEFINE_FLOAT64, root);
		prev->result = unq_nm;
	}else{
		prev = newICGElm(prev, ICG_DEFINE_PTR, root);
		prev->result = unq_nm;
	}

	if(root->child2 != NULL){
		prev = icGenAssnToX((PTree*)root->child2, prev, (char*)root->tok->extra, d);
	}

	return prev;
}

void icGenArtih_free(ICGElm *elm){
	free(elm->result);
	free(elm->op1);
}

void icGenArith_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_MOVL_INT8){
		fprintf(f, "movli8 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_INT16){
		fprintf(f, "movli16 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_INT32){
		fprintf(f, "movli32 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_INT64){
		fprintf(f, "movli64 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_FLOAT32){
		fprintf(f, "movlf32 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_FLOAT64){
		fprintf(f, "movlf64 %s, %s", (char*)elm->result, (char*)elm->op1);
	}
}*/

