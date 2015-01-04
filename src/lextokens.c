/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  lextokens.c
 *  Lexical Token Data Structures
 *
 */

#include "lextokens.h"

// private functions
LexicalToken *pushToken(LexicalTokenList *lst);
LexicalToken *outputToken(LexicalToken *tok);


LexicalTokenList *createLexicalTokenList()
{
	LexicalTokenList *L = malloc(sizeof(LexicalTokenList));
	if(L == NULL){
		fatalError("Out of Memory [createLexicalTokenList]\n");
	}

	L->first = NULL;
	L->last = NULL;

	return L;
}

void freeLexicalTokenList(LexicalTokenList *lst)
{
	if(lst == NULL)
		return;
	if(lst->last == NULL){
		free(lst);
		return;
	}
	LexicalToken *tok = lst->last;
	while(tok != NULL){
		LexicalToken *tokOld = tok;
		tok = tokOld->prev;
		free(tokOld);
	}
	free(lst);
	return;
}


LexicalToken *pushToken(LexicalTokenList *lst)
{
	LexicalToken *tok = malloc(sizeof(LexicalToken));
	if(tok == NULL){
		fatalError("Out of Memory [pushToken]\n");
	}

	tok->extra = NULL;
	tok->next = NULL;
	tok->prev = NULL;
	tok->typ = LT_NULL;

	if(lst->first == NULL){
		lst->first = tok;
	}else{
		lst->last->next = tok;
		tok->prev = lst->last;
	}
	lst->last = tok;
	return tok;
}

void pushBasicToken(LexicalTokenList *lst, LexicalTokenType typ)
{
	LexicalToken *tok = pushToken(lst);
	tok->typ = typ;
}

LexicalToken *outputToken(LexicalToken *tok)
{
	if(tok->typ == LT_ADD){
		printf("+");
	}else if(tok->typ == LT_SUB){
		printf("-");
	}else{
		printf("UNKNOWN TOKEN [outputToken]!!!\n");
	}
}

void outputLexicalTokenList(LexicalTokenList *lst)
{
	LexicalToken *tok = lst->first;
	while(tok != NULL){
		outputToken(tok);
		printf("\n");
		tok = tok->next;
	}
}


