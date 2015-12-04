/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  functions.c
 *  Function Storage Container
 *
 */

#include "functions.h"

UT_icd FunctionVersion_icd = {sizeof(FunctionVersion), NULL, NULL, NULL};
UT_icd SearchAndReplace_icd = {sizeof(SearchAndReplace), NULL, NULL, NULL};


NamedFunctionMapEnt *NFM = NULL;
PTreeFreeList *FL = NULL;

UT_array *FUSED = NULL;

int lastid = 1;

void initFunctionSystem()
{
	utarray_new(FUSED, &FunctionVersion_icd);
}

void freeFreeList(PTreeFreeList *f){
	if(f == NULL) return;
	freeFreeList((PTreeFreeList*)f->next);
	if(f->tree != NULL){
		//dumpParseTreeDet(f->tree, 0);
		freeParseTreeNode(f->tree);
	}
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

	utarray_free(FUSED);
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


FunctionVersion *newFunctionVersion()
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
	ret->verid = 1;
	ret->sr = NULL;
	ret->funcName = NULL;
	//if(parent != NULL){
		//parent->next = (void*)ret;
		//ret->verid = parent->verid + 1;
	//}
	ret->verid = 0;
	return ret;
}

static inline bool addFunctionVerToList_checkAlreadyExists(char *nm, FunctionVersion *verOld, FunctionVersion *ver)
{
	if(typesEqualMostly(ver->sig, verOld->sig)){
		if(ver->performReplacement){
			reportError("@FS051", "Notice: Function template overridden: %s\n"
					"\t\tUsing line %d instead of line %d",
					nm, verOld->defRoot->tok->lineNo, ver->defRoot->tok->lineNo);
			errShowType("\tSIGNATURE: ", (void*)&ver->sig);
		}else{
			reportError("#FS050", "Warning: Function already defined %s\n"
					"\t\tUsing line %d instead of line %d",
					nm, verOld->defRoot->tok->lineNo, ver->defRoot->tok->lineNo);
			errShowType("\tSIGNATURE: ", (void*)&ver->sig);
		}
		freeFunctionVersion(ver);
		return false;
	}
	return true;
}

