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
		if(tokOld->extra != NULL){
			free(tokOld->extra);
		}
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
	tok->lineNo = 0;

	if(lst->first == NULL){
		lst->first = tok;
	}else{
		lst->last->next = tok;
		tok->prev = lst->last;
	}
	lst->last = tok;
	return tok;
}

void pushBasicToken(LexicalTokenList *lst, LexicalTokenType typ, long ln)
{
	LexicalToken *tok = pushToken(lst);
	tok->typ = typ;
	tok->lineNo = ln;
}

void pushStringToken(LexicalTokenList *lst, LexicalTokenType typ, long ln, char *dta)
{
	LexicalToken *tok = pushToken(lst);
	tok->typ = typ;
	tok->lineNo = ln;
	tok->extra = dta;
}

LexicalToken *outputToken(LexicalToken *tok)
{
	if(tok->typ == LT_TEXT){
		printf("[%d] LT_TEXT: %s", tok->lineNo, tok->extra);
	}else if(tok->typ == LT_KEYWORD){
		printf("[%d] LT_KEYWORD: %s", tok->lineNo, tok->extra);
	}else if(tok->typ == LT_OP){
		printf("[%d] LT_OP: %s", tok->lineNo, tok->extra);
	}else if(tok->typ == LT_TYPE){
		printf("[%d] LT_TYPE: %s", tok->lineNo, tok->extra);
	}else if(tok->typ == LT_IDENTIFIER){
		printf("[%d] LT_IDENTIFIER: %s", tok->lineNo, tok->extra);
	}else if(tok->typ == LT_INTEGER){
		printf("[%d] LT_INTEGER: %s", tok->lineNo, tok->extra);
	}else if(tok->typ == LT_FLOAT){
		printf("[%d] LT_FLOAT: %s", tok->lineNo, tok->extra);
	}else{
		printf("[%d] UNKNOWN TOKEN [outputToken]!!!");
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


