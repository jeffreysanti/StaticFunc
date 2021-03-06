/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  types.c
 *  Type Lookup System
 *
 */

#include "types.h"
#include "parsetree.h"

TypeStringMapEnt *TM = NULL;
TypeListStringMapEnt *TLM = NULL;

UT_icd TypeDeductions_icd = {sizeof(Type), NULL, NULL, NULL};

Type TprogRet;

void initTypeSystem()
{
	registerType("int", newBasicType(TB_NATIVE_INT));

	registerType("float", newBasicType(TB_NATIVE_FLOAT));
	
	//registerType("char", newBasicType(TB_NATIVE_CHAR));
	registerType("bool", newBasicType(TB_NATIVE_BOOL));
	registerType("string", newBasicType(TB_NATIVE_STRING));

	TprogRet = newBasicType(TB_FUNCTION);
	allocTypeChildren(&TprogRet, 1);
	((Type*)TprogRet.children)[0] = newBasicType(TB_NATIVE_INT);
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
	if(t.numchildren > 0 && t.children != NULL){
		int i;
		for(i=0; i<t.numchildren; i++){
			freeType(((Type*)t.children)[i]);
		}
		t.numchildren = 0;
		free(t.children);
	}
	if(t.altName != NULL){
		free(t.altName);
		t.altName = NULL;
	}
	if(t.typelistName != NULL){
		free(t.typelistName);
	}
}

void allocTypeChildren(Type *in, int n)
{
	in->numchildren = n;
	in->children = malloc(in->numchildren * sizeof(Type));
	if(in->children == NULL){
		fatalError("Out of Memory [allocTypeChildren]\n");
	}
	int i;
	for(i=0; i<in->numchildren; i++){
		((Type*)in->children)[i] = newBasicType(TB_ERROR);
	}
}

Type newBasicType(TypeBase typ)
{
	Type t;
	t.base = typ;
	t.numchildren = 0;
	t.children = NULL;
	t.altName = NULL;
	t.hasTypeListParam = false;
	t.typelistName = NULL;
	return t;
}

Type newVectorType(Type typ)
{
	Type t = newBasicType(TB_VECTOR);
	allocTypeChildren(&t, 1);
	((Type*)t.children)[0] = typ;
	return t;
}


Type newDictionaryType(Type keytype, Type valtype)
{
	Type t = newBasicType(TB_DICT);
	allocTypeChildren(&t, 2);
	((Type*)t.children)[0] = keytype;
	((Type*)t.children)[1] = valtype;
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
	ret.numchildren = typ.numchildren;
	ret.altName = NULL;
	ret.children = NULL;
	ret.typelistName = NULL;
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
	if(typ.typelistName != NULL){
		ret.typelistName = malloc(strlen(typ.typelistName) + 1);
		if(ret.typelistName == NULL){
			fatalError("Out of Memory [duplicateType : typelistName]\n");
		}
		strcpy(ret.typelistName, typ.typelistName);
	}
	return ret;
}

Type substituteTypeTemplate(Type typ, Type temp, char *search)
{
	if(typ.base == TB_TYPELIST && strcmp(search, typ.typelistName) == 0){
		freeType(typ);
		return temp;
	}
	else if(typ.base == TB_TYPELIST)
		return typ; // do not change -> another substitution
	Type ret = typ;
	if(ret.numchildren > 0){
		int i;
		for(i=0; i<ret.numchildren; i++){
			Type *ptr = (Type*)ret.children + i;
			*ptr = substituteTypeTemplate(((Type*)typ.children)[i], temp, search);
		}
	}
	return ret;
}

static inline void paramFailed(PTree* t){
	reportError("TS004", "Parameter Type Failed: Line %ld", t->tok->lineNo);
}
static inline void paramVoid(PTree* t){
	reportError("TS006", "Parameter Type Cannot Be Void: Line %ld", t->tok->lineNo);
}
static inline void paramMultTypLists(PTree* t){
	reportError("TS012", "Type cannot have multiple different type list params: Line %ld", t->tok->lineNo);
}

