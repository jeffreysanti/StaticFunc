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
	ICG_DEFINE_INT8,
	ICG_DEFINE_INT16,
	ICG_DEFINE_INT32,
	ICG_DEFINE_INT64,
	ICG_DEFINE_FLOAT32,
	ICG_DEFINE_FLOAT64,
	ICG_DEFINE_PTR,
	ICG_MOVL_INT8,
	ICG_MOVL_INT16,
	ICG_MOVL_INT32,
	ICG_MOVL_INT64,
	ICG_MOVL_FLOAT32,
	ICG_MOVL_FLOAT64
}ICGElmType;

typedef struct{
	ICGElmType typ;
	unsigned long id;

	void *result;
	void *op1;
	void *op2;

	PTree *ref;

	struct ICGElm *prev;
	struct ICGElm *next;

} ICGElm;

void icRunGen(PTree *root);
ICGElm * icGen(PTree *root, ICGElm *prev);


ICGElm *newICGElm(ICGElm *parent, ICGElmType typ, PTree *ref);
void freeICGElm(ICGElm *elm);
void printSingleICGElm(ICGElm *elm, FILE *f);
void printICG(ICGElm *root, FILE *f);


#endif /* STATICFUNC_SRC_ICG_H_ */
