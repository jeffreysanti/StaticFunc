/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  parsetree.c
 *  Parsetree Data Structure
 *
 */

#include "parsetree.h"


PTree *newParseTreeNode()
{
	PTree *pTree = malloc(sizeof(PTree));
	if(pTree == NULL){
		fatalError("Out of Memory [createLexicalTokenList]\n");
	}
	pTree->child1 = NULL;
	pTree->child2 = NULL;
	pTree->parent = NULL;
	pTree->typ = PTT_NOTYPE;
	pTree->extra = NULL;
}


freeParseTreeNode(PTree *pTree)
{
	if(pTree == NULL)
		return;
	if(pTree->extra != NULL)
		free(pTree->extra);
	if(pTree->child1 != NULL){
		free(pTree->child1);
		pTree->child1 = NULL;
	}
	if(pTree->child2 != NULL){
		free(pTree->child2);
		pTree->child2 = NULL;
	}
	if(pTree->parent != NULL){
		if(((PTree*)pTree->parent)->child1 == pTree){
			((PTree*)pTree->parent)->child1 = NULL;
		}
		if(((PTree*)pTree->parent)->child2 == pTree){
			((PTree*)pTree->parent)->child2 = NULL;
		}
	}
	free(pTree);
}

PTree *newParseTree(PTType typ)
{
	PTree *pTree = newParseTreeNode();
	pTree->typ = typ;
	return pTree;
}

PTree *newChildNode(PTree *parent, PTType typ)
{
	PTree *pTree = newParseTree(typ);
	pTree->parent = parent;
	if(parent->child1 == NULL){
		parent->child1 = pTree;
	}else{
		parent->child2 = pTree;
	}
	return pTree;
}













