/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  types.c
 *  Type Lookup System
 *
 */

#include "types.h"

TypeStringMapEnt *TM = NULL;

void initTypeSystem()
{
	registerType("int8", newBasicType(TB_NATIVE_INT8));
	registerType("int16", newBasicType(TB_NATIVE_INT16));
	registerType("int32", newBasicType(TB_NATIVE_INT32));
	registerType("int64", newBasicType(TB_NATIVE_INT64));

	registerType("float32", newBasicType(TB_NATIVE_FLOAT32));
	registerType("float64", newBasicType(TB_NATIVE_FLOAT64));

	registerType("char", newBasicType(TB_NATIVE_CHAR));
	registerType("bool", newBasicType(TB_NATIVE_BOOL));
	registerType("string", newBasicType(TB_NATIVE_STRING));
}

void freeTypeSystem()
{
	TypeStringMapEnt *ent, *tmp;
	HASH_ITER(hh, TM, ent, tmp) {
		HASH_DEL(TM, ent);
		free(ent->typeName);
		freeType(ent->T);
		free(ent);
	}
}

void freeType(Type t)
{
	if(t.numchildren > 0){
		int i;
		for(i=0; i<t.numchildren; i++){
			freeType(((Type*)t.children)[i]);
		}
		t.numchildren = 0;
		free(t.children);
	}
}

Type newBasicType(TypeBase typ)
{
	Type t;
	t.base = typ;
	t.mutable = false;
	t.numchildren = 0;
	t.children = NULL;
	return t;
}

void registerType(char *nm, Type t)
{
	char *str = malloc(strlen(nm)+1);
	if(str == NULL){
		fatalError("Out of Memory [registerType: str]\n");
	}

	TypeStringMapEnt *ent = malloc(sizeof(TypeStringMapEnt));
	if(ent == NULL){
		fatalError("Out of Memory [registerType: ent]\n");
	}
	strcpy(str, nm);
	ent->typeName = str;
	ent->T = t;
	HASH_ADD_KEYPTR(hh, TM, ent->typeName, strlen(nm), ent);
}


Type deduceType(PTree *t)
{
	printf("TYP:\n");
	dumpParseTree(t, 0);
	printf("///\n");
	return newBasicType(TB_NATIVE_FLOAT32);
}



