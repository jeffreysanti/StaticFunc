/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  scopeprocessor.h
 *  Preprocessor processor
 *
 */


#ifndef STATICFUNC_SRC_SCOPEPROCESSOR_H_
#define STATICFUNC_SRC_SCOPEPROCESSOR_H_

#include "symbols.h"
#include "uthash/utarray.h"
#include "hashset/hashset.h"
#include "hashset/hashset_itr.h"

typedef struct PackedLambdaData{
	UT_array *vars;
	UT_array *vars_offsets;

	UT_array *spaces;
	UT_array *spaces_offsets;
	//UT_array *spaces_hops; // number of hops methodscope hops up to this space

	int size;
}PackedLambdaData;

typedef struct PackedLambdaOffset{
	int offsetDirect; // main offset from lambda pointer
	int offsetIndirect; // offset (if non-zero) from address at offsetDirect
} PackedLambdaOffset;

typedef struct PackedGlobalVariables{
	UT_array *vars;
	UT_array *vars_offsets;

	int size;
} PackedGlobalVariables;


void initScopeProcessor();
void freeScopeProcessor();

PackedLambdaData *newPackedLambda();
void freePackedLambda();

void processSystemGlobal(Variable *v);

int getStaticVarSize();

PackedLambdaData *processMethodScope(Scope *scope);

PackedLambdaOffset findVariableLambdaOffset(Variable *v, Scope *from);
int findGlobalVariableOffset(Variable *v);



#endif /* STATICFUNC_SRC_SCOPEPROCESSOR_H_ */



