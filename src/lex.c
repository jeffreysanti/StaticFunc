/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  lex.c
 *  Lexical Analyzer
 *
 */

#include "lextokens.h"

LexicalTokenList *lexicalAnalyze(FILE *fp)
{
	LexicalTokenList *TL = createLexicalTokenList();
	char errorStatus = 0;

	while(!feof(fp)){
		char ch;
		fread(&ch, sizeof(char), 1, fp);
		printf("%c", ch);
	}




	if(errorStatus != 0){
		freeLexicalTokenList(TL);
		return NULL;
	}
	return TL;
}