bool addFunctionVerToList(char *nm, FunctionVersion *ver)
{
	NamedFunctionMapEnt  *ent;
	HASH_FIND_STR(NFM, nm, ent);
	if(ent){
		FunctionVersion *fVer = ent->V;
		if(!addFunctionVerToList_checkAlreadyExists(nm, fVer, ver))
			return false;
		while(fVer->next != NULL){
			if(!addFunctionVerToList_checkAlreadyExists(nm, fVer, ver))
				return false;
			fVer = (FunctionVersion*)fVer->next;
		}
		fVer->next = (void*)ver;
		ver->funcName = fVer->funcName;
		return true;
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
	ent->V = ver;
	ver->funcName = ent->funcName;
	ent->nameid = lastid;
	lastid ++;
	HASH_ADD_KEYPTR(hh, NFM, ent->funcName, strlen(ent->funcName), ent);
	return true;
}

void freeFunctionVersion(FunctionVersion *fv)
{
	if(fv == NULL) return;
	freeFunctionVersion((FunctionVersion*)fv->next);
	freeType(fv->sig);
	if(fv->sr != NULL){
		SearchAndReplace *sar;
		for(sar=(SearchAndReplace*)utarray_front(fv->sr);sar!=NULL; sar=(SearchAndReplace*)utarray_next(fv->sr,sar)) {
			free(sar->ident);
			freeType(sar->replace);
		}
		utarray_free(fv->sr);
	}
	free(fv);
}



static inline void registerFunction(PTree *root, int paramCnt, UT_array *sr)
{
	FunctionVersion *fVer = newFunctionVersion();
	fVer->defRoot = root;
	fVer->performReplacement = (sr != NULL);
	Type sig = newBasicType(TB_FUNCTION);
	allocTypeChildren(&sig, paramCnt + 1);
	PTree *typeTree = (PTree*)root->child1;
	PTree *retTypeTree = (PTree*)typeTree->child1;
	PTree *paramTypeTree = (PTree*)typeTree->child2;

	// First, Return Type
	if(fVer->performReplacement && getDeclTypeListName(retTypeTree) != NULL){
		SearchAndReplace *sar;
		((Type*)sig.children)[0] = deduceTypeDeclType(retTypeTree);
		for(sar=(SearchAndReplace*)utarray_front(sr);sar!=NULL; sar=(SearchAndReplace*)utarray_next(sr,sar)) {
			((Type*)sig.children)[0] = substituteTypeTemplate(((Type*)sig.children)[0],
											sar->replace,
											sar->ident);
		}
	}else{
		((Type*)sig.children)[0] = deduceTypeDeclType(retTypeTree);
	}

	int i;
	for(i=1; i<=paramCnt; i++){
		PTree *pTempRoot = (PTree*)((PTree*)paramTypeTree->child1)->child1;
		if(fVer->performReplacement){
			SearchAndReplace *sar;
			((Type*)sig.children)[i] = deduceTypeDeclType(pTempRoot);
			for(sar=(SearchAndReplace*)utarray_front(sr);sar!=NULL; sar=(SearchAndReplace*)utarray_next(sr,sar)) {
				((Type*)sig.children)[i] = substituteTypeTemplate(((Type*)sig.children)[i],
												sar->replace,
												sar->ident);
			}
		}else{
			((Type*)sig.children)[i] = deduceTypeDeclType(pTempRoot);
		}
		paramTypeTree = (PTree*)paramTypeTree->child2;
	}
	fVer->sig = sig;
	if(fVer->performReplacement){
		utarray_new(fVer->sr, &SearchAndReplace_icd);
		SearchAndReplace *sar;
		for(sar=(SearchAndReplace*)utarray_front(sr);sar!=NULL; sar=(SearchAndReplace*)utarray_next(sr,sar)) {
			SearchAndReplace sar_cpy;
			sar_cpy.ident = malloc(sizeof(char) * (strlen(sar->ident) + 1));
			strcpy(sar_cpy.ident, sar->ident);
			sar_cpy.replace = duplicateType(sar->replace);
			utarray_push_back(fVer->sr, &sar_cpy);
		}
	}

	if(addFunctionVerToList((char*)fVer->defRoot->tok->extra, fVer)){
		printf("Registerd Function: %s\n", (char*)fVer->defRoot->tok->extra);
	}
}

int typeListLength(TypeList tl){
	int cnt = 1;
	TypeList *ptr = (TypeList *)tl.next;
	while(ptr != NULL){
		cnt ++;
		ptr = (TypeList *)ptr->next;
	}
	return cnt;
}
Type typeListGet(TypeList tl, int i){
	TypeList *ptr = &tl;
	int x = 0;
	while(x < i){
		ptr = (TypeList *)ptr->next;
		x ++;
	}
	return ptr->T;
}

void registerAllTemplateVersions(PTree *root, int pCount, UT_array *templateList, int tNo, UT_array *sr)
{
	char *search = *((char**)utarray_eltptr(templateList, tNo));
	TypeList tl = getTypeListByName(search);
	int entCount = typeListLength(tl);

	int tCnt = utarray_len(templateList);
	if(tCnt-1 == tNo){ // end case
		int ent;
		for(ent=0; ent<entCount; ent++){
			SearchAndReplace sar;
			sar.ident = search;
			sar.replace = duplicateType(typeListGet(tl, ent));
			utarray_push_back(sr, &sar);
			// REGISTER IT
			registerFunction(root, pCount, sr);
			utarray_pop_back(sr);
		}
	}else{ // branch
		int ent;
		for(ent=0; ent<entCount; ent++){
			SearchAndReplace sar;
			sar.ident = search;
			sar.replace = duplicateType(typeListGet(tl, ent));
			utarray_push_back(sr, &sar);
			registerAllTemplateVersions(root, pCount, templateList, tNo+1, sr); // RECURSE
			utarray_pop_back(sr);
		}
	}
}

void seperateFunctionsFromParseTree(PTree **topRoot, bool templatePass)
{
	PTree *root = *topRoot;

	// transverse parse tree finding all functions at root level
	while(root != NULL && root->child1 != NULL){
		if(((PTree*)root->child1)->typ == PTT_FUNCTION){ // bingo
			PTree *funcRoot = (PTree*)root->child1;
			funcRoot->parent = NULL;
			root->child1 = NULL;
			PTree *typeTree = (PTree*)funcRoot->child1;
			PTree *paramTypeTree = (PTree*)typeTree->child2;
			bool isTemplate = false;

			UT_array *templateIdentList;
			utarray_new(templateIdentList, &ut_str_icd);

			Type Treturn = deduceTypeDeclType((PTree*)typeTree->child1); // return type
			if(Treturn.base == TB_ERROR){
				reportError("FS001", "Function has invalid return type: Line %ld", funcRoot->tok->lineNo);
				root = (PTree*)root->child2;
				freeType(Treturn);
				utarray_free(templateIdentList);
				addTreeToFreeList(funcRoot);
				continue;
			}
			if(Treturn.hasTypeListParam){
				isTemplate = true;
				char *tmp = getDeclTypeListName((PTree*)typeTree->child1);
				utarray_push_back(templateIdentList, &tmp);
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
				if(tempParamType.hasTypeListParam){
					isTemplate = true;
					char *tmp = getDeclTypeListName((PTree*)((PTree*)paramTypeTree->child1)->child1);
					char **p = NULL;
					bool toAdd = true;
					while ( (p=(char**)utarray_next(templateIdentList,p))) {
						if(strcmp(*p, tmp) == 0)
							toAdd = false;
					}
					if(toAdd)
						utarray_push_back(templateIdentList, &tmp);
				}
				freeType(tempParamType);
				paramTypeTree = (PTree*)paramTypeTree->child2;
			}
			if(err){
				addTreeToFreeList(funcRoot);
				freeType(Treturn);
				utarray_free(templateIdentList);
				root = (PTree*)root->child2;
				continue;
			}
			//printf("%d ARGS\n", paramCnt);

			if(isTemplate && !templatePass){ // in first pass we skip templates
				freeType(Treturn);
				utarray_free(templateIdentList);

				// add back to parse tree for now
				root->child1 = (void*)funcRoot;
				funcRoot->parent = (void*)root;

				root = (PTree*)root->child2;
				continue;
			}
			addTreeToFreeList(funcRoot);
			if(isTemplate){
				/*TypeList tl = getTypeListByName(templateIdent);
				registerFunction(funcRoot, paramCnt, templateIdent, tl.T);
				TypeList *tlptr = (TypeList*)tl.next;
				while(tlptr != NULL){
					registerFunction(funcRoot, paramCnt, templateIdent, tlptr->T);
					tlptr = (TypeList*)tlptr->next;
				}*/

				UT_array *sr;
				utarray_new(sr, &SearchAndReplace_icd);
				registerAllTemplateVersions(funcRoot, paramCnt, templateIdentList, 0, sr);
				utarray_free(sr);
			}else{
				registerFunction(funcRoot, paramCnt, NULL);
			}
			freeType(Treturn);
			utarray_free(templateIdentList);
		}
		root = (PTree*)root->child2;
	}
	if(!templatePass){
		cleanUpEmptyStatments(*topRoot);
		seperateFunctionsFromParseTree(topRoot, true);
		return;
	}
	cleanUpEmptyStatments(*topRoot);
}


NamedFunctionMapEnt *getFunctionVersions(char *nm)
{
	NamedFunctionMapEnt  *ent;
	HASH_FIND_STR(NFM, nm, ent);
	return ent;
}

PTree *copyTreeSubTemplate(PTree *orig, UT_array *sr){
	PTree *node = newParseTree(orig->typ);
	if(node->typ == PTT_DECL_TYPE){
		SearchAndReplace *sar;
		for(sar=(SearchAndReplace*)utarray_front(sr);sar!=NULL; sar=(SearchAndReplace*)utarray_next(sr,sar)) {
			if(strcmp(sar->ident, (char*)orig->tok->extra) == 0){
				freeParseTreeNode(node);
				node = (PTree*)getTypeAsPTree(sar->replace);
				if(orig->child1 != NULL && ((PTree*)orig->child1)->typ == PTT_DECL_MOD){
				}
				return node;
			}
		}
	}

	freeTypeDeductions(node->deducedTypes);
	node->deducedTypes = duplicateTypeDeductions(orig->deducedTypes);
	node->tok = orig->tok;
	if(orig->child1 != NULL){
		node->child1 = (void*)copyTreeSubTemplate((PTree*)orig->child1, sr);
		((PTree*)node->child1)->parent = (void*)node;
	}
	if(orig->child2 != NULL){
		node->child2 = (void*)copyTreeSubTemplate((PTree*)orig->child2, sr);
		((PTree*)node->child2)->parent = (void*)node;
	}
	return node;
}

void markFunctionVersionUsed(FunctionVersion *fver)
{
	if(fver->stat == FS_LISTED){
		fver->stat = FS_CALLED;

		if(fver->performReplacement){
			// need to split parse tree
			PTree *copy = copyTreeSubTemplate(fver->defRoot, fver->sr);
			fver->defRoot = copy;
			addTreeToFreeList(copy);
		}

		utarray_push_back(FUSED, fver);
	}
}

FunctionVersion *markFirstUsedVersionChecked()
{
	FunctionVersion *ret = (FunctionVersion *)utarray_back(FUSED);
	if(ret == NULL)
		return NULL;
	utarray_pop_back(FUSED); // remove from the used list
	ret->stat = FS_CHECKED;
	return ret;
}

void registerNativeFunction(char *nm, Type sig)
{
	FunctionVersion *fv = newFunctionVersion();
	fv->defRoot = NULL;
	fv->stat = FS_NATIVE;
	fv->sig = sig;
	addFunctionVerToList(nm, fv);
}

