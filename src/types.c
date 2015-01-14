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
TypeListStringMapEnt *TLM = NULL;

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

void freeTypeList(TypeList t)
{
	if(t.next != NULL){
		freeTypeList(*((TypeList*)t.next));
		free(t.next);
	}
	freeType(t.T);
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

	TypeListStringMapEnt *entlist, *tmplist;
	HASH_ITER(hh, TLM, entlist, tmplist) {
		HASH_DEL(TLM, entlist);
		free(entlist->typeListName);
		freeTypeList(entlist->TL);
		free(entlist);
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
	if(t.altName != NULL){
		free(t.altName);
	}
}

void allocTypeChildren(Type *in, int n)
{
	in->numchildren = n;
	in->children = malloc(in->numchildren * sizeof(Type));
	if(in->children == NULL){
		fatalError("Out of Memory [allocTypeChildren]\n");
	}
}

Type newBasicType(TypeBase typ)
{
	Type t;
	t.base = typ;
	t.mutable = false;
	t.numchildren = 0;
	t.children = NULL;
	t.altName = NULL;
	return t;
}

bool isTypeRegistered(char *nm)
{
	TypeStringMapEnt *entTmp;
	HASH_FIND_STR(TM, nm, entTmp);
	if(entTmp){
		return true;
	}
	return false;
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
	if(isTypeRegistered(str)){
		fatalError("Type Redefinition - Should Have Been Already Caught\n");
	}
	HASH_ADD_KEYPTR(hh, TM, ent->typeName, strlen(nm), ent);
}

Type duplicateType(Type typ)
{
	Type ret;
	ret.base = typ.base;
	ret.mutable = typ.mutable;
	ret.numchildren = typ.numchildren;
	ret.altName = NULL;
	if(ret.numchildren > 0){
		ret.children = malloc(ret.numchildren * sizeof(Type));
		if(ret.children == NULL){
			fatalError("Out of Memory [duplicateType : children]\n");
		}
		int i;
		for(i=0; i<ret.numchildren; i++){
			Type *ptr = (Type*)ret.children + i;
			*ptr = duplicateType(((Type*)typ.children)[i]);
		}
	}
	if(typ.altName != NULL){
		ret.altName = malloc(strlen(typ.altName) + 1);
		if(ret.altName == NULL){
			fatalError("Out of Memory [duplicateType : altname]\n");
		}
		strcpy(ret.altName, typ.altName);
	}
	return ret;
}

inline void paramFailed(PTree* t){
	reportError("TS004", "Parameter Type Failed: Line %ld", t->tok->lineNo);
}
inline void paramVoid(PTree* t){
	reportError("TS006", "Parameter Type Cannot Be Void: Line %ld", t->tok->lineNo);
}

Type deduceType(PTree *t)
{
	TypeStringMapEnt *ent;

	bool mutable = false;
	if(t->child1 != NULL && ((PTree*)t->child1)->typ == PTT_DECL_MOD){
		if(strcmp(((PTree*)t->child1)->tok->extra, "mut") == 0){
			mutable = true;
		}
	}

	char *ident = (char*)t->tok->extra;
	if(strcmp(ident, "void") == 0){
		return newBasicType(TB_NATIVE_VOID);
	}else if(strcmp(ident, "function") == 0){
		int numElms = 0;
		PTree *treePtr = t;
		while(treePtr->child2 != NULL){
			numElms ++;
			treePtr = (PTree*)treePtr->child2;
		}
		if(numElms < 2){
			reportError("TS009", "Function Needs At Least Two Parameters (can be void): Line %ld", t->tok->lineNo);
			return newBasicType(TB_ERROR);
		}
		Type ret = newBasicType(TB_FUNCTION);
		allocTypeChildren(&ret, numElms);
		treePtr = t;
		int i;
		for(i=0; i<numElms; i++){
			treePtr = (PTree*)treePtr->child2;
			((Type*)ret.children)[i] = deduceType((PTree*)treePtr->child1);
			if(((Type*)ret.children)[i].base == TB_ERROR){
				paramFailed(t);
				freeType(ret);
				return newBasicType(TB_ERROR);
			}
			if(((Type*)ret.children)[i].base == TB_NATIVE_VOID && i != 0 && !(i == 1 && numElms == 2)){
				reportError("TS010", "Parameter Type Cannot Be Void: Line %ld\n"
							"\tOnly first param can be void and second if only two total.", t->tok->lineNo);
				freeType(ret);
				return newBasicType(TB_ERROR);
			}
		}
		return ret;
	}else if(strcmp(ident, "tuple") == 0){
		int numElms = 0;
		PTree *treePtr = t;
		while(treePtr->child2 != NULL){
			numElms ++;
			treePtr = (PTree*)treePtr->child2;
		}
		if(numElms < 1){
			reportError("TS008", "Tuple Needs At Least One Parameter: Line %ld", t->tok->lineNo);
			return newBasicType(TB_ERROR);
		}
		Type ret = newBasicType(TB_TUPLE);
		allocTypeChildren(&ret, numElms);
		treePtr = t;
		int i;
		for(i=0; i<numElms; i++){
			treePtr = (PTree*)treePtr->child2;
			((Type*)ret.children)[i] = deduceType((PTree*)treePtr->child1);
			if(((Type*)ret.children)[i].base == TB_ERROR){
				paramFailed(t);
				freeType(ret);
				return newBasicType(TB_ERROR);
			}
			if(((Type*)ret.children)[i].base == TB_NATIVE_VOID){
				paramVoid(t);
				freeType(ret);
				return newBasicType(TB_ERROR);
			}
			if(treePtr->tok != NULL){ // named param
				ret.altName = malloc(strlen((char*)treePtr->tok->extra) + 1);
				if(ret.altName == NULL){
					fatalError("Out of Memory [deduceType : tuple altname]\n");
				}
				strcpy(ret.altName, (char*)treePtr->tok->extra);
			}
		}
		return ret;
	}else if(strcmp(ident, "dict") == 0){
		Type ret = newBasicType(TB_DICT);
		allocTypeChildren(&ret, 2);
		if(t->child2 == NULL || ((PTree*)t->child2)->child1 == NULL || ((PTree*)t->child2)->child2 == NULL ||
				((PTree*)((PTree*)t->child2)->child2)->child1 == NULL ||
				((PTree*)((PTree*)t->child2)->child2)->child2 != NULL){
			reportError("TS007", "Dictionary Needs Exactly Two Parameters: Line %ld", t->tok->lineNo);
			freeType(ret);
			return newBasicType(TB_ERROR);
		}
		((Type*)ret.children)[0] = deduceType((PTree*)((PTree*)t->child2)->child1);
		((Type*)ret.children)[1] = deduceType((PTree*)((PTree*)((PTree*)t->child2)->child2)->child1);
		if(((Type*)ret.children)[0].base == TB_ERROR || ((Type*)ret.children)[1].base == TB_ERROR){
			paramFailed(t);
			freeType(ret);
			return newBasicType(TB_ERROR);
		}
		if(((Type*)ret.children)[0].base == TB_NATIVE_VOID || ((Type*)ret.children)[1].base == TB_NATIVE_VOID){
			paramVoid(t);
			freeType(ret);
			return newBasicType(TB_ERROR);
		}
		return ret;
	}else if(strcmp(ident, "vector") == 0 || strcmp(ident, "set") == 0){
		Type ret = newBasicType(TB_VECTOR);
		if(strcmp(ident, "set") == 0)
			ret = newBasicType(TB_SET);
		allocTypeChildren(&ret, 1);
		if(t->child2 == NULL || ((PTree*)t->child2)->child1 == NULL || ((PTree*)t->child2)->child2 != NULL){
			reportError("TS003", "Vector/Set Needs Exactly One Parameter: Line %ld", t->tok->lineNo);
			freeType(ret);
			return newBasicType(TB_ERROR);
		}
		((Type*)ret.children)[0] = deduceType((PTree*)((PTree*)t->child2)->child1);
		if((*((Type*)ret.children)).base == TB_ERROR){
			paramFailed(t);
			freeType(ret);
			return newBasicType(TB_ERROR);
		}
		if((*((Type*)ret.children)).base == TB_NATIVE_VOID){
			paramVoid(t);
			freeType(ret);
			return newBasicType(TB_ERROR);
		}
		return ret;
	}
	// Otherwise, a basic or already defined type
	if(t->child2 != NULL){
		reportError("TS001", "Type Cannot Take Parameters (Already Defined, Or Basic Type): Line %ld", t->tok->lineNo);
		return newBasicType(TB_ERROR);
	}
	HASH_FIND_STR(TM, (char*)t->tok->extra, ent);
	if(ent){
		Type ret = duplicateType(ent->T);
		if(mutable) ret.mutable = true;
		return ret;
	}
	reportError("TS002", "Unrecognized Type: Line %ld", t->tok->lineNo);
	return newBasicType(TB_ERROR);
}

bool typesEqual(Type t1, Type t2)
{
	if(t1.base != t2.base) return false;
	if(t1.mutable != t2.mutable) return false;
	if(t1.numchildren != t2.numchildren) return false;
	if((t1.altName == NULL && t2.altName != NULL) || (t1.altName != NULL && t2.altName == NULL)) return false;
	if(t1.altName != NULL && strcmp(t1.altName, t2.altName) != 0) return false;
	if(t1.numchildren > 0){
		int i;
		for(i=0; i<t1.numchildren; i++){
			if(!typesEqual(((Type*)t1.children)[i], ((Type*)t2.children)[i])) return false;
		}
	}
	return true;
}

void addToTypeList(char *list, Type t)
{
	TypeListStringMapEnt *entTmp;
	HASH_FIND_STR(TLM, list, entTmp);
	if(entTmp){ // already exists -> add to linked list
		TypeList *ptr = &(entTmp->TL);
		while(ptr->next != NULL){
			if(typesEqual(t, ptr->T)){ // already exists in list
				freeType(t);
				return;
			}
			ptr = (TypeList*)ptr->next;
		}
		ptr->next = malloc(sizeof(TypeList));
		if(ptr->next == NULL){
			fatalError("Out of Memory [addToTypeList: ptr->next]\n");
		}
		ptr = (TypeList*)ptr->next;
		ptr->T = t;
		ptr->next = NULL;
	}else{ // create new map entry
		char *str = malloc(strlen(list)+1);
		if(str == NULL){
			fatalError("Out of Memory [addToTypeList: str]\n");
		}
		TypeListStringMapEnt *ent = malloc(sizeof(TypeListStringMapEnt));
		if(ent == NULL){
			fatalError("Out of Memory [addToTypeList: ent]\n");
		}
		strcpy(str, list);
		ent->typeListName = str;
		ent->TL.T = t;
		ent->TL.next = NULL;
		HASH_ADD_KEYPTR(hh, TLM, ent->typeListName, strlen(str), ent);
	}
}



