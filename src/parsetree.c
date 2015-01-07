/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  parsetree.c
 *  Parsetree Data Structure
 *
 */

#include "parsetree.h"
#include "lextokens.h"


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
	freeParseTreeNode_onlychildren(pTree);
	if(pTree != NULL)
		free(pTree);
}

freeParseTreeNode_onlychildren(PTree *pTree)
{
	if(pTree == NULL)
		return;
	//if(pTree->extra != NULL) // managed by token list TODO
	//	free(pTree->extra);
	if(pTree->child1 != NULL){
		freeParseTreeNode(pTree->child1);
		pTree->child1 = NULL;
	}
	if(pTree->child2 != NULL){
		freeParseTreeNode(pTree->child2);
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
}

PTree *newParseTree(PTType typ)
{
	PTree *pTree = newParseTreeNode();
	pTree->typ = typ;
	return pTree;
}

PTree *newChildNode(PTree *parent)
{
	PTree *pTree = newParseTree(PTT_NOTYPE);
	pTree->parent = parent;
	if(parent->child1 == NULL){
		parent->child1 = pTree;
	}else if(parent->child2 == NULL){
		parent->child2 = pTree;
	}else{
		printf("ERROR: Child Node Cannot Be Added\n");
	}
	return pTree;
}

void removeParentParseNodeLeaveLChild(PTree *root)
{
	PTree *lchild = root->child1;
	lchild->parent = NULL;
	root->child1 = NULL; // so it does not overwrite replacement

	// free other possible child
	freeParseTreeNode_onlychildren(root);

	// now overwrite root memory
	memcpy(root, lchild, sizeof(PTree));

	// free space associated originally with child
	free(lchild);
}

void insertParseNodeFromList(PTree *root, PTType typ_cont, PTree *node)
{
	root->typ = typ_cont;
	if(root->child1 != NULL){
		if(root->child2 == NULL){
			root->child2 = newChildNode(root);
		}
		insertParseNodeFromList(root->child2, typ_cont, node);
	}else{
		root->child1 = node;
	}
}

void mergeEndParseNodes(PTree *root)
{
	if(root->child1 == NULL)
		return;
	while(root->child2 != NULL){
		root = root->child2;
	}
	// at bottom of tree
	PTree *child = root;
	root = root->parent;
	root->child2 = child->child1;
	((PTree*)root->child2)->parent = root;
	child->child1 = NULL;
	freeParseTreeNode(child);
}

void dumpParseTree(PTree *root, int level)
{
	if(root == NULL)
		return;
	int i;
	for(i=0; i<level; i++){
		printf("  ");
	}
	if(root->typ == PTT_INT)
		printf("Val: %s\n", (char*)((LexicalToken*)root->extra)->extra);
	else
		printf("NODE %d\n", root->typ);

	dumpParseTree(root->child1, level+1);
	dumpParseTree(root->child2, level+1);
}














