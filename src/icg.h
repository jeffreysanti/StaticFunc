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

	ICG_DEFINE,
	ICG_MOV,
	ICG_OBJCOPY,

	ICG_ADD,
	ICG_SUB,
	ICG_MUL,
	ICG_DIV
}ICGElmType;

typedef enum{
	ICGDT_NONE,
	ICGDT_INT8,
	ICGDT_INT16,
	ICGDT_INT32,
	ICGDT_INT64,
	ICGDT_FLOAT32,
	ICGDT_FLOAT64,
	ICGDT_PTR
}ICGDataType;

typedef enum{
	ICGO_LIT,
	ICGO_REG,
	ICGO_RO_ADDR,

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

	ICGDataType dataType;

	PTree *ref;

	struct ICGElm *prev;
	struct ICGElm *next;

} ICGElm;

struct ROStringLit {
    const char *varname;
    const char *rodata;
    UT_hash_handle hh;
};

void icRunGen(PTree *root);
ICGElm * icGen(PTree *root, ICGElm *prev);


char *newTempVariable(Type t);
char *newROStringLit(char *str);

ICGElm *newICGElm(ICGElm *parent, ICGElmType typ, ICGDataType dt, PTree *ref);
void freeICGElm(ICGElm *elm);
void printSingleICGElm(ICGElm *elm, FILE *f);
void printICG(ICGElm *root, FILE *f);

ICGElmOp *newOp(ICGElmOpType typ, char *data);
ICGElmOp *newOpCopyData(ICGElmOpType typ, char *data);

ICGDataType typeToICGDataType(Type t);
void printICGTypeSuffix(ICGElm *elm, FILE* f);

#endif /* STATICFUNC_SRC_ICG_H_ */
