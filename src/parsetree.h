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
	PTT_SUB,
	PTT_MULT,
	PTT_DIV,
	PTT_MOD,
	PTT_EXP,
	PTT_INT,
	PTT_FLOAT,
	PTT_OR
} PTType;

typedef struct{
	PTType typ;
	struct PTree *child1;
	struct PTree *child2;
	struct PTree *parent;
	void *extra;
} PTree;


PTree *newParseTree(PTType typ);
PTree *newChildNode(PTree *parent);
freeParseTreeNode(PTree *pTree);
freeParseTreeNode_onlychildren(PTree *pTree);

void removeParentParseNodeLeaveLChild(PTree *root);

void insertParseNodeFromList(PTree *root, PTType typ_cont, PTree *node);
void mergeEndParseNodes(PTree *root);


void dumpParseTree(PTree *root, int level);


#endif /* STATICFUNC_SRC_PARSETREE_H_ */