Type deduceTypeDeclType(PTree *t)
{
	if(t->typ == PTT_DECL_TYPE_DEDUCED){
		Type *typ = (Type*)utarray_front(t->deducedTypes._types);
		return duplicateType(*typ);
	}

	TypeStringMapEnt *ent;

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
			if(treePtr->tok != NULL){
				if(treePtr->tok->extra != NULL && strlen(treePtr->tok->extra) > 0){ // named param
					if(((Type*)ret.children)[i].altName != NULL){
						free(((Type*)ret.children)[i].altName);
						((Type*)ret.children)[i].altName = NULL;
					}
					((Type*)ret.children)[i].altName = malloc(strlen((char*)treePtr->tok->extra) + 1);
					if(((Type*)ret.children)[i].altName == NULL){
						fatalError("Out of Memory [deduceType : tuple altname]\n");
					}
					strcpy(((Type*)ret.children)[i].altName, (char*)treePtr->tok->extra);
				}
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
	}else if(strcmp(ident, "vector") == 0/* || strcmp(ident, "set") == 0*/){
		Type ret = newBasicType(TB_VECTOR);
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
		return ret;
	}
	// attempt to find if this is a type list
	TypeListStringMapEnt *enttl;
	HASH_FIND_STR(TLM, (char*)t->tok->extra, enttl);
	if(enttl){ // already exists -> add to linked list
		Type ret = newBasicType(TB_TYPELIST);
		ret.typelistName = malloc(1+strlen((char*)t->tok->extra));
		strcpy(ret.typelistName, (char*)t->tok->extra);
		ret.hasTypeListParam = true;
		return ret;
	}

	reportError("TS002", "Unrecognized Type: Line %ld", t->tok->lineNo);
	return newBasicType(TB_ERROR);
}
/*
Type deduceTypeExpr(PTree *t)
{
	if(t == NULL){
		reportError("TS020", "Type Is Undeducable");
		return newBasicType(TB_ERROR);
	}
	if(t->typ == PTT_INT){
		return newBasicType(TB_ANY_INT);
	}if(t->typ == PTT_FLOAT){
		return newBasicType(TB_ANY_FLOAT);
	}
}
*/
bool typesEqual(Type t1, Type t2)
{
	if(t1.base != t2.base) return false;
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

// does not care if metadata(typename/flags) match
bool typesEqualMostly(Type t1, Type t2)
{
	if(t1.base != t2.base) return false;
	if(t1.numchildren != t2.numchildren) return false;
	if(t1.numchildren > 0){
		int i;
		for(i=0; i<t1.numchildren; i++){
			if(!typesEqualMostly(((Type*)t1.children)[i], ((Type*)t2.children)[i])) return false;
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


char *getTypeAsString(Type t)
{
	int len = 0;
	if(t.base == TB_NATIVE_INT)
		len += 3;
	if(t.base == TB_NATIVE_FLOAT)
		len += 5;
	if(t.base == TB_NATIVE_BOOL /*|| t.base == TB_NATIVE_CHAR*/ || t.base == TB_NATIVE_VOID)
		len += 4;
	if(t.base == TB_VECTOR || t.base == TB_NATIVE_STRING)
		len += 6;
	if(t.base == TB_DICT)
		len += 4;
	if(t.base == TB_TUPLE)
		len += 5;
	if(t.base == TB_FUNCTION)
		len += 8;
	if(len == 0){
		char *ret = malloc(4*sizeof(char));
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
	if(t.base == TB_NATIVE_INT) { strcpy(ptr, "int"); ptr += 3; }
	if(t.base == TB_NATIVE_FLOAT) { strcpy(ptr, "float"); ptr += 5; }
	if(t.base == TB_NATIVE_BOOL) { strcpy(ptr, "bool"); ptr += 4; }
	//if(t.base == TB_NATIVE_CHAR) { strcpy(ptr, "char"); ptr += 4; }
	if(t.base == TB_NATIVE_VOID) { strcpy(ptr, "void"); ptr += 4; }
	if(t.base == TB_NATIVE_STRING) { strcpy(ptr, "string"); ptr += 6; }
	if(t.base == TB_VECTOR) { strcpy(ptr, "vector"); ptr += 6; }
	if(t.base == TB_DICT) { strcpy(ptr, "dict"); ptr += 4; }
	if(t.base == TB_TUPLE) { strcpy(ptr, "tuple"); ptr += 5; }
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



Type getLogicalIntegerTypeByLiteral(char *lit)
{
	return newBasicType(TB_NATIVE_INT); // TODO
}

Type getLogicalFloatTypeByLiteral(char *lit)
{
	return newBasicType(TB_NATIVE_FLOAT); // TODO
}


PTree *getTypeAsPTree(Type t)
{
	PTree *tree = newParseTree(PTT_DECL_TYPE_DEDUCED);
	freeTypeDeductions(tree->deducedTypes);
	tree->deducedTypes = expandedTypeDeduction(t, CAST_UP); // TODO: Is CAST_UP Correct?
	return tree;
}




TypeDeductions newTypeDeductions(){
	TypeDeductions ret;
	utarray_new(ret._types, &TypeDeductions_icd);
	ret.extra = NULL;
	ret.onChooseDeduction = NULL;
	ret.extraPtr1 = NULL;
	ret.extraPtr2 = NULL;
	ret.extraPtr3 = NULL;
	return ret;
}

void freeTypeDeductions(TypeDeductions ret){
	Type *p = NULL;
	int i=0;
	while((p=(Type*)utarray_next(ret._types,p))){
		//printf("Freeing type # %d\n", i);
		freeType(*p);
		//errShowType("NOT REALL:",p);
		i++;
	}
	utarray_free(ret._types);
	if(ret.extra != NULL){
		utarray_free(ret.extra);
	}
}

TypeDeductions singleTypeDeduction(Type t){
	t = duplicateType(t);
	TypeDeductions ret = newTypeDeductions();
	addTypeDeductionsType(&ret, t);
	return ret;
}

TypeDeductions expandedTypeDeduction(Type type, CastDirection cd)
{
	TypeDeductions ret = newTypeDeductions();
	if(type.base == TB_NATIVE_INT){
	  addTypeDeductionsType(&ret, newBasicType(TB_NATIVE_INT));
	}else if(type.base == TB_NATIVE_FLOAT){
	    addTypeDeductionsType(&ret, newBasicType(TB_NATIVE_FLOAT));
	}else if(type.base == TB_NATIVE_STRING){
		addTypeDeductionsType(&ret, newBasicType(TB_NATIVE_STRING));
	}else if(type.base == TB_VECTOR){
		TypeDeductions innerDeductions = expandedTypeDeduction(((Type*)type.children)[0], cd);
		Type *p = NULL;
		while((p=(Type*)utarray_next(innerDeductions._types,p))){
			addTypeDeductionsType(&ret, newVectorType(duplicateType(*p)));
		}
		freeTypeDeductions(innerDeductions);
	}else if(type.base == TB_FUNCTION){
		addTypeDeductionsType(&ret, duplicateType(type));
	}else if(type.base == TB_DICT){
		TypeDeductions innerDeductionsKey = expandedTypeDeduction(((Type*)type.children)[0], cd);
		TypeDeductions innerDeductionsVal = expandedTypeDeduction(((Type*)type.children)[1], cd);
		Type *p = NULL;
		Type *q = NULL;
		while((p=(Type*)utarray_next(innerDeductionsKey._types,p))){
			while((q=(Type*)utarray_next(innerDeductionsVal._types,q))){
				Type tmp =  newDictionaryType(duplicateType(*p), duplicateType(*q));
				addTypeDeductionsType(&ret, tmp);
			}
		}
		freeTypeDeductions(innerDeductionsKey);
		freeTypeDeductions(innerDeductionsVal);
	}else if(type.base == TB_TUPLE){
		TypeDeductions *arr = malloc(sizeof(TypeDeductions) * type.numchildren);
		int i;
		for(i=0; i<type.numchildren; i++){
			arr[i] = expandedTypeDeduction(((Type*)type.children)[i], cd);
		}
		addAllTuplesOfTypeDeductions(&ret, arr, type.numchildren);
		for(i=0; i<type.numchildren; i++){
			freeTypeDeductions(arr[i]);
		}
		free(arr);
	}else{
		fatalError("Unknown Type Base in expandedTypeDeduction");
		return singleTypeDeduction(newBasicType(TB_ERROR));
	}
	if(type.altName != NULL && strlen(type.altName) > 0){
		Type *p = NULL;
		while((p=(Type*)utarray_next(ret._types,p))){
			if(p->altName != NULL)
				free(p->altName);
			p->altName = malloc(strlen(type.altName)+1);
			strcpy(p->altName, type.altName);
		}
	}
	return ret;
}

TypeDeductions mergeTypeDeductions(TypeDeductions expected, TypeDeductions found){
	//Type ret = newBasicType(TB_ERROR);
	TypeDeductions ret = newTypeDeductions();
	Type *pf = NULL;
	Type *pe = NULL;
	int i = 0;
	while((pf=(Type*)utarray_next(found._types,pf))){
		while((pe=(Type*)utarray_next(expected._types,pe))){
			if(typesEqualMostly(*pe, *pf)){
				Type typCpy = duplicateType(*pf);
				addTypeDeductionsType(&ret, typCpy);
			}
		}
		i ++;
	}
	ret.onChooseDeduction = (expected.onChooseDeduction == NULL ?
			found.onChooseDeduction : expected.onChooseDeduction);
	ret.extra = (expected.extra == NULL ?
			found.extra : expected.extra);
	ret.extraPtr1 = (expected.extraPtr1 == NULL ?
				found.extraPtr1 : expected.extraPtr1);
	ret.extraPtr2 = (expected.extraPtr2 == NULL ?
				found.extraPtr2 : expected.extraPtr2);
	ret.extraPtr3 = (expected.extraPtr3 == NULL ?
				found.extraPtr3 : expected.extraPtr3);
	found.extra = expected.extra = NULL;
	found.onChooseDeduction = expected.onChooseDeduction = NULL;
	found.extraPtr1 = found.extraPtr2 = found.extraPtr3;
	expected.extraPtr1 = expected.extraPtr2 = expected.extraPtr3;
	return ret;
}

void showTypeDeductionMergeError(TypeDeductions expected, TypeDeductions found)
{
	reportError("TS087", "Cannot Merge Type Deductions:");
	reportTypeDeductions("LHS", expected);
	reportTypeDeductions("RHS", found);
}

void reportTypeDeductions(char *prefix, TypeDeductions td)
{
	Type *p = NULL;
	while((p=(Type*)utarray_next(td._types,p))){
		char *pref = malloc(8+strlen(prefix));
		sprintf(pref, "     %s: ",prefix);
		errShowType(pref,p);
		free(pref);
	}
}

TypeDeductions mergeTypeDeductionsOrErr(TypeDeductions expected, TypeDeductions found, int *err){
	TypeDeductions merged = mergeTypeDeductions(expected, found);
	if(utarray_len(merged._types) == 0){
		(*err) ++;
		reportError("TS087", "Cannot Merge Type Deductions:");
		reportTypeDeductions("LHS", expected);
		reportTypeDeductions("RHS", found);
	}
	return merged;
}

bool typeDeductionMergeExists(TypeDeductions expected, TypeDeductions found)
{
	Type *pf = NULL;
	Type *pe = NULL;
	int i = 0;
	while((pf=(Type*)utarray_next(found._types,pf))){
		while((pe=(Type*)utarray_next(expected._types,pe))){
			if(typesEqualMostly(*pe, *pf)){
				return true;
			}
		}
		i ++;
	}
	return false;
}

TypeDeductions duplicateTypeDeductions(TypeDeductions d)
{
	TypeDeductions ret = newTypeDeductions();
	ret.extra = d.extra;
	ret.extraPtr1 = d.extraPtr1;
	ret.extraPtr2 = d.extraPtr2;
	ret.extraPtr3 = d.extraPtr3;
	ret.onChooseDeduction = d.onChooseDeduction;
	Type *p = NULL;
	while((p=(Type*)utarray_next(d._types,p))){
		Type typCpy = duplicateType(*p);
		addTypeDeductionsType(&ret, typCpy);
	}
	return ret;
}

void showTypeDeductionOption(TypeDeductions op){
	Type *p = NULL;
	while((p=(Type*)utarray_next(op._types,p))){
		errShowType("  FOUND: ",p);
	}
}

void addVectorsOfTypeDeduction(TypeDeductions *dest, TypeDeductions in)
{
	Type *p = NULL;
	while((p=(Type*)utarray_next(in._types,p))){
		Type t = duplicateType(*p);
		Type v = newVectorType(t);
		addTypeDeductionsType(dest, v);
	}
}

void singlesOfVectorsTypeDeduction(TypeDeductions *dest, TypeDeductions in)
{
	Type *p = NULL;
	while((p=(Type*)utarray_next(in._types,p))){
		if(p->base != TB_VECTOR){
			continue;
		}
		Type t = duplicateType(((Type*)p->children)[0]);
		addTypeDeductionsType(dest, t);
	}
}

void appendToTypeDeductionAndFree(TypeDeductions *dest, TypeDeductions in){
	Type *p = NULL;
	while((p=(Type*)utarray_next(in._types,p))){
		addTypeDeductionsType(dest, duplicateType(*p));
	}
	freeTypeDeductions(in);
}



void addDictsOfTypeDeduction(TypeDeductions *dest, TypeDeductions keys, TypeDeductions values){
	Type *k = NULL;
	Type *v = NULL;
	while((k=(Type*)utarray_next(keys._types,k))){
		while((v=(Type*)utarray_next(values._types,v))){
			Type tkey = duplicateType(*k);
			Type tval = duplicateType(*v);
			Type v = newDictionaryType(tkey, tval);
			addTypeDeductionsType(dest, v);
		}
	}
}

static inline void addAllTuplesOfTypeDeductionsAux(TypeDeductions *dest, TypeDeductions *array, int cnt, int curlen, Type* tarr){
	if(cnt <= 0){
		Type tuple = newBasicType(TB_TUPLE);
		tuple.numchildren = curlen;
		tuple.children = (void*)tarr;
		addTypeDeductionsType(dest, tuple);
		return;
	}
	Type *p = NULL;
	while((p=(Type*)utarray_next(array[curlen]._types,p))){
		Type *type = malloc(sizeof(Type)*(curlen+1));
		int i;
		for(i=0; i<curlen; i++){
			*(type+i) = duplicateType(*(tarr+i));
		}
		*(type+curlen) = duplicateType(*p);
		addAllTuplesOfTypeDeductionsAux(dest, array, cnt-1, curlen+1, type);
	}
	int i;
	for(i=0; i<curlen; i++){
		freeType(*(tarr+i));
	}
	free(tarr);
}

void addAllTuplesOfTypeDeductions(TypeDeductions *dest, TypeDeductions *array, int cnt)
{
	Type *p = NULL;
	while((p=(Type*)utarray_next(array[0]._types,p))){ // first element
		Type *type = malloc(sizeof(Type));
		*type = duplicateType(*p);
		addAllTuplesOfTypeDeductionsAux(dest, array, cnt-1, 1, type); // second element
	}
}

bool isTypeNumeric(Type t)
{
  return (t.base == TB_NATIVE_INT || t.base == TB_NATIVE_FLOAT || t.base == TB_NATIVE_BOOL);
}


void addTypeDeductionsType(TypeDeductions *dest, Type t)
{
	Type *p = NULL;
	while((p=(Type*)utarray_next(dest->_types,p))){
		if(typesEqualMostly(t, *p)){
			freeType(t);
			return;
		}
	}
	utarray_push_back(dest->_types, &t);
}

void filterVectorAndDictTypes(TypeDeductions *dest, TypeDeductions in){
  Type *p = NULL;
  while((p=(Type*)utarray_next(in._types,p))){
    if(p->base != TB_VECTOR && p->base != TB_DICT){
      continue;
    }
    Type t = duplicateType(*p);
    addTypeDeductionsType(dest, t);
  }
}

void filterDictTypes(TypeDeductions *dest, TypeDeductions in){
  Type *p = NULL;
  while((p=(Type*)utarray_next(in._types,p))){
    if(p->base != TB_DICT){
      continue;
    }
    Type t = duplicateType(*p);
    addTypeDeductionsType(dest, t);
  }
}


void keysOfDictTypeDeductions(TypeDeductions *dest, TypeDeductions in){
  Type *p = NULL;
  while((p=(Type*)utarray_next(in._types,p))){
    if(p->base != TB_DICT){
      continue;
    }
    Type t = duplicateType(((Type*)p->children)[0]);
    addTypeDeductionsType(dest, t);
  }
}


void valuesOfDictTypeDeductions(TypeDeductions *dest, TypeDeductions in){
  Type *p = NULL;
  while((p=(Type*)utarray_next(in._types,p))){
    if(p->base != TB_DICT){
      continue;
    }
    Type t = duplicateType(((Type*)p->children)[1]);
    addTypeDeductionsType(dest, t);
  }
}


