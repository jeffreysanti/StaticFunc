/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Declarations
 *
 */

#include "icg.h"

extern ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, char *to, Type assignType);


ICGElm * icGenDecl(PTree *root, ICGElm *prev){
	Type d = root->finalType;
	addSymbol((char*)root->tok->extra, duplicateType(d));

	char *ident = (char*)root->tok->extra;

	if(d.base == TB_NATIVE_INT8 || d.base == TB_NATIVE_BOOL){
		prev = newICGElm(prev, ICG_DEFINE_INT8, root);
		prev->resultb = newOp(ICGO_VAR_INT8, getSymbolUniqueName(ident));
	}else if(d.base == TB_NATIVE_INT16){
		prev = newICGElm(prev, ICG_DEFINE_INT16, root);
		prev->resultb = newOp(ICGO_VAR_INT16, getSymbolUniqueName(ident));
	}else if(d.base == TB_NATIVE_INT32){
		prev = newICGElm(prev, ICG_DEFINE_INT32, root);
		prev->resultb = newOp(ICGO_VAR_INT32, getSymbolUniqueName(ident));
	}else if(d.base == TB_NATIVE_INT64){
		prev = newICGElm(prev, ICG_DEFINE_INT64, root);
		prev->resultb = newOp(ICGO_VAR_INT64, getSymbolUniqueName(ident));
	}else if(d.base == TB_NATIVE_FLOAT32){
		prev = newICGElm(prev, ICG_DEFINE_FLOAT32, root);
		prev->resultb = newOp(ICGO_VAR_FLOAT32, getSymbolUniqueName(ident));
	}else if(d.base == TB_NATIVE_FLOAT64){
		prev = newICGElm(prev, ICG_DEFINE_FLOAT64, root);
		prev->resultb = newOp(ICGO_VAR_FLOAT64, getSymbolUniqueName(ident));
	}else{
		prev = newICGElm(prev, ICG_DEFINE_PTR, root);
		prev->resultb = newOp(ICGO_VAR_PTR, getSymbolUniqueName(ident));
	}
	prev->result = newOp(ICGO_IDENT, ident);

	if(root->child2 != NULL){
		prev = icGenAssnToX((PTree*)root->child2, prev, (char*)root->tok->extra, d);
	}

	return prev;
}

void icGenDecl_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_DEFINE_INT8){
		fprintf(f, "di8");
	}else if(elm->typ == ICG_DEFINE_INT16){
		fprintf(f, "di16");
	}else if(elm->typ == ICG_DEFINE_INT32){
		fprintf(f, "di32");
	}else if(elm->typ == ICG_DEFINE_INT64){
		fprintf(f, "di64");
	}else if(elm->typ == ICG_DEFINE_FLOAT32){
		fprintf(f, "df32");
	}else if(elm->typ == ICG_DEFINE_FLOAT64){
		fprintf(f, "df64");
	}else{
		fprintf(f, "dptr");
	}
	fprintf(f, " $%s", elm->resultb->data);
}

