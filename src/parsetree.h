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

#include "lextokens.h"

typedef enum{
	PTT_NOTYPE,
	PTT_EXPR,
	PTT_ADD,
	PTT_SUB,
	PTT_MULT,
	PTT_DIV,
	PTT_MOD,
	PTT_XOR,
	PTT_AND,
	PTT_OR,
	PTT_NOT,
	PTT_EXP,
	PTT_INT,
	PTT_STRING,
	PTT_FLOAT,
	PTT_EQUAL,
	PTT_NOT_EQUAL,
	PTT_GTE,
	PTT_GT,
	PTT_LTE,
	PTT_LT,
	PTT_SHR,
	PTT_SHL,
	PTT_IDENTIFIER,
	PTT_DOT,
	PTT_ARR_ACCESS,
	PTT_PARAM_CONT,
	PTT_ARRAY_ELM,
	PTT_ARRAY_COMP,
	PTT_ARRAY_COMP_IN,
	PTT_ARRAY_COMP_OUT,
	PTT_DECL_MOD,
	PTT_DECL_TYPE,
	PTT_DECL_PARAM,
	PTT_DECL,
	PTT_STMTBLOCK,
	PTT_ASSIGN,
	PTT_IF,
	PTT_IFELSE_SWITCH,
	PTT_FOR,
	PTT_WHILE,
	PTT_FOR_COND
} PTType;

typedef struct{
	PTType typ;
	struct PTree *child1;
	struct PTree *child2;
	struct PTree *parent;
	LexicalToken *tok;
} PTree;


PTree *newParseTree(PTType typ);
PTree *newChildNode(PTree *parent);
freeParseTreeNode(PTree *pTree);
freeParseTreeNode_onlychildren(PTree *pTree);

PTree *extractIndependentLeftParseNodeLeaveChild(PTree *root);

void insertParseNodeFromList(PTree *root, PTType typ_cont, PTree *node);
void setSecondLastParseNodeToken(PTree *root, LexicalToken *tok);
void mergeEndParseNodes(PTree *root);

PTree *lastRightInternalParseNode(PTree *root);

typedef enum {
	PC_LEFT,
	PC_RIGHT
} ParseChild;

void setParseNodeChild(PTree *parent, PTree *child, ParseChild side);


void dumpParseTree(PTree *root, int level);


#endif /* STATICFUNC_SRC_PARSETREE_H_ */
