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


typedef enum {
	LT_INIT,
	LT_PREPROCESSOR,
	LT_LINE_COMMENT,
	LT_STRING_DBL,
	LT_STRING_SINGLE
} LexicalState;

#define MAX_LITERAL 1023


#define MATCHED 1
#define UNMATCHED 0

inline char matchStringPattern(char *fulltxt, long *cpos, long clen, char *pattern)
{
	if(*cpos + strlen(pattern) > clen)
		return UNMATCHED;
	int i;
	int oldCpos = *cpos;
	for(i=0; i<strlen(pattern); i++){
		if(tolower(fulltxt[*cpos]) != tolower(pattern[i])){
			*cpos = oldCpos;
			return UNMATCHED;
		}
		(*cpos) ++;
	}
	return MATCHED;
}


inline char matchLineComment(char *fulltxt, long *cpos, long clen) // "//"
{
	return matchStringPattern(fulltxt, cpos, clen, "//");
}

inline char matchPreprocessor(char *fulltxt, long *cpos, long clen) // "#"
{
	if(fulltxt[*cpos] != '#')
		return UNMATCHED;
	(*cpos) ++;
	return MATCHED;
}

inline char isNumerich(char c, char hexMatch){
	if(c >= '0' && c <= '9'){
		return MATCHED;
	}
	if(hexMatch == MATCHED && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))){
		return MATCHED;
	}
	return UNMATCHED;
}

inline char isNumeric(char c){
	isNumerich(c, UNMATCHED);
}

inline char isAlpha(char c){
	if(		(c >= 'A' && c <= 'Z') ||
			(c >= 'a' && c <= 'z') ||
			c == '_') {
		return MATCHED;
	}
	return UNMATCHED;
}

inline char matchIdentifier(char *fulltxt, long *cpos, long clen)
{
	// first character
	if(isAlpha(fulltxt[*cpos]) == UNMATCHED)
		return UNMATCHED;
	(*cpos) ++;
	while(*cpos < clen && (isAlpha(fulltxt[*cpos]) == MATCHED || isNumeric(fulltxt[*cpos]) == MATCHED)) {
		(*cpos) ++;
	}
	return MATCHED;
}

inline char matchIntegerLiteral(char *fulltxt, long *cpos, long clen)
{
	long cposOrig = *cpos;

	// match possible preceeding negative and/or 0x
	matchStringPattern(fulltxt, cpos, clen, "-");
	char hex = matchStringPattern(fulltxt, cpos, clen, "0x");

	// now match at least one digit
	if(isNumerich(fulltxt[*cpos], hex) == UNMATCHED){
		*cpos = cposOrig;
		return UNMATCHED;
	}
	(*cpos) ++;
	while(*cpos < clen && isNumerich(fulltxt[*cpos], hex) == MATCHED) {
		(*cpos) ++;
	}
	return MATCHED;
}

inline char matchFloatLiteral(char *fulltxt, long *cpos, long clen)
{
	long cposOrig = *cpos;

	// match possible preceeding negative
	matchStringPattern(fulltxt, cpos, clen, "-");
	char matchedDeci = matchStringPattern(fulltxt, cpos, clen, ".");

	// now match at least one digit
	if(isNumeric(fulltxt[*cpos]) == UNMATCHED){
		*cpos = cposOrig;
		return UNMATCHED;
	}
	(*cpos) ++;
	while(*cpos < clen && isNumeric(fulltxt[*cpos]) == MATCHED) {
		(*cpos) ++;
	}

	if(matchedDeci == UNMATCHED){
		matchedDeci = matchStringPattern(fulltxt, cpos, clen, ".");
		if(matchedDeci){
			while(*cpos < clen && isNumeric(fulltxt[*cpos]) == MATCHED) {
				(*cpos) ++;
			}
		}
	}

	// floats MUST have a decimal point to distinguish from integer
	if(!matchedDeci){
		*cpos = cposOrig;
		return UNMATCHED;
	}
	return MATCHED;
}



inline char matchOperator(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, "||") ||
			matchStringPattern(fulltxt, cpos, clen, "&&") ||
			matchStringPattern(fulltxt, cpos, clen, "==") ||
			matchStringPattern(fulltxt, cpos, clen, "!=") ||
			matchStringPattern(fulltxt, cpos, clen, ">=") ||
			matchStringPattern(fulltxt, cpos, clen, "<=") ||
			matchStringPattern(fulltxt, cpos, clen, "+") ||
			matchStringPattern(fulltxt, cpos, clen, "-") ||
			matchStringPattern(fulltxt, cpos, clen, "*") ||
			matchStringPattern(fulltxt, cpos, clen, "/") ||
			matchStringPattern(fulltxt, cpos, clen, "%") ||
			matchStringPattern(fulltxt, cpos, clen, "&") ||
			matchStringPattern(fulltxt, cpos, clen, "|") ||
			matchStringPattern(fulltxt, cpos, clen, "~") ||
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
		return MATCHED;
	return UNMATCHED;
}

inline char matchInlineTypes(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, "int64") ||
			matchStringPattern(fulltxt, cpos, clen, "int32") ||
			matchStringPattern(fulltxt, cpos, clen, "int16") ||
			matchStringPattern(fulltxt, cpos, clen, "int8") ||
			matchStringPattern(fulltxt, cpos, clen, "vector") ||
			matchStringPattern(fulltxt, cpos, clen, "string"))
		return MATCHED;
	return UNMATCHED;
}

inline char matchWhiteSpace(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, " ") ||
			matchStringPattern(fulltxt, cpos, clen, "\f") ||
			matchStringPattern(fulltxt, cpos, clen, "\r") ||
			matchStringPattern(fulltxt, cpos, clen, "\t") ||
			matchStringPattern(fulltxt, cpos, clen, "\v"))
		return MATCHED;
	return UNMATCHED;
}

inline char matchKeyword(char *fulltxt, long *cpos, long clen)
{
	if(		matchStringPattern(fulltxt, cpos, clen, "mut") ||
			matchStringPattern(fulltxt, cpos, clen, "return") ||
			matchStringPattern(fulltxt, cpos, clen, "where") ||
			matchStringPattern(fulltxt, cpos, clen, "in") ||
			matchStringPattern(fulltxt, cpos, clen, "for") ||
			matchStringPattern(fulltxt, cpos, clen, "if") ||
			matchStringPattern(fulltxt, cpos, clen, "int64") ||
			matchStringPattern(fulltxt, cpos, clen, "int32") ||
			matchStringPattern(fulltxt, cpos, clen, "int16") ||
			matchStringPattern(fulltxt, cpos, clen, "int8") ||
			matchStringPattern(fulltxt, cpos, clen, "vector") ||
			matchStringPattern(fulltxt, cpos, clen, "string")){

		// next character must be non-alpha-numeric
		if(isAlpha(fulltxt[*cpos]) || isNumeric(fulltxt[*cpos]))
			return UNMATCHED;

		return MATCHED;
	}
	return UNMATCHED;
}


inline char *extractString(char *fulltxt, long start, long end){
	char *str = malloc((1+ end-start)*sizeof(char));
	if(str == NULL){
		fatalError("Out of Memory [extractString]\n");
	}
	strncpy(str, fulltxt+(start*sizeof(char)), end-start);
	str[end-start] = '\0';
	return str;
}


#endif /* STATICFUNC_SRC_LEX_H_ */
