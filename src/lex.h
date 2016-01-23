/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  lex.h
 *  Lexical Analyzer Defn.
 *
 */



#ifndef STATICFUNC_SRC_LEX_H_
#define STATICFUNC_SRC_LEX_H_

#include "lextokens.h"

LexicalTokenList *lexicalAnalyze(FILE *fp);
LexicalTokenList *lexicalAnalyzeString(char *s, int len);


typedef enum {
	LT_INIT,
	LT_PREPROCESSOR,
	LT_LINE_COMMENT,
	LT_STRING_DBL,
	LT_STRING_SINGLE
} LexicalState;

typedef struct{
	char errorStatus;
	long cpos;
	long clen;
	char *fulltxt;
	LexicalTokenList *TL;
	long lnNo;
} LS;

#define MAX_LITERAL 1023

static bool matchStringPattern(char *fulltxt, long *cpos, long clen, char *pattern)
{
	if(*cpos + strlen(pattern) > clen)
		return false;
	int i;
	int oldCpos = *cpos;
	for(i=0; i<strlen(pattern); i++){
		if(tolower(fulltxt[*cpos]) != tolower(pattern[i])){
			*cpos = oldCpos;
			return false;
		}
		(*cpos) ++;
	}
	return true;
}


static bool matchLineComment(char *fulltxt, long *cpos, long clen) // "//"
{
	return matchStringPattern(fulltxt, cpos, clen, "//");
}

static bool matchPreprocessor(char *fulltxt, long *cpos, long clen) // "#"
{
	if(fulltxt[*cpos] != '#')
		return false;
	(*cpos) ++;
	return true;
}

static bool isNumerich(char c, char hexMatch){
	if(c >= '0' && c <= '9'){
		return true;
	}
	if(hexMatch == true && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))){
		return true;
	}
	return false;
}

static bool isNumeric(char c){
	return isNumerich(c, false);
}

static char isAlpha(char c){
	if(		(c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			c == '_') {
		return true;
	}
	return false;
}

static char matchIdentifier(char *fulltxt, long *cpos, long clen)
{
	// first character
	if(isAlpha(fulltxt[*cpos]) == false)
		return false;
	(*cpos) ++;
	while(*cpos < clen && (isAlpha(fulltxt[*cpos]) == true || isNumeric(fulltxt[*cpos]) == true)) {
		(*cpos) ++;
	}
	return true;
}

static char matchIntegerLiteral(char *fulltxt, long *cpos, long clen)
{
	long cposOrig = *cpos;

	// match possible preceeding negative and/or 0x
	if(matchStringPattern(fulltxt, cpos, clen, "-") == true){
		// make sure - not operator
		// TODO
	}
	char hex = matchStringPattern(fulltxt, cpos, clen, "0x");

	// now match at least one digit
	if(isNumerich(fulltxt[*cpos], hex) == false){
		*cpos = cposOrig;
		return false;
	}
	(*cpos) ++;
	while(*cpos < clen && isNumerich(fulltxt[*cpos], hex) == true) {
		(*cpos) ++;
	}
	return true;
}

static bool matchFloatLiteral(char *fulltxt, long *cpos, long clen)
{
	long cposOrig = *cpos;

	// match possible preceeding negative
	matchStringPattern(fulltxt, cpos, clen, "-");
	char trueDeci = matchStringPattern(fulltxt, cpos, clen, ".");

	// now match at least one digit
	if(isNumeric(fulltxt[*cpos]) == false){
		*cpos = cposOrig;
		return false;
	}
	(*cpos) ++;
	while(*cpos < clen && isNumeric(fulltxt[*cpos]) == true) {
		(*cpos) ++;
	}

	if(trueDeci == false){
		trueDeci = matchStringPattern(fulltxt, cpos, clen, ".");
		if(trueDeci){
			while(*cpos < clen && isNumeric(fulltxt[*cpos]) == true) {
				(*cpos) ++;
			}
		}
	}

	// floats MUST have a decimal point to distinguish from integer
	if(!trueDeci){
		*cpos = cposOrig;
		return false;
	}
	return true;
}



static bool matchOperator(char *fulltxt, long *cpos, long clen)
{
	if(
			matchStringPattern(fulltxt, cpos, clen, "==") ||
			matchStringPattern(fulltxt, cpos, clen, "!=") ||
			matchStringPattern(fulltxt, cpos, clen, ">=") ||
			matchStringPattern(fulltxt, cpos, clen, "<=") ||
			matchStringPattern(fulltxt, cpos, clen, "+") ||
			matchStringPattern(fulltxt, cpos, clen, "-") ||
			matchStringPattern(fulltxt, cpos, clen, "*") ||
			matchStringPattern(fulltxt, cpos, clen, "/") ||
			matchStringPattern(fulltxt, cpos, clen, "^") ||
			matchStringPattern(fulltxt, cpos, clen, "%") ||
			matchStringPattern(fulltxt, cpos, clen, "&") ||
			matchStringPattern(fulltxt, cpos, clen, "|") ||
			matchStringPattern(fulltxt, cpos, clen, "~") ||
			matchStringPattern(fulltxt, cpos, clen, "!") ||
			matchStringPattern(fulltxt, cpos, clen, "shr") ||
			matchStringPattern(fulltxt, cpos, clen, "shl") ||
			matchStringPattern(fulltxt, cpos, clen, ",") ||
			matchStringPattern(fulltxt, cpos, clen, ";") ||
			matchStringPattern(fulltxt, cpos, clen, ":") ||
			matchStringPattern(fulltxt, cpos, clen, "(") ||
			matchStringPattern(fulltxt, cpos, clen, ")") ||
			matchStringPattern(fulltxt, cpos, clen, "{") ||
			matchStringPattern(fulltxt, cpos, clen, "}") ||
			matchStringPattern(fulltxt, cpos, clen, "[") ||
			matchStringPattern(fulltxt, cpos, clen, "]") ||
			matchStringPattern(fulltxt, cpos, clen, ".") ||
			matchStringPattern(fulltxt, cpos, clen, "<") ||
			matchStringPattern(fulltxt, cpos, clen, ">") ||
			matchStringPattern(fulltxt, cpos, clen, "="))
		return true;
	return false;
}

static bool matchInlineTypes(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, "int") ||
			matchStringPattern(fulltxt, cpos, clen, "vector") ||
			matchStringPattern(fulltxt, cpos, clen, "string"))
		return true;
	return false;
}

