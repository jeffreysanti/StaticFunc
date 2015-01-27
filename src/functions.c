/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  functions.c
 *  Function Storage Container
 *
 */

#include "functions.h"

NamedFunctionMapEnt *NFM = NULL;
PTreeFreeList *FL = NULL;


void initFunctionSystem()
{

}

void freeFreeList(PTreeFreeList *f){
	if(f == NULL) return;
	freeFreeList((PTreeFreeList*)f->next);
	if(f->tree != NULL) freeParseTreeNode(f->tree);
	free(f);
}

void freeFunctionSystem()
{
	freeFreeList(FL);
	NamedFunctionMapEnt *ent, *tmp;
	HASH_ITER(hh, NFM, ent, tmp) {
		HASH_DEL(NFM, ent);
		free(ent->funcName);
		freeFunctionVersion(ent->V);
		free(ent);
	}
}

void addTreeToFreeList(PTree *root)
{
	if(FL == NULL){
		FL = malloc(sizeof(PTreeFreeList));
		if(FL == NULL){
			fatalError("Out of Memory [addTreeToFreeList]\n");
		}
		FL->next = NULL;
		FL->tree = root;
		return;
	}
	PTreeFreeList *ptr = FL;
	while(ptr->next != NULL){
		ptr = (PTreeFreeList*)ptr->next;
	}
	ptr->next = malloc(sizeof(PTreeFreeList));
	if(ptr->next == NULL){
		fatalError("Out of Memory [addTreeToFreeList]\n");
	}
	((PTreeFreeList*)ptr->next)->next = NULL;
	((PTreeFreeList*)ptr->next)->tree = root;
	return;
}


FunctionVersion *newFunctionVersion(FunctionVersion *parent)
{
	FunctionVersion *ret = malloc(sizeof(FunctionVersion));
	if(ret == NULL){
		fatalError("Out of Memory [newFunctionVersion]\n");
	}
	ret->defRoot = NULL;
	ret->next = NULL;
	ret->performReplacement = false;
	ret->stat = FS_LISTED;
	ret->sig = newBasicType(TB_NATIVE_VOID);
	if(parent != NULL){
		parent->next = (void*)ret;
	}
	return ret;
}

FunctionVersion *newFunctionVersionByName(char *nm)
{
	NamedFunctionMapEnt  *ent;
	HASH_FIND_STR(NFM, nm, ent);
	if(ent){
		FunctionVersion *fVer = ent->V;
		while(fVer->next != NULL){
			fVer = (FunctionVersion*)fVer->next;
		}
		return newFunctionVersion(fVer);
	}
	char *nmcpy = malloc(strlen(nm)+1);
	strcpy(nmcpy, nm);
	if(nmcpy == NULL){
		fatalError("Out of Memory [newFunctionVersionByName : nmcpy]\n");
	}
	ent = malloc(sizeof(NamedFunctionMapEnt));
	if(ent == NULL){
		fatalError("Out of Memory [newFunctionVersionByName : ent]\n");
	}
	ent->funcName = nmcpy;
	ent->V = newFunctionVersion(NULL);
	HASH_ADD_KEYPTR(hh, NFM, ent->funcName, strlen(ent->funcName), ent);
	return (FunctionVersion*)ent->V;
}

void freeFunctionVersion(FunctionVersion *fv)
{
	if(fv == NULL) return;
	freeFunctionVersion((FunctionVersion*)fv->next);
	freeType(fv->sig);
	free(fv);
}



