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
void outputToken(LexicalToken *tok);


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
		tok = (LexicalToken *)tokOld->prev;
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
		lst->last->next = (void*)tok;
		tok->prev = (void*)lst->last;
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

void outputToken(LexicalToken *tok)
{
	if(tok->typ == LT_TEXT){
		printf("[%ld] LT_TEXT: %s", tok->lineNo, (char*)tok->extra);
	}else if(tok->typ == LT_KEYWORD){
		printf("[%ld] LT_KEYWORD: %s", tok->lineNo, (char*)tok->extra);
	}else if(tok->typ == LT_OP){
		printf("[%ld] LT_OP: %s", tok->lineNo, (char*)tok->extra);
	}else if(tok->typ == LT_IDENTIFIER){
		printf("[%ld] LT_IDENTIFIER: %s", tok->lineNo, (char*)tok->extra);
	}else if(tok->typ == LT_INTEGER){
		printf("[%ld] LT_INTEGER: %s", tok->lineNo, (char*)tok->extra);
	}else if(tok->typ == LT_FLOAT){
		printf("[%ld] LT_FLOAT: %s", tok->lineNo, (char*)tok->extra);
	}else if(tok->typ == LT_EOF){
		printf("[%ld] LT_EOF", tok->lineNo);
	}else{
		printf("[%ld] UNKNOWN TOKEN [outputToken]!!!", tok->lineNo);
	}
}

void outputLexicalTokenList(LexicalTokenList *lst)
{
	LexicalToken *tok = lst->first;
	while(tok != NULL){
		outputToken(tok);
		printf("\n");
		tok = (LexicalToken*)tok->next;
	}
}

// NOTE: Does not copy "extra" data
LexicalToken *duplicateAndPlaceAfterToken(LexicalToken *orig)
{
	LexicalToken *tok = malloc(sizeof(LexicalToken));
	if(tok == NULL){
		fatalError("Out of Memory [duplicateAndPlaceToken]\n");
	}
	tok->extra = NULL;
	tok->typ = orig->typ;
	tok->lineNo = orig->lineNo;
	tok->prev = (void*)orig;
	tok->next = (void*)orig->next;

	((LexicalToken*)tok->next)->prev = (void*)tok;
	orig->next = (void*)tok;
	return tok;
}


