/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  parsetree.h
 *  Parsetree Data Structure
 *
 */

#ifndef STATICFUNC_SRC_PARSETREE_H_
#define STATICFUNC_SRC_PARSETREE_H_

#include <string.h>

typedef enum{
	PTT_NOTYPE,
	PTT_EXPR,
	PTT_ADD,
	PTT_MULT,
	PTT_DIV,
	PTT_MOD,
	PTT_EXP,
	PTT_INT
} PTType;

typedef struct{
	PTType typ;
	struct PTree *child1;
	struct PTree *child2;
	struct PTree *parent;
	void *extra;
} PTree;


PTree *newParseTree(PTType typ);
PTree *newChildNode(PTree *parent, PTType typ);
freeParseTreeNode(PTree *pTree);



#endif /* STATICFUNC_SRC_PARSETREE_H_ */
