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



struct DataFlowTracker{
	Variable *var;

};


ICGElm *localOptimizeBlock(ICGElm *icgroot);
void performICGOptimization(UT_array *funcs, FILE *outfl);




#endif /* STATICFUNC_SRC_ICGOPTIMIZE_H_ */



