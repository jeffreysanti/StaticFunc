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

#define integralType(x) (x.base == TB_NATIVE_INT8 || x.base == TB_NATIVE_INT16 || x.base == TB_NATIVE_INT32 || \
				x.base == TB_NATIVE_INT64)

#define floatingType(x) (x.base == TB_NATIVE_FLOAT32 || x.base == TB_NATIVE_FLOAT64)


typedef struct{
	UT_array *types;
}TypeDeductions;
static UT_icd TypeDeductions_icd = {sizeof(Type), NULL, NULL, NULL};


static TypeDeductions newTypeDeductions(){
	TypeDeductions ret;
	utarray_new(ret.types, &TypeDeductions_icd);
	return ret;
}

static void freeTypeDeductions(TypeDeductions ret){
	utarray_free(ret.types);
}

static TypeDeductions singleType(Type t){
	TypeDeductions ret = newTypeDeductions();
	utarray_push_back(ret.types, &t);
	return ret;
}

static void showTypeVsErr(TypeDeductions expected, Type recvd){
	Type *p = NULL;
	while((p=(Type*)utarray_next(expected.types,p))){
		errShowType("EXPECTED [OR LESS]: ",p);
	}
	errShowType("FOUND             : ",&recvd);
}
static void showTypeDeductionOption(TypeDeductions op){
	Type *p = NULL;
	while((p=(Type*)utarray_next(op.types,p))){
		errShowType("  FOUND: ",p);
	}
}
static void showTypeVsErr_multrecv(Type expected, TypeDeductions recvd){
	Type *p = NULL;
	errShowType("EXPECTED [OR LESS]: ",&expected);
	while((p=(Type*)utarray_next(recvd.types,p))){
		errShowType("FOUND             : ",p);
	}

}
static void showTypeVsErr_multboth(TypeDeductions expected, TypeDeductions recvd){
	Type *p = NULL;
	while((p=(Type*)utarray_next(expected.types,p))){
		errShowType("EXPECTED [OR LESS]: ",p);
	}
	while((p=(Type*)utarray_next(recvd.types,p))){
		errShowType("FOUND             : ",p);
	}
}

static Type findDeductionMatching(TypeDeductions expected, Type found){
	Type ret = newBasicType(TB_ERROR);
	Type *p = NULL;
	while((p=(Type*)utarray_next(expected.types,p))){
		if(typesMatchAllowDownConvert(*p, found)){
			ret = *p;
			freeTypeDeductions(expected);
			return ret;
		}
	}
	reportError("SA012", "No Type Deduction Matched");
	showTypeVsErr(expected, found);
	freeTypeDeductions(expected);
	return ret;
}

static Type findDeductionMatching_multboth(TypeDeductions expected, TypeDeductions found){
	Type ret = newBasicType(TB_ERROR);
	Type *pf = NULL;
	Type *pe = NULL;
	while((pf=(Type*)utarray_next(found.types,pf))){
		while((pe=(Type*)utarray_next(expected.types,pe))){
			if(typesMatchAllowDownConvert(*pe, *pf)){
				ret = *pf;
				freeTypeDeductions(expected);
				freeTypeDeductions(found);
				return ret;
			}
		}
	}
	reportError("SA012", "No Type Deduction Matched");
	showTypeVsErr_multboth(expected, found);
	freeTypeDeductions(expected);
	freeTypeDeductions(found);
	return ret;
}

static Type findDeductionMatching_multrecv(Type expected, TypeDeductions found){
	Type ret = newBasicType(TB_ERROR);
	Type *p = NULL;
	while((p=(Type*)utarray_next(found.types,p))){
		if(typesMatchAllowDownConvert(expected, *p)){
			ret = *p;
			freeTypeDeductions(found);
			return ret;
		}
	}
	reportError("SA012", "No Type Deduction Matched");
	showTypeVsErr_multrecv(expected, found);
	freeTypeDeductions(found);
	return ret;
}

static Type findDeductionMatching_multrecv_silent(Type expected, TypeDeductions found){
	Type ret = newBasicType(TB_ERROR);
	Type *p = NULL;
	while((p=(Type*)utarray_next(found.types,p))){
		if(typesMatchAllowDownConvert(expected, *p)){
			ret = *p;
			freeTypeDeductions(found);
			return ret;
		}
	}
	return ret;
}


bool semAnalyFunc(PTree *root, bool global, Type sig);

bool semAnalyExpr(PTree *root, Type expect, bool silent);
TypeDeductions handleFunctCall(PTree *root, int *err);

TypeDeductions deduceTreeType(PTree *root, int *err);



#endif /* STATICFUNC_SRC_FUNCANALY_H_ */
