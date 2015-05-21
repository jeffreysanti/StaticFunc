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

	prev = newICGElm(prev, ICG_DEFINE, typeToICGDataType(d), root);
	prev->result = newOp(ICGO_NUMERICREG, getSymbolUniqueName(ident));

	if(root->child2 != NULL){
		prev = icGenAssnToX((PTree*)root->child2, prev, (char*)root->tok->extra, d);
	}
	return prev;
}

void icGenDecl_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "d");
	printICGTypeSuffix(elm, f);
	fprintf(f, " $%s", elm->result->data);
}

