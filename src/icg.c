/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icg.c
 *  Intermediate Code Generator
 *
 */

#include "icg.h"

static long long lastICGID = 0;

// all external code generators
extern ICGElm * icGenDecl(PTree *, ICGElm *);
extern void icGenDecl_print(ICGElm *, FILE*);
extern void icGenAssn_print(ICGElm *, FILE*);




ICGElm *newICGElm(ICGElm *parent, ICGElmType typ, PTree *ref)
{
	ICGElm *ret = malloc(sizeof(ICGElm));
	if(ret == NULL){
		fatalError("Out of Memory [newICGElm]\n");
	}
	ret->id = lastICGID ++;
	ret->typ = typ;

	ret->result = NULL;
	ret->op1 = NULL;
	ret->op2 = NULL;

	ret->next = NULL;
	ret->prev = (void*)parent;
	if(parent != NULL){
		parent->next = (void*)ret;
	}
	ret->ref = ref;
	return ret;
}

void freeICGElm(ICGElm *elm)
{
	if(elm->prev != NULL){
		((ICGElm*)elm->prev)->next = NULL;
		elm->prev = NULL;
	}
	if(elm->next != NULL){
		((ICGElm*)elm->next)->prev = NULL;
		freeICGElm((ICGElm*)elm->next);
		elm->next = NULL;
	}

	/*if(elm->typ == ...){
		// call free func
	}*/

	free(elm);
}

void printSingleICGElm(ICGElm *elm, FILE *f){
	if(elm->typ == ICG_NONE){
		fprintf(f, "nop");
	}else if(elm->typ == ICG_DEFINE_INT8 || elm->typ == ICG_DEFINE_INT16 ||
			elm->typ == ICG_DEFINE_INT32 || elm->typ == ICG_DEFINE_INT64 ||
			elm->typ == ICG_DEFINE_FLOAT32 || elm->typ == ICG_DEFINE_FLOAT64 ||
			elm->typ == ICG_DEFINE_PTR){
		icGenDecl_print(elm, f);
	}else if(elm->typ == ICG_MOVL_INT8 || elm->typ == ICG_MOVL_INT16 ||
			elm->typ == ICG_MOVL_INT32 || elm->typ == ICG_MOVL_INT64 ||
			elm->typ == ICG_MOVL_FLOAT32 || elm->typ == ICG_MOVL_FLOAT64){
		icGenAssn_print(elm, f);
	}
}

void printICG(ICGElm *root, FILE *f)
{
	while(root != NULL){
		fprintf(f, "%16lx: ", root->id);
		printSingleICGElm(root, f);
		fprintf(f, "\n");
		root = (ICGElm*)root->next;
	}
}

ICGElm *icGen(PTree *root, ICGElm *prev)
{
	// for every type of statement:
	if(root->typ == PTT_DECL){
		prev = icGenDecl(root, prev);
	}else if(root->typ == PTT_INT || root->typ == PTT_FLOAT){
		return newICGElm(NULL, ICG_LITERAL, root);
	}else{
		//fatalError("ICG Code GEN: Unknown Tree Expression: %s", getParseNodeName(root));
		fprintf(stderr, "ICG Code GEN: Unknown Tree Expression: %s", getParseNodeName(root));
	}
	return prev;
}

void icRunGen(PTree *root)
{
	ICGElm *icgroot = newICGElm(NULL, ICG_NONE, NULL);
	ICGElm *ptr = icgroot;

	while(root != NULL && root->typ == PTT_STMTBLOCK){
		PTree *gen = (PTree*)root->child1;
		ptr = icGen(gen, ptr);
		root = (PTree*)root->child2;
	}
	printICG(icgroot, stdout);
	freeICGElm(icgroot);
}