inline void registerFunction(PTree *root, int paramCnt, char *needle, Type haystack)
{
	FunctionVersion *fVer = newFunctionVersionByName((char*)root->tok->extra);
	fVer->defRoot = root;
	fVer->performReplacement = (needle != NULL);
	Type sig = newBasicType(TB_FUNCTION);
	allocTypeChildren(&sig, paramCnt + 1);
	PTree *typeTree = (PTree*)root->child1;
	PTree *retTypeTree = (PTree*)typeTree->child1;
	PTree *paramTypeTree = (PTree*)typeTree->child2;

	// First, Return Type
	if(fVer->performReplacement && getDeclTypeListName(retTypeTree) != NULL){
		((Type*)sig.children)[0] = substituteTypeTemplate(deduceTypeDeclType(retTypeTree), haystack);
	}else{
		((Type*)sig.children)[0] = deduceTypeDeclType(retTypeTree);
	}

	int i;
	for(i=1; i<=paramCnt; i++){
		PTree *pTempRoot = (PTree*)((PTree*)paramTypeTree->child1)->child1;
		if(fVer->performReplacement && strcmp(needle, (char*)pTempRoot->tok->extra) == 0){
			((Type*)sig.children)[i] = substituteTypeTemplate(deduceTypeDeclType(pTempRoot), haystack);
		}else{
			((Type*)sig.children)[i] = deduceTypeDeclType(pTempRoot);
		}
		paramTypeTree = (PTree*)paramTypeTree->child2;
	}
	fVer->sig = sig;
	printf("Registerd Function: %s\n", (char*)fVer->defRoot->tok->extra);
}

void seperateFunctionsFromParseTree(PTree *root)
{
	// transverse parse tree finding all functions at root level
	while(root != NULL && root->child1 != NULL){
		if(((PTree*)root->child1)->typ == PTT_FUNCTION){ // bingo
			PTree *funcRoot = (PTree*)root->child1;
			funcRoot->parent = NULL;
			root->child1 = NULL;
			addTreeToFreeList(funcRoot);
			PTree *typeTree = (PTree*)funcRoot->child1;
			PTree *paramTypeTree = (PTree*)typeTree->child2;
			bool isTemplate = false;
			char *templateIdent = NULL;
			Type Treturn = deduceTypeDeclType((PTree*)typeTree->child1); // return type
			if(Treturn.base == TB_ERROR){
				reportError("FS001", "Function has invalid return type: Line %ld", funcRoot->tok->lineNo);
				root = (PTree*)root->child2;
				freeType(Treturn);
				continue;
			}
			if(Treturn.hasTypeListParam){
				isTemplate = true;
				templateIdent = getDeclTypeListName((PTree*)typeTree->child1);
			}
			int paramCnt = 0;
			bool err = false;
			while(paramTypeTree != NULL){
				paramCnt ++;
				Type tempParamType = deduceTypeDeclType((PTree*)((PTree*)paramTypeTree->child1)->child1);
				if(tempParamType.base == TB_ERROR){
					reportError("FS002", "Function has invalid param %d type: Line %ld", paramCnt,
							funcRoot->tok->lineNo);
					err = true;
					break;
				}
				if(isTemplate && tempParamType.hasTypeListParam &&
						strcmp(getDeclTypeListName((PTree*)((PTree*)paramTypeTree->child1)->child1), templateIdent) != 0){
					reportError("FS003", "Function cannot have multiple template params: Line %ld",
												funcRoot->tok->lineNo);
					err = true;
					break;
				}
				if(tempParamType.hasTypeListParam){
					isTemplate = true;
					templateIdent = getDeclTypeListName((PTree*)typeTree->child1);
				}
				freeType(tempParamType);
				paramTypeTree = (PTree*)paramTypeTree->child2;
			}
			if(err){
				freeType(Treturn);
				root = (PTree*)root->child2;
				continue;
			}
			//printf("%d ARGS\n", paramCnt);
			if(isTemplate){
				TypeList tl = getTypeListByName(templateIdent);
				registerFunction(funcRoot, paramCnt, templateIdent, tl.T);
				TypeList *tlptr = (TypeList*)tl.next;
				while(tlptr != NULL){
					registerFunction(funcRoot, paramCnt, templateIdent, tlptr->T);
					tlptr = (TypeList*)tlptr->next;
				}
			}else{
				registerFunction(funcRoot, paramCnt, NULL, newBasicType(TB_NATIVE_VOID));
			}
			freeType(Treturn);
		}
		root = (PTree*)root->child2;
	}
}
