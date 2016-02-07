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

#define MAX_POINTER_SIZE   16

typedef enum{
	ICG_NONE,
	ICG_LITERAL,
	ICG_IDENT,

	ICG_LBL,

	ICG_DEFINE,
	ICG_MOV,
	ICG_OBJCOPY,

	ICG_ADD,
	ICG_SUB,
	ICG_MUL,
	ICG_DIV,
	ICG_AND,
	ICG_OR,
	ICG_NOT,
	ICG_XOR,
	ICG_GT,
	ICG_GTE,
	ICG_LT,
	ICG_LTE,
	ICG_SHR,
	ICG_SHL,
	ICG_EXP,
	ICG_MOD,

	ICG_NEWVEC,
	ICG_VECSTORE,
	ICG_VECLOAD,

	ICG_VPUSH,
	ICG_VPOP,
	ICG_VQUEUE,
	ICG_VDEQUEUE,
	ICG_VSIZE,

	ICG_VREMOVE,
	ICG_VCONTAINS,
	ICG_DREMOVE,
	ICG_DCONTAINS,

	ICG_DKEYS,
	ICG_DVALS,
	
	ICG_NEWSET,
	ICG_SETSTORE,

	ICG_NEWDICT,
	ICG_DICTSTORE,
	ICG_DICTLOAD,

	ICG_NEWTUPLE,
	ICG_TPLSTORE,
	ICG_TPLLOAD,
	
	ICG_ITER_INIT,
	ICG_ITER_NEXT,
	ICG_ITER_CLOSE,

	ICG_JNZ,
	ICG_JZ,
	ICG_JMP,

	ICG_COMPOBJ,
	ICG_DR
}ICGElmType;

typedef enum{
	ICGDT_NONE,
	ICGDT_INT,
	ICGDT_FLOAT,
	ICGDT_PTR
}ICGDataType;

typedef enum{
	ICGO_NUMERICLIT,
	ICGO_REG,
	ICGO_RO_ADDR,
	ICGO_OBJREFNEW,

	ICGO_LABEL

	//ICGO_IDENT
}ICGElmOpType;

typedef struct{
	ICGElmOpType typ;
	void *data;
}ICGElmOp;

typedef struct{
	ICGElmType typ;
	unsigned long id;

	ICGElmOp *result;
	ICGElmOp *op1;
	ICGElmOp *op2;
	ICGElmOp *op3;

	/*ICGElmOp *resultb;
	ICGElmOp *op1b;
	ICGElmOp *op2b;
	ICGElmOp *op3b;*/

	ICGDataType dataType;

	PTree *ref;

	struct ICGElm *prev;
	struct ICGElm *next;

} ICGElm;

struct ROStringLit {
    char *varname;
    char *rodata;
    UT_hash_handle hh;
};

void icRunGen(PTree *root, char *outfl);
ICGElm * icGenBlock(PTree *root, ICGElm *prev);
ICGElm * icGen(PTree *root, ICGElm *prev);


char *newROStringLit(char *str);
char *newLabel(char *base);

ICGElm *newICGElm(ICGElm *parent, ICGElmType typ, ICGDataType dt, PTree *ref);
void freeICGElm(ICGElm *elm);
void printSingleICGElm(ICGElm *elm, FILE *f);
void printICG(ICGElm *root, FILE *f, bool address);

ICGElmOp *newOp(ICGElmOpType typ, Variable *data);
ICGElmOp *newOpInt(int val);
ICGElmOp *newOpInt_s(char* val);
ICGElmOp *newOpInt_sc(char* val);
ICGElmOp *newOpCopy(ICGElmOp *cpy);
ICGElmOp *newOpROA_c(char *val);
ICGElmOp *newOpLabel_c(char *val);

void printOp(FILE *f, ICGElmOp *op);

//ICGElmOp *newOpCopyData(ICGElmOpType typ, char *data);

ICGDataType typeToICGDataType(Type t);
void printICGTypeSuffix(ICGElm *elm, FILE* f);


ICGElm *derefScope(ICGElm *prev);





static inline ICGElmOp *bitSizeTupleOp(Type t){
	char *dta = calloc(t.numchildren*2 + 1, 1);
	char *origDta = dta;
	int i;
	for(i=0; i<t.numchildren; i++){
		Type child = ((Type*)t.children)[i];
		if(child.base == TB_NATIVE_INT || child.base == TB_NATIVE_FLOAT){
			sprintf(dta, "8;");
			dta += 2;
		}
		else{
			sprintf(dta, "P;");
			dta += 2;
		}
	}
	return newOpInt_s(origDta);
}

static inline ICGElmOp *bitSizeOp(Type t){
	ICGElmOp *op = NULL;
	if(t.base == TB_NATIVE_INT || t.base == TB_NATIVE_FLOAT){
	  op = newOpInt_sc("8");
	}else{
	  op = newOpInt_sc("PTR");
	}
	return op;
}


extern FILE *openOutputFile(char *outfl, char ext[3]);



#endif /* STATICFUNC_SRC_ICG_H_ */
