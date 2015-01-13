/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  lex.c
 *  Lexical Analyzer
 *
 */

#include "lex.h"
#include "prep.h"


LexicalState processInit(LS *ls);
LexicalState processPrep(LS *ls);
LexicalState processLineComment(LS *ls);
LexicalState processStr(LS *ls, char c);


LexicalTokenList *lexicalAnalyze(FILE *fp)
{
	// read entire file into fulltxt
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *fulltxt = malloc(fsize + 1);
	if(fulltxt == NULL){
		fatalError("Out of Memory [lexicalAnalyze : fulltxt]\n");
	}
	fread(fulltxt, fsize, 1, fp);
	fulltxt[fsize] = 0;

	LexicalTokenList * ret = lexicalAnalyzeString(fulltxt, fsize);

	free(fulltxt);
	return ret;
}

LexicalTokenList *lexicalAnalyzeString(char *s, int len){
	LS ls;
	ls.TL = createLexicalTokenList();
	ls.errorStatus = 0;
	ls.cpos = 0;
	ls.clen = len;
	ls.lnNo = 1;
	ls.fulltxt = s;
	ls.fulltxt[len] = 0;


	LexicalState state = LT_INIT;

	while(ls.cpos < ls.clen){
		if(state == LT_INIT){
			state = processInit(&ls);
		}
		else if(state == LT_PREPROCESSOR){
			state = processPrep(&ls);
		}
		else if(state == LT_LINE_COMMENT){
			state = processLineComment(&ls);
		}
		else if(state == LT_STRING_DBL){
			state = processStr(&ls, '"');
		}
		else if(state == LT_STRING_SINGLE){
			state = processStr(&ls, '\'');
		}
	}
	if(ls.errorStatus != 0){
		freeLexicalTokenList(ls.TL);
		return NULL;
	}

	pushBasicToken(ls.TL, LT_EOF, ls.lnNo);

	return ls.TL;
}


LexicalState processInit(LS *ls)
{
	long start = ls->cpos;
	if(matchLineComment(ls->fulltxt, &ls->cpos, ls->clen)){
		return LT_LINE_COMMENT;
	}else if(matchPreprocessor(ls->fulltxt, &ls->cpos, ls->clen)){
		return LT_PREPROCESSOR;
	}else if(matchStringPattern(ls->fulltxt, &ls->cpos, ls->clen, "\"")){
		return LT_STRING_DBL;
	}else if(matchStringPattern(ls->fulltxt, &ls->cpos, ls->clen, "'")){
		return LT_STRING_SINGLE;
	}else if(matchFloatLiteral(ls->fulltxt, &ls->cpos, ls->clen)){

		// TODO: Convert To Float of Arbitrary Percision
		pushStringToken(ls->TL, LT_FLOAT, ls->lnNo, extractString(ls->fulltxt, start, ls->cpos));

	}else if(matchIntegerLiteral(ls->fulltxt, &ls->cpos, ls->clen)){

		// TODO: Convert To Integer of Arbitrary Size
		pushStringToken(ls->TL, LT_INTEGER, ls->lnNo, extractString(ls->fulltxt, start, ls->cpos));

	}else if(matchOperator(ls->fulltxt, &ls->cpos, ls->clen)){
		pushStringToken(ls->TL, LT_OP, ls->lnNo, extractString(ls->fulltxt, start, ls->cpos));
	}else if(matchKeyword(ls->fulltxt, &ls->cpos, ls->clen)){
		pushStringToken(ls->TL, LT_KEYWORD, ls->lnNo, extractString(ls->fulltxt, start, ls->cpos));
	}else if(matchIdentifier(ls->fulltxt, &ls->cpos, ls->clen)){
		pushStringToken(ls->TL, LT_IDENTIFIER, ls->lnNo, extractString(ls->fulltxt, start, ls->cpos));
	}else if(matchStringPattern(ls->fulltxt, &ls->cpos, ls->clen, "\n")){
		ls->lnNo ++;
	}else if(matchWhiteSpace(ls->fulltxt, &ls->cpos, ls->clen)){
		// nothing - discard
	}else{
		reportError("LX001", 	"Unrecognized character in main code section: Line %d\n"
								"\tCharacter is: %c\n"
								"\tCode: %d\n", ls->lnNo, ls->fulltxt[ls->cpos], (int)ls->fulltxt[ls->cpos]);
		ls->errorStatus ++;
		ls->cpos ++;
	}

	return LT_INIT;
}

LexicalState processLineComment(LS *ls)
{
	while(ls->cpos < ls->clen && ls->fulltxt[ls->cpos] != '\n'){
		ls->cpos ++;
		// absorb comments
	}
	return LT_INIT;
}

LexicalState processPrep(LS *ls)
{
	char buf[MAX_LITERAL + 1];
	memset(buf, 0, (MAX_LITERAL+1)*sizeof(char));
	int bufpos = 0;
	while(ls->cpos < ls->clen && ls->fulltxt[ls->cpos] != '\n'){

		if(bufpos >= MAX_LITERAL){
			reportError("LX006", 	"Excessively long preprocessor found: Line %d\n"
									"\tExceeded Max %d Characters\n", ls->lnNo, (int)MAX_LITERAL);
			ls->errorStatus ++;
			return LT_INIT;
		}

		buf[bufpos] = ls->fulltxt[ls->cpos];
		ls->cpos ++;
		bufpos ++;
	}
	if(parsePreprocessor(buf, bufpos) == 0){
		reportError("LX007", 	"Invalid preprocessor found: Line %d\n", ls->lnNo);
		ls->errorStatus ++;
		return LT_INIT;
	}

	ls->cpos ++;
	return LT_INIT;
}


LexicalState processStr(LS *ls, char c)
{
	char buf[MAX_LITERAL + 1];
	memset(buf, 0, (MAX_LITERAL+1)*sizeof(char));
	int bufpos = 0;

	while(ls->cpos < ls->clen && ls->fulltxt[ls->cpos] != c){
		if(bufpos >= MAX_LITERAL){
			reportError("LX002", 	"Excessively long string literal found: Line %d\n"
									"\tExceeded Max %d Characters\n", ls->lnNo, (int)MAX_LITERAL);
			ls->errorStatus ++;
			return LT_INIT;
		}

		if(ls->fulltxt[ls->cpos] == '\n'){
			reportError("LX003", 	"Illegal Newline inside quote: Line %d\n", ls->lnNo);
			ls->errorStatus ++;
			ls->lnNo ++;
			ls->cpos ++;
			continue;
		}
		if(ls->fulltxt[ls->cpos] == '\0'){
			reportError("LX004", 	"Illegal Null inside quote: Line %d\n", ls->lnNo);
			ls->errorStatus ++;
			ls->cpos ++;
			continue;
		}

		buf[bufpos] = ls->fulltxt[ls->cpos];

		ls->cpos ++;
		bufpos ++;

		if(ls->cpos >= ls->clen){
			reportError("LX005", 	"Quote Does Not End (EOF): Line %d\n", ls->lnNo);
			ls->errorStatus ++;
			break;
		}
	}

	// push literal to token list
	char *strToken = malloc(sizeof(char) * (bufpos+1));
	if(strToken == NULL){
		fatalError("Out of Memory [processStr]\n");
	}
	strncpy(strToken, buf, bufpos);
	strToken[bufpos] = '\0';
	pushStringToken(ls->TL, LT_TEXT, ls->lnNo, strToken);

	ls->cpos ++;
	return LT_INIT;
}
