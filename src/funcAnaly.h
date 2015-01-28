/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  funcAnaly.h
 *  Function Semantic Analyzer
 *
 */

#ifndef STATICFUNC_SRC_FUNCANALY_H_
#define STATICFUNC_SRC_FUNCANALY_H_

#include "parsetree.h"
#include "types.h"
#include "symbols.h"
#include "functions.h"

#define integralType(x) x.base == TB_NATIVE_INT8 || x.base == TB_NATIVE_INT16 || x.base == TB_NATIVE_INT32 || \
				x.base == TB_NATIVE_INT64

#define floatingType(x) x.base == TB_NATIVE_FLOAT32 || x.base == TB_NATIVE_FLOAT64


bool semAnalyFunc(PTree *root, bool global, Type sig);


#endif /* STATICFUNC_SRC_FUNCANALY_H_ */