static bool matchWhiteSpace(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, " ") ||
			matchStringPattern(fulltxt, cpos, clen, "\f") ||
			matchStringPattern(fulltxt, cpos, clen, "\r") ||
			matchStringPattern(fulltxt, cpos, clen, "\t") ||
			matchStringPattern(fulltxt, cpos, clen, "\v"))
		return true;
	return false;
}

static bool matchKeyword(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, "mut") ||
			matchStringPattern(fulltxt, cpos, clen, "def") ||
			matchStringPattern(fulltxt, cpos, clen, "lambda") ||
			matchStringPattern(fulltxt, cpos, clen, "return") ||
			matchStringPattern(fulltxt, cpos, clen, "where") ||
			matchStringPattern(fulltxt, cpos, clen, "in") ||
			matchStringPattern(fulltxt, cpos, clen, "for") ||
			matchStringPattern(fulltxt, cpos, clen, "while") ||
			matchStringPattern(fulltxt, cpos, clen, "if") ||
			matchStringPattern(fulltxt, cpos, clen, "else") ||
			matchStringPattern(fulltxt, cpos, clen, "size") ||
			matchStringPattern(fulltxt, cpos, clen, "queue") ||
			matchStringPattern(fulltxt, cpos, clen, "push") ||
			matchStringPattern(fulltxt, cpos, clen, "pop") ||
			matchStringPattern(fulltxt, cpos, clen, "dequeue")){

		// next character must be non-alpha-numeric
		if(isAlpha(fulltxt[*cpos]) || isNumeric(fulltxt[*cpos]))
			return false;

		return true;
	}
	return false;
}


static bool *extractString(char *fulltxt, long start, long end){
	char *str = malloc((1+ end-start)*sizeof(char));
	if(str == NULL){
		fatalError("Out of Memory [extractString]\n");
	}
	strncpy(str, fulltxt+(start*sizeof(char)), end-start);
	str[end-start] = '\0';
	return str;
}


#endif /* STATICFUNC_SRC_LEX_H_ */
