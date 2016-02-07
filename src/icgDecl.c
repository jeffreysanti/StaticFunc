/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Declarations
 *
 */

#include "icg.h"

extern ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, Variable *to, Type assignType);


ICGElm * icGenDecl(PTree *root, ICGElm *prev){
	Type d = root->finalType;
	Variable *var = defineVariable((char*)root->tok->extra, d);

	prev = newICGElm(prev, ICG_DEFINE, typeToICGDataType(d), root);
	prev->result = newOp(ICGO_REG, var);

	if(root->child2 != NULL){
		prev = icGenAssnToX((PTree*)root->child2, prev, var, d);
	}
	return prev;
}

void icGenDecl_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "d");
	printICGTypeSuffix(elm, f);
	fprintf(f, " ");
	printOp(f, elm->result);
}

