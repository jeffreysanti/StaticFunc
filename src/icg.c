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
static long lastTempVar = 0;

// all external code generators
extern ICGElm * icGenDecl(PTree *, ICGElm *);
extern void icGenDecl_print(ICGElm *, FILE*);

extern void icGenAssn_print(ICGElm *, FILE*);

extern ICGElm * icGenArtih(PTree *, ICGElm *);
extern void icGenArtih_print(ICGElm *, FILE*);



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

	ret->resultb = NULL;
	ret->op1b = NULL;
	ret->op2b = NULL;

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

	if(elm->result != NULL){
		if(elm->result->typ != ICGO_IDENT){
			free(elm->result->data);
		}
		free(elm->result);
	}
	if(elm->op1 != NULL){
		if(elm->op1->typ != ICGO_IDENT){
			free(elm->op1->data);
		}
		free(elm->op1);
	}
	if(elm->op2 != NULL){
		if(elm->op2->typ != ICGO_IDENT){
			free(elm->op2->data);
		}
		free(elm->op2);
	}

	if(elm->resultb != NULL){
		free(elm->resultb->data);
		free(elm->resultb);
	}
	if(elm->op1b != NULL){
		free(elm->op1b->data);
		free(elm->op1b);
	}
	if(elm->op2b != NULL){
		free(elm->op2b->data);
		free(elm->op2b);
	}


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
	}else if(elm->typ == ICG_MOV){
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

ICGElmOp *newOpLiteral(Type assignType, char *data){

	char *newData = malloc(strlen(data)+1);
	strcpy(newData, data);

	if(assignType.base == TB_NATIVE_INT8 || assignType.base == TB_NATIVE_BOOL){
		return newOp(ICGO_LIT_INT8, newData);
	}else if(assignType.base == TB_NATIVE_INT16){
		return newOp(ICGO_LIT_INT16, newData);
	}else if(assignType.base == TB_NATIVE_INT32){
		return newOp(ICGO_LIT_INT32, newData);
	}else if(assignType.base == TB_NATIVE_INT64){
		return newOp(ICGO_LIT_INT64, newData);
	}else if(assignType.base == TB_NATIVE_FLOAT32){
		return newOp(ICGO_LIT_FLOAT32, newData);
	}else if(assignType.base == TB_NATIVE_FLOAT64){
		return newOp(ICGO_LIT_FLOAT64, newData);
	}else{
		fatalError("Unknown Literal Assignment!");
	}
	return NULL;
}

ICGElmOp *newOpVariable(Type assignType, char *data){
	char *newData = malloc(strlen(data)+1);
	strcpy(newData, data);

	if(assignType.base == TB_NATIVE_INT8 || assignType.base == TB_NATIVE_BOOL){
		return newOp(ICGO_VAR_INT8, newData);
	}else if(assignType.base == TB_NATIVE_INT16){
		return newOp(ICGO_VAR_INT16, newData);
	}else if(assignType.base == TB_NATIVE_INT32){
		return newOp(ICGO_VAR_INT32, newData);
	}else if(assignType.base == TB_NATIVE_INT64){
		return newOp(ICGO_VAR_INT64, newData);
	}else if(assignType.base == TB_NATIVE_FLOAT32){
		return newOp(ICGO_VAR_FLOAT32, newData);
	}else if(assignType.base == TB_NATIVE_FLOAT64){
		return newOp(ICGO_VAR_FLOAT64, newData);
	}else{
		return newOp(ICGO_VAR_PTR, newData);
	}
	return NULL;
}

ICGElm *icGen(PTree *root, ICGElm *prev)
{
	// for every type of statement:
	if(root->typ == PTT_INT || root->typ == PTT_FLOAT){
		return newICGElm(NULL, ICG_LITERAL, root);
	}else if(root->typ == PTT_IDENTIFIER){
		return newICGElm(NULL, ICG_IDENT, root);
	}else if(root->typ == PTT_DECL){
		prev = icGenDecl(root, prev);
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT ||
			root->typ == PTT_DIV){
		//prev = icGenArith(root, prev);
	}else{
		//fatalError("ICG Code GEN: Unknown Tree Expression: %s", getParseNodeName(root));
		fprintf(stderr, "ICG Code GEN: Unknown Tree Expression: %s\n", getParseNodeName(root));
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

char *newTempVariable(Type t)
{
	lastTempVar ++;
	char *nm = calloc(26, 1);
	sprintf(nm, ".temp%ld", lastTempVar);
	addSymbol(nm, duplicateType(t));
	return nm;
}

ICGElmOp *newOp(ICGElmOpType typ, char *data)
{
	ICGElmOp *ret = malloc(sizeof(ICGElmOp));
	if(ret == NULL){
		fatalError("Out of Memory [newOp]\n");
	}
	ret->data = data;
	ret->typ = typ;
	return ret;
}

bool literalOp(ICGElmOp *op)
{
	return (op->typ == ICGO_LIT_INT8 || op->typ == ICGO_LIT_INT16 ||
			op->typ == ICGO_LIT_INT32 || op->typ == ICGO_LIT_INT64 ||
			op->typ == ICGO_LIT_FLOAT32 || op->typ == ICGO_LIT_FLOAT64);
}

