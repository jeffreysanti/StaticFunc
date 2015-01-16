/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  functions.h
 *  Function Storage Container
 *
 */

#ifndef STATICFUNC_SRC_FUNCTIONS_H_
#define STATICFUNC_SRC_FUNCTIONS_H_

#include "parsetree.h"
#include "types.h"

typedef struct{
	PTree *defRoot;
	Type sig;
	bool performReplacement;
	bool used;
	struct FunctionVersion *next;
}FunctionVersion;

typedef struct{
	char *funcName;
	FunctionVersion *V;
	UT_hash_handle hh;
}NamedFunctionMapEnt;

typedef struct{
	PTree *tree;
	struct PTreeFreeList *next;
}PTreeFreeList;


FunctionVersion *newFunctionVersion(FunctionVersion *parent);
FunctionVersion *newFunctionVersionByName(char *nm);

void freeFunctionVersion(FunctionVersion *fv);

void initFunctionSystem();
void freeFunctionSystem();

void seperateFunctionsFromParseTree(PTree *root);

void addTreeToFreeList(PTree *root);

#endif /* STATICFUNC_SRC_FUNCTIONS_H_ */
