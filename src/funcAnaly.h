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


/*
#define integralType(x) (x.base == TB_NATIVE_INT8 || x.base == TB_NATIVE_INT16 || x.base == TB_NATIVE_INT32 || \
				x.base == TB_NATIVE_INT64)

#define floatingType(x) (x.base == TB_NATIVE_FLOAT32 || x.base == TB_NATIVE_FLOAT64)





static Type freeTypeDeductionsExcept(TypeDeductions ret, Type leave){
	Type *p = NULL;
	while((p=(Type*)utarray_next(ret.types,p))){
		if(!(typesEqual(leave, (*p))))
			freeType(*p);
	}
	utarray_free(ret.types);
	if(ret.extra != NULL){
		utarray_free(ret.extra);
	}
	return leave;
}

static TypeDeductions singleType(Type t){
	TypeDeductions ret = newTypeDeductions();
	utarray_push_back(ret.types, &t);
	return ret;
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

static Type findDeductionMatching_multboth(TypeDeductions expected, TypeDeductions found){
	Type ret = newBasicType(TB_ERROR);
	Type *pf = NULL;
	Type *pe = NULL;
	int i = 0;
	while((pf=(Type*)utarray_next(found.types,pf))){
		while((pe=(Type*)utarray_next(expected.types,pe))){
			if(typesMatchAllowDownConvert(*pe, *pf)){
				ret = *pf;
				if(found.onChooseDeduction != NULL){
					found.onChooseDeduction(&found, i);
				}
				freeTypeDeductions(expected);
				freeTypeDeductionsExcept(found, ret);
				return ret;
			}
		}
		i ++;
	}
	reportError("SA012", "No Type Deduction Matched");
	showTypeVsErr_multboth(expected, found);
	freeTypeDeductions(expected);
	freeTypeDeductions(found);
	return ret;
}

static Type findDeductionMatching_multrecv_silent_nofree(Type expected, TypeDeductions found){
	Type ret = newBasicType(TB_ERROR);
	Type *p = NULL;
	while((p=(Type*)utarray_next(found.types,p))){
		if(typesMatchAllowDownConvert(expected, *p)){
			ret = *p;
			return ret;
		}
	}
	return ret;
}

static Type findDeductionMatching_multrecv(Type expected, TypeDeductions found){

	Type ret = newBasicType(TB_ERROR);
	Type *p = NULL;
	int i = 0;
	while((p=(Type*)utarray_next(found.types,p))){
		if(typesMatchAllowDownConvert(expected, *p)){
			ret = *p;
			if(found.onChooseDeduction != NULL){
				found.onChooseDeduction(&found, i);
			}
			freeTypeDeductionsExcept(found, ret);
			freeType(expected);
			return ret;
		}
		i++;
	}
	reportError("SA012", "No Type Deduction Matched");
	showTypeVsErr_multrecv(expected, found);
	freeTypeDeductions(found);
	freeType(expected);
	return ret;
}

static Type findDeductionMatching_any(TypeDeductions found, int lineno){
	Type ret = newBasicType(TB_ERROR);
	if(utarray_len(found.types) > 1){
		reportError("#SA020", "Warning: Multiple Type Deductions Found: Line %d", lineno);
		Type *p = NULL;
		while((p=(Type*)utarray_next(found.types,p))){
			errShowType("TYPE: ", p);
		}
	}
	if(utarray_len(found.types) == 0){
		reportError("SA021", "No Type Deductions Exist");
		freeTypeDeductions(found);
		return ret;
	}
	Type *first_ptr = (Type*)utarray_front(found.types);
	Type first = *first_ptr;
	if(found.onChooseDeduction != NULL){
		found.onChooseDeduction(&found, 0);
	}
	freeTypeDeductionsExcept(found, first);
	return first;
}

*/


bool semAnalyFunc(PTree *root, bool global, Type sig);

bool semAnalyExpr(PTree *root, Type expect, bool silent);
TypeDeductions handleFunctCall(PTree *root, int *err);

TypeDeductions deduceTreeType(PTree *root, int *err);
void propagateTreeType(PTree *root);
bool finalizeSingleDeduction(PTree *root);

bool blockUnit(PTree *root, Type sig, bool global);



#endif /* STATICFUNC_SRC_FUNCANALY_H_ */
