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
#include "types.h"

typedef enum{
	PTT_NOTYPE = 0,
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
	PTT_FOR_COND,
	PTT_RETURN,
	PTT_FUNCTION,
	PTT_FUNCTION_TYPE,
	PTT_LAMBDA,

	PTT_DECL_TYPE_DEDUCED,
	PTT_ARRAY_ELM_PAIR,
	PTT_OBJECT_EQUAL_CHECK
} PTType;

typedef struct _PTree{
	PTType typ;
	struct PTree *child1;
	struct PTree *child2;
	struct PTree *parent;
	LexicalToken *tok;
	TypeDeductions deducedTypes;
	Type finalType;
} PTree;


PTree *newParseTree(PTType typ);
PTree *newChildNode(PTree *parent);
void freeParseTreeNode(PTree *pTree);
void freeParseTreeNode_onlychildren(PTree *pTree);

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


void dumpParseTree(PTree *root, int level, FILE *f);
void dumpParseTreeDet(PTree *root, int level, FILE *f);

void cleanUpEmptyStatments(PTree *ptr);

char *getParseNodeName(PTree *root);

void setTypeDeductions(PTree *root, TypeDeductions ded);
void setFinalTypeDeduction(PTree *root, Type typ);

#endif /* STATICFUNC_SRC_PARSETREE_H_ */
