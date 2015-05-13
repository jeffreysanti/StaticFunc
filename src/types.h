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
	CAST_UP,
	CAST_DOWN
}CastDirection;

typedef struct{
	TypeBase base;
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
Type newVectorType(Type typ);
Type newDictionaryType(Type keytype, Type valtype);
bool typesEqual(Type t1, Type t2);

bool typesEqualMostly(Type t1, Type t2);

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

TypeDeductions expandedTypeDeduction(Type type, CastDirection cd);
TypeDeductions singleTypeDeduction(Type type);
TypeDeductions mergeTypeDeductions(TypeDeductions expected, TypeDeductions found);
TypeDeductions mergeTypeDeductionsOrErr(TypeDeductions expected, TypeDeductions found, int *err);
void reportTypeDeductions(char *prefix, TypeDeductions td);
bool typeDeductionMergeExists(TypeDeductions expected, TypeDeductions found);
void showTypeDeductionMergeError(TypeDeductions expected, TypeDeductions found);
TypeDeductions duplicateTypeDeductions(TypeDeductions d);
void showTypeDeductionOption(TypeDeductions op);

void addVectorsOfTypeDeduction(TypeDeductions *dest, TypeDeductions in);
void addDictsOfTypeDeduction(TypeDeductions *dest, TypeDeductions keys, TypeDeductions values);
void addAllTuplesOfTypeDeductions(TypeDeductions *dest, TypeDeductions *array, int cnt);

void singlesOfVectorsTypeDeduction(TypeDeductions *dest, TypeDeductions in);

void appendToTypeDeductionAndFree(TypeDeductions *dest, TypeDeductions in);



bool isTypeNumeric(Type t);

#endif /* STATICFUNC_SRC_TYPES_H_ */
