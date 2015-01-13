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
#include "io.h"
#include "parsetree.h"

typedef enum{
	TB_NATIVE_INT8,
	TB_NATIVE_INT16,
	TB_NATIVE_INT32,
	TB_NATIVE_INT64,

	TB_NATIVE_FLOAT32,
	TB_NATIVE_FLOAT64,

	TB_NATIVE_BOOL,
	TB_NATIVE_CHAR,
	TB_NATIVE_STRING
}TypeBase;

typedef struct{
	TypeBase base;
	bool mutable;

	int numchildren;
	struct Type *children;
} Type;


typedef struct{
	char *typeName;
	Type T;
	UT_hash_handle hh;
} TypeStringMapEnt;

void initTypeSystem();
void freeTypeSystem();
void freeType(Type t);

Type newBasicType(TypeBase typ);
void registerType(char *nm, Type t);

Type deduceType(PTree *t);




#endif /* STATICFUNC_SRC_TYPES_H_ */
