/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  types.h
 *  Type Lookup System
 *
 */

#ifndef STATICFUNC_SRC_TYPES_H_
#define STATICFUNC_SRC_TYPES_H_

#include "uthash/uthash.h"
#include "uthash/utarray.h"
#include "io.h"

typedef enum{
	TB_NATIVE_INT8,
	TB_NATIVE_INT16,
	TB_NATIVE_INT32,
	TB_NATIVE_INT64,
	//TB_ANY_INT,

	TB_NATIVE_FLOAT32,
	TB_NATIVE_FLOAT64,
	//TB_ANY_FLOAT,

	TB_NATIVE_BOOL,
	TB_NATIVE_CHAR,
	TB_NATIVE_STRING,

	TB_NATIVE_VOID,

	TB_VECTOR,
	TB_DICT,
	TB_TUPLE,
	TB_SET,
	TB_FUNCTION,

	TB_TYPELIST,
	TB_ERROR
}TypeBase;

typedef enum{
	MS_RHS,
	MS_LHS,
	MS_NEITHER
}MutableSide;

typedef struct{
	TypeBase base;
	bool mutable;
	bool hasTypeListParam;

	int numchildren;
	struct Type *children;

	char *altName; // in case of tuple
	char *typelistName;
} Type;


typedef struct{
	char *typeName;
	Type T;
	UT_hash_handle hh;
} TypeStringMapEnt;

typedef struct{
	Type T;
	struct TypeList *next;
} TypeList;

typedef struct{
	char *typeListName;
	TypeList TL;
	UT_hash_handle hh;
} TypeListStringMapEnt;


typedef struct{
	UT_array *types;
	UT_array *extra;
	void *extraPtr1;
	void *extraPtr2;
	void *extraPtr3;
	void (*onChooseDeduction)(void *);
}TypeDeductions;


struct _PTree;

void initTypeSystem();
void freeTypeSystem();
void freeType(Type t);
void freeTypeList(TypeList t);

Type newBasicType(TypeBase typ);
Type newBasicType_m(TypeBase typ, bool mut);
Type newVectorType(Type typ);
bool typesEqual(Type t1, Type t2);

bool typesEqualMostly(Type t1, Type t2);
bool typesMatchAllowDownConvert(Type expected, Type found);

bool isTypeRegistered(char *nm);
void registerType(char *nm, Type t);

void allocTypeChildren(Type *in, int n);

Type duplicateType(Type typ);
Type substituteTypeTemplate(Type typ, Type temp, char *search);


Type deduceTypeDeclType(struct _PTree *t);
Type deduceTypeExpr(struct _PTree *t);

TypeList getTypeListByName(char *nm);

void addToTypeList(char *list, Type t);
bool isTypeList(char *list);

char *getDeclTypeListName(struct _PTree *t);

Type getProgramReturnType();


char *getTypeAsString(Type t);
struct _PTree *getTypeAsPTree(Type t);

Type getMostGeneralType(Type t1, Type t2);

Type getLogicalIntegerTypeByLiteral(char *lit);
Type getLogicalFloatTypeByLiteral(char *lit);



// Type Deductions
TypeDeductions newTypeDeductions();
void freeTypeDeductions(TypeDeductions ret);

void markTypeDeductionsMutable(TypeDeductions d);

TypeDeductions expandedTypeDeduction(Type type);
TypeDeductions singleTypeDeduction(Type type);
TypeDeductions mergeTypeDeductions(TypeDeductions expected, TypeDeductions found, MutableSide ms);
TypeDeductions mergeTypeDeductionsOrErr(TypeDeductions expected, TypeDeductions found, int *err, MutableSide ms);
bool typeDeductionMergeExists(TypeDeductions expected, TypeDeductions found);
TypeDeductions duplicateTypeDeductions(TypeDeductions d);
void showTypeDeductionOption(TypeDeductions op);

#endif /* STATICFUNC_SRC_TYPES_H_ */
