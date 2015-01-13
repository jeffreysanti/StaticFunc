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
	pTree->tok = NULL;
	return pTree;
}


void freeParseTreeNode(PTree *pTree)
{
	freeParseTreeNode_onlychildren(pTree);
	if(pTree != NULL)
		free(pTree);
}

void freeParseTreeNode_onlychildren(PTree *pTree)
{
	if(pTree == NULL)
		return;
	//if(pTree->extra != NULL) // managed by token list TODO
	//	free(pTree->extra);
	if(pTree->child1 != NULL){
		freeParseTreeNode((PTree*)pTree->child1);
		pTree->child1 = NULL;
	}
	if(pTree->child2 != NULL){
		freeParseTreeNode((PTree*)pTree->child2);
		pTree->child2 = NULL;
	}
	if(pTree->parent != NULL){
		if((PTree*)((PTree*)pTree->parent)->child1 == pTree){
			((PTree*)pTree->parent)->child1 = NULL;
		}
		if((PTree*)((PTree*)pTree->parent)->child2 == pTree){
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
	pTree->parent = (void*)parent;
	if(parent->child1 == NULL){
		parent->child1 = (void*)pTree;
	}else if(parent->child2 == NULL){
		parent->child2 = (void*)pTree;
	}else{
		fatalError("ERROR: Child Node Cannot Be Added\n");
	}
	return pTree;
}

/*void removeParentParseNodeLeaveLChild(PTree *root)
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
}*/
PTree *extractIndependentLeftParseNodeLeaveChild(PTree *root)
{
	PTree *lchild = (PTree*)root->child1;
	lchild->parent = NULL;
	root->child1 = NULL; // so it does not overwrite replacement
	return lchild;
}

void insertParseNodeFromList(PTree *root, PTType typ_cont, PTree *node)
{
	root->typ = typ_cont;
	if(root->child1 != NULL){
		if(root->child2 == NULL){
			root->child2 = (void*)newChildNode(root);
		}
		insertParseNodeFromList((PTree*)root->child2, typ_cont, node);
	}else{
		root->child1 = (void*)node;
		node->parent = (void*)root;
	}
}

// sets the lexical token of the second to last internal node of the tree
void setSecondLastParseNodeToken(PTree *root, LexicalToken *tok)
{
	if(root->child1 == NULL || root->child2 == NULL)
		return;
	while(root->child2 != NULL){
		root = (PTree*)root->child2;
	}
	root = (PTree*)root->parent;
	root->tok = tok;
}

void mergeEndParseNodes(PTree *root)
{
	if(root->child1 == NULL)
		return;
	while(root->child2 != NULL){
		root = (PTree*)root->child2;
	}
	// at bottom of tree
	PTree *child = root;
	root = (PTree*)root->parent;
	root->child2 = child->child1;
	((PTree*)root->child2)->parent = (void*)root;
	child->child1 = NULL;
	freeParseTreeNode(child);
}

void setParseNodeChild(PTree *parent, PTree *child, ParseChild side)
{
	if(child == NULL){
		return;
	}

	child->parent = (void*)parent;
	if(side == PC_LEFT && parent->child1 == NULL){
		parent->child1 = (void*)child;
	}else if(side == PC_RIGHT && parent->child2 == NULL){
		parent->child2 = (void*)child;
	}else{
		fatalError("ERROR: Parent Cannot Aquire Child Node\n");
	}
}

PTree *lastRightInternalParseNode(PTree *root)
{
	while(root->child2 != NULL){
		root = (PTree*)root->child2;
	}
	return root;
}

void dumpParseTree(PTree *root, int level)
{
	if(root == NULL)
		return;
	int i;
	for(i=0; i<level; i++){
		printf("  ");
	}
	/*if(root->typ == PTT_INT)
		printf("Val: %s\n", (char*)((LexicalToken*)root->tok)->extra);
	else
		printf("NODE %d\n", root->typ);
*/
	if(root->tok != NULL){
		outputToken((LexicalToken*)root->tok);
		printf("\n");
	}else{
		printf("? %d\n", root->typ);
	}

	dumpParseTree((PTree*)root->child1, level+1);
	dumpParseTree((PTree*)root->child2, level+1);
}














