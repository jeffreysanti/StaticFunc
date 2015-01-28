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

typedef enum{
	FS_LISTED,
	FS_CALLED,
	FS_CHECKED,
	FS_CODEGEN
} FunctionStatus;

typedef struct{
	PTree *defRoot;
	Type sig;
	bool performReplacement;
	int verid;
	FunctionStatus stat;
	struct FunctionVersion *next;
}FunctionVersion;

typedef struct{
	char *funcName;
	int nameid;
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

NamedFunctionMapEnt *getFunctionVersions(char *nm);


#endif /* STATICFUNC_SRC_FUNCTIONS_H_ */
