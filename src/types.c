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

Type TprogRet;

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

	TprogRet = newBasicType(TB_FUNCTION);
	allocTypeChildren(&TprogRet, 1);
	((Type*)TprogRet.children)[0] = newBasicType(TB_NATIVE_INT32);
}

Type getProgramReturnType()
{
	return TprogRet;
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

	freeType(TprogRet);
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
	t.hasTypeListParam = false;
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
	ret.hasTypeListParam = typ.hasTypeListParam;
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

Type substituteTypeTemplate(Type typ, Type temp)
{
	if(typ.base == TB_TYPELIST)
		return temp;
	Type ret = typ;
	if(ret.numchildren > 0){
		int i;
		for(i=0; i<ret.numchildren; i++){
			Type *ptr = (Type*)ret.children + i;
			*ptr = substituteTypeTemplate(((Type*)typ.children)[i], temp);
		}
	}
	return ret;
}

inline void paramFailed(PTree* t){
	reportError("TS004", "Parameter Type Failed: Line %ld", t->tok->lineNo);
}
inline void paramVoid(PTree* t){
	reportError("TS006", "Parameter Type Cannot Be Void: Line %ld", t->tok->lineNo);
}
inline void paramMultTypLists(PTree* t){
	reportError("TS012", "Type cannot have multiple different type list params: Line %ld", t->tok->lineNo);
}

Type deduceTypeDeclType(PTree *t)
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
			((Type*)ret.children)[i] = deduceTypeDeclType((PTree*)treePtr->child1);
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
			if(((Type*)ret.children)[i].base == TB_TYPELIST || ((Type*)ret.children)[i].hasTypeListParam){
				ret.hasTypeListParam = true;
			}
		}
		return ret;
	}else if(strcmp(ident, "tuple") == 0){
		int numElms = 0;
		PTree *treePtr = t;
		char *typelistTempName = NULL;
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
			((Type*)ret.children)[i] = deduceTypeDeclType((PTree*)treePtr->child1);
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
			if(((Type*)ret.children)[i].base == TB_TYPELIST || ((Type*)ret.children)[i].hasTypeListParam){
				if(typelistTempName != NULL &&
						strcmp(typelistTempName, getDeclTypeListName((PTree*)treePtr->child1)) != 0){
					paramMultTypLists(t);
					freeType(ret);
					return newBasicType(TB_ERROR);
				}
				ret.hasTypeListParam = true;
				typelistTempName = getDeclTypeListName((PTree*)treePtr->child1);
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
		((Type*)ret.children)[0] = deduceTypeDeclType((PTree*)((PTree*)t->child2)->child1);
		((Type*)ret.children)[1] = deduceTypeDeclType((PTree*)((PTree*)((PTree*)t->child2)->child2)->child1);
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
		if(((Type*)ret.children)[0].base == TB_TYPELIST || ((Type*)ret.children)[0].hasTypeListParam ||
				((Type*)ret.children)[1].base == TB_TYPELIST || ((Type*)ret.children)[1].hasTypeListParam){
			// make sure they aren't different
			if(((Type*)ret.children)[0].hasTypeListParam && ((Type*)ret.children)[1].hasTypeListParam){
				if(strcmp(getDeclTypeListName((PTree*)((PTree*)t->child2)->child1),
						getDeclTypeListName((PTree*)((PTree*)((PTree*)t->child2)->child2)->child1)) != 0){
					paramMultTypLists(t);
					freeType(ret);
					return newBasicType(TB_ERROR);
				}
			}
			ret.hasTypeListParam = true;
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
		((Type*)ret.children)[0] = deduceTypeDeclType((PTree*)((PTree*)t->child2)->child1);
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
		if((*((Type*)ret.children)).base == TB_TYPELIST || (*((Type*)ret.children)).hasTypeListParam){
			ret.hasTypeListParam = true;
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
	// attempt to find if this is a type list
	TypeListStringMapEnt *enttl;
	HASH_FIND_STR(TLM, (char*)t->tok->extra, enttl);
	if(enttl){ // already exists -> add to linked list
		Type ret = newBasicType(TB_TYPELIST);
		ret.hasTypeListParam = true;
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

TypeList getTypeListByName(char *nm)
{
	TypeListStringMapEnt *enttl;
	HASH_FIND_STR(TLM, nm, enttl);
	if(enttl){ // already exists -> add to linked list
		return enttl->TL;
	}
	fatalError("getTypeListByName: nm Does Not Exist: Logic Error\n");
	TypeList def;
	return def;
}

char *getDeclTypeListName(PTree *t)
{
	// try base case
	Type base = deduceTypeDeclType(t);
	if(base.base == TB_TYPELIST){
		freeType(base);
		return (char*)t->tok->extra;
	}
	PTree *treePtr = t;
	int i;
	for(i=0; i<base.numchildren; i++){
		treePtr = (PTree*)treePtr->child2;
		char *tmp = getDeclTypeListName((PTree*)treePtr->child1);
		if(tmp != NULL){
			freeType(base);
			return tmp;
		}
	}
	freeType(base);
	return NULL;
}


/*TB_NATIVE_INT8,
	TB_NATIVE_INT16,
	TB_NATIVE_INT32,
	TB_NATIVE_INT64,

	TB_NATIVE_FLOAT32,
	TB_NATIVE_FLOAT64,

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
	TB_ERROR*/

char *getTypeAsString(Type t)
{
	int len = 0;
	if(t.mutable)
		len += 4;
	if(t.base == TB_NATIVE_INT16 || t.base == TB_NATIVE_INT32 || t.base == TB_NATIVE_INT64)
		len += 5;
	if(t.base == TB_NATIVE_FLOAT32 || t.base == TB_NATIVE_FLOAT64)
		len += 7;
	if(t.base == TB_NATIVE_BOOL || t.base == TB_NATIVE_CHAR || t.base == TB_NATIVE_VOID)
		len += 4;
	if(t.base == TB_VECTOR)
		len += 6;
	if(t.base == TB_DICT)
		len += 4;
	if(t.base == TB_TUPLE)
		len += 5;
	if(t.base == TB_SET)
		len += 3;
	if(t.base == TB_FUNCTION)
		len += 8;
	if(len == 0){
		char *ret = malloc(3*sizeof(char));
		strcpy(ret, "???");
		return ret;
	}
	if(t.numchildren > 0){
		len += 2;
		len += t.numchildren - 1; // commas
		int i;
		for(i=0; i<t.numchildren; i++){
			char *tmp = getTypeAsString(((Type*)t.children)[i]);
			len += strlen(tmp);
			free(tmp);
		}
	}
	if(t.altName != NULL){
		len += 1; // colon
		len += strlen(t.altName);
	}
	char *ret = calloc(len+1, sizeof(char));
	char *ptr = ret;
	if(t.mutable) { strcpy(ptr, "mut "); ptr += 4; }
	if(t.base == TB_NATIVE_INT16) { strcpy(ptr, "int16"); ptr += 5; }
	if(t.base == TB_NATIVE_INT32) { strcpy(ptr, "int32"); ptr += 5; }
	if(t.base == TB_NATIVE_INT64) { strcpy(ptr, "int64"); ptr += 5; }
	if(t.base == TB_NATIVE_FLOAT32) { strcpy(ptr, "float32"); ptr += 7; }
	if(t.base == TB_NATIVE_FLOAT64) { strcpy(ptr, "float64"); ptr += 7; }
	if(t.base == TB_NATIVE_BOOL) { strcpy(ptr, "bool"); ptr += 4; }
	if(t.base == TB_NATIVE_CHAR) { strcpy(ptr, "char"); ptr += 4; }
	if(t.base == TB_NATIVE_VOID) { strcpy(ptr, "void"); ptr += 4; }
	if(t.base == TB_VECTOR) { strcpy(ptr, "vector"); ptr += 6; }
	if(t.base == TB_DICT) { strcpy(ptr, "dict"); ptr += 4; }
	if(t.base == TB_TUPLE) { strcpy(ptr, "tuple"); ptr += 5; }
	if(t.base == TB_SET) { strcpy(ptr, "void"); ptr += 3; }
	if(t.base == TB_FUNCTION) { strcpy(ptr, "function"); ptr += 8; }

	if(t.numchildren > 0){
		strcpy(ptr, "<"); ptr += 1;
		int i;
		for(i=0; i<t.numchildren; i++){
			if(i>0){
				strcpy(ptr, ","); ptr += 1;
			}
			char *tmp = getTypeAsString(((Type*)t.children)[i]);
			strcpy(ptr, tmp); ptr += strlen(tmp);
			free(tmp);
		}
		strcpy(ptr, ">"); ptr += 1;
	}
	if(t.altName != NULL){
		strcpy(ptr, ":"); ptr += 1;
		strcpy(ptr, t.altName); ptr += strlen(t.altName);
	}

	return ret;
}


