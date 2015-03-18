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
#include "uthash/utarray.h"

typedef enum{
	FS_LISTED,
	FS_CALLED,
	FS_CHECKED,
	FS_CODEGEN,

	FS_NATIVE
} FunctionStatus;

typedef struct{
	PTree *defRoot;
	Type sig;
	bool performReplacement;
	int verid;
	FunctionStatus stat;
	UT_array *sr;
	struct FunctionVersion *next;
	char *funcName;
}FunctionVersion;
extern UT_icd FunctionVersion_icd;

typedef struct{
	char *funcName;
	int nameid;
	FunctionVersion *V;
	UT_hash_handle hh;
}NamedFunctionMapEnt;

typedef struct{
	char *ident;
	Type replace;
}SearchAndReplace;

typedef struct{
	PTree *tree;
	struct PTreeFreeList *next;
}PTreeFreeList;


FunctionVersion *newFunctionVersion();
bool addFunctionVerToList(char *nm, FunctionVersion *ver);

void registerNativeFunction(char *nm, Type sig);

void freeFunctionVersion(FunctionVersion *fv);

void initFunctionSystem();
void freeFunctionSystem();

void seperateFunctionsFromParseTree(PTree **root, bool templatePass);

void addTreeToFreeList(PTree *root);

NamedFunctionMapEnt *getFunctionVersions(char *nm);

void markFunctionVersionUsed(FunctionVersion *fver);
FunctionVersion *markFirstUsedVersionChecked();


#endif /* STATICFUNC_SRC_FUNCTIONS_H_ */
