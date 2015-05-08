/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  lextokens.h
 *  Lexical Token Data Structures
 *
 */

#ifndef STATICFUNC_SRC_LEXTOKENS_H_
#define STATICFUNC_SRC_LEXTOKENS_H_

#include <string.h>
#include <ctype.h>
#include "io.h"

typedef enum {
	LT_NULL,
	LT_OP,
	LT_KEYWORD,
	LT_TEXT,
	LT_INTEGER,
	LT_FLOAT,
	LT_IDENTIFIER,

	LT_EOF

} LexicalTokenType;


// Lexical Token Element (Linked List)
typedef struct{
	LexicalTokenType typ;
	long lineNo;
	void *extra;

	struct LexicalToken *prev;
	struct LexicalToken *next;

} LexicalToken;

// Lexical Token List
typedef struct{
	LexicalToken *first;
	LexicalToken *last;
} LexicalTokenList;


LexicalTokenList *createLexicalTokenList();
void freeLexicalTokenList(LexicalTokenList *lst);
void pushBasicToken(LexicalTokenList *lst, LexicalTokenType typ, long ln);
void pushStringToken(LexicalTokenList *lst, LexicalTokenType typ, long ln, char* dta);

void outputLexicalTokenList(LexicalTokenList *lst);
void outputToken(LexicalToken *tok);

LexicalToken *duplicateAndPlaceAfterToken(LexicalToken *orig);

#endif /* STATICFUNC_SRC_LEXTOKENS_H_ */
