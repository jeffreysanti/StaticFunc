/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  scopeprocessor.h
 *  Preprocessor processor
 *
 */


#ifndef STATICFUNC_SRC_ICGOPTIMIZE_H_
#define STATICFUNC_SRC_ICGOPTIMIZE_H_

#include "icg.h"
#include "uthash/utarray.h"
#include "uthash/uthash.h"


typedef struct VariableFlowTracker{
	Variable *var;
	int reads;
	int writes;
	char flagLocal;

	ICGElm *lastRead;
	ICGElm *lastWrite;
	ICGElm *lastAccess;
	ICGElm *firstAccess;

	UT_hash_handle hh;
}VariableFlowTracker;

typedef struct MethodAnalysis{
	ICGElm *root;
	VariableFlowTracker *variableflow;
}MethodAnalysis;

typedef struct OptimizationCounter{
	int removeZeroRead;
}OptimizationCounter;


MethodAnalysis *newMethodAnalysis();
void freeMethodAnalysis(MethodAnalysis *ma);

VariableFlowTracker *newVariableFlowTracker(Variable *v, ICGElm *firstAccess);
void freeVariableFlowTracker(VariableFlowTracker *tracker);

OptimizationCounter newOptimizationCounter();
OptimizationCounter sumOptimizationCounter(OptimizationCounter o1, OptimizationCounter o2);

OptimizationCounter performMethodICGOptimization(ICGElm** root);
MethodAnalysis *performMethodAnalysis(ICGElm** root);
ICGElm *localOptimizeBlock(ICGElm *icgroot, MethodAnalysis *ma, OptimizationCounter *stats);
UT_array *performICGOptimization(UT_array *funcs, FILE *outfl);




#endif /* STATICFUNC_SRC_ICGOPTIMIZE_H_ */



