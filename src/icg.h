/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icg.h
 *  Intermediate Code Generator
 *
 */

#ifndef STATICFUNC_SRC_ICG_H_
#define STATICFUNC_SRC_ICG_H_

#include "parsetree.h"
#include "symbols.h"

// ICCodeGen Element (Linked List)

typedef enum{
	ICG_NONE,
	ICG_LITERAL,
	ICG_IDENT,

	ICG_DEFINE_INT8,
	ICG_DEFINE_INT16,
	ICG_DEFINE_INT32,
	ICG_DEFINE_INT64,
	ICG_DEFINE_FLOAT32,
	ICG_DEFINE_FLOAT64,
	ICG_DEFINE_PTR,
	ICG_MOV
}ICGElmType;

typedef enum{
	ICGO_LIT_INT8,
	ICGO_LIT_INT16,
	ICGO_LIT_INT32,
	ICGO_LIT_INT64,
	ICGO_LIT_FLOAT32,
	ICGO_LIT_FLOAT64,

	ICGO_VAR_INT8,
	ICGO_VAR_INT16,
	ICGO_VAR_INT32,
	ICGO_VAR_INT64,
	ICGO_VAR_FLOAT32,
	ICGO_VAR_FLOAT64,
	ICGO_VAR_PTR,

	ICGO_IDENT
}ICGElmOpType;

typedef struct{
	ICGElmOpType typ;
	char *data;
}ICGElmOp;

typedef struct{
	ICGElmType typ;
	unsigned long id;

	ICGElmOp *result;
	ICGElmOp *op1;
	ICGElmOp *op2;

	ICGElmOp *resultb;
	ICGElmOp *op1b;
	ICGElmOp *op2b;

	PTree *ref;

	struct ICGElm *prev;
	struct ICGElm *next;

} ICGElm;

void icRunGen(PTree *root);
ICGElm * icGen(PTree *root, ICGElm *prev);


char *newTempVariable(Type t);

ICGElm *newICGElm(ICGElm *parent, ICGElmType typ, PTree *ref);
void freeICGElm(ICGElm *elm);
void printSingleICGElm(ICGElm *elm, FILE *f);
void printICG(ICGElm *root, FILE *f);

ICGElmOp *newOp(ICGElmOpType typ, char *data);
ICGElmOp *newOpLiteral(Type assignType, char *data);
ICGElmOp *newOpVariable(Type assignType, char *data);

bool literalOp(ICGElmOp *op);

#endif /* STATICFUNC_SRC_ICG_H_ */
