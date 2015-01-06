/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  lex.c
 *  Lexical Analyzer
 *
 */

#include "lex.h"

// current state:
char errorStatus;
long cpos;
long clen;
char *fulltxt;
LexicalTokenList *TL;
long lnNo;

LexicalState processInit();
LexicalState processPrep();
LexicalState processLineComment();
LexicalState processStr(char c);


LexicalTokenList *lexicalAnalyze(FILE *fp)
{
	// read entire file into fulltxt
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fulltxt = malloc(fsize + 1);
	if(fulltxt == NULL){
		fatalError("Out of Memory [lexicalAnalyze : fulltxt]\n");
	}
	fread(fulltxt, fsize, 1, fp);
	fulltxt[fsize] = 0;

	TL = createLexicalTokenList();
	errorStatus = 0;
	cpos = 0;
	clen = strlen(fulltxt);
	lnNo = 1;

	LexicalState state = LT_INIT;

	while(cpos < clen){
		if(state == LT_INIT){
			state = processInit();
		}
		else if(state == LT_PREPROCESSOR){
			state = processPrep();
		}
		else if(state == LT_LINE_COMMENT){
			state = processLineComment();
		}
		else if(state == LT_STRING_DBL){
			state = processStr('"');
		}
		else if(state == LT_STRING_SINGLE){
			state = processStr('\'');
		}
	}

	free(fulltxt);
	if(errorStatus != 0){
		freeLexicalTokenList(TL);
		return NULL;
	}

	pushBasicToken(TL, LT_EOF, lnNo);

	return TL;
}


LexicalState processInit()
{
	long start = cpos;
	if(matchLineComment(fulltxt, &cpos, clen) == MATCHED){
		return LT_LINE_COMMENT;
	}else if(matchPreprocessor(fulltxt, &cpos, clen) == MATCHED){
		return LT_PREPROCESSOR;
	}else if(matchStringPattern(fulltxt, &cpos, clen, "\"") == MATCHED){
		return LT_STRING_DBL;
	}else if(matchStringPattern(fulltxt, &cpos, clen, "'") == MATCHED){
		return LT_STRING_SINGLE;
	}else if(matchFloatLiteral(fulltxt, &cpos, clen) == MATCHED){

		// TODO: Convert To Float of Arbitrary Percision
		pushStringToken(TL, LT_FLOAT, lnNo, extractString(fulltxt, start, cpos));

	}else if(matchIntegerLiteral(fulltxt, &cpos, clen) == MATCHED){

		// TODO: Convert To Integer of Arbitrary Size
		pushStringToken(TL, LT_INTEGER, lnNo, extractString(fulltxt, start, cpos));

	}else if(matchOperator(fulltxt, &cpos, clen) == MATCHED){
		pushStringToken(TL, LT_OP, lnNo, extractString(fulltxt, start, cpos));
	}else if(matchKeyword(fulltxt, &cpos, clen) == MATCHED){
		pushStringToken(TL, LT_KEYWORD, lnNo, extractString(fulltxt, start, cpos));
	}else if(matchIdentifier(fulltxt, &cpos, clen) == MATCHED){
		pushStringToken(TL, LT_IDENTIFIER, lnNo, extractString(fulltxt, start, cpos));
	}else if(matchStringPattern(fulltxt, &cpos, clen, "\n") == MATCHED){
		lnNo ++;
	}else if(matchWhiteSpace(fulltxt, &cpos, clen) == MATCHED){
		// nothing - discard
	}else{
		reportError("LX001", 	"Unrecognized character in main code section: Line %d\n"
								"\tCharacter is: %c\n"
								"\tCode: %d\n", lnNo, fulltxt[cpos], (int)fulltxt[cpos]);
		errorStatus ++;
		cpos ++;
	}

	return LT_INIT;
}

LexicalState processPrep()
{
	while(cpos < clen && fulltxt[cpos] != '\n'){
		cpos ++;
		// absorb preprocessor
	}
	// TODO: Parse This
	return LT_INIT;
}

LexicalState processLineComment()
{
	while(cpos < clen && fulltxt[cpos] != '\n'){
		cpos ++;
		// absorb comments
	}
	return LT_INIT;
}


LexicalState processStr(char c)
{
	char buf[MAX_LITERAL + 1];
	memset(buf, 0, (MAX_LITERAL+1)*sizeof(char));
	int bufpos = 0;

	while(cpos < clen && fulltxt[cpos] != c){
		if(bufpos >= MAX_LITERAL){
			reportError("LX002", 	"Excessively long string literal found: Line %d\n"
									"\tExceeded Max %d Characters\n", lnNo, (int)MAX_LITERAL);
			errorStatus ++;
			return LT_INIT;
		}

		if(fulltxt[cpos] == '\n'){
			reportError("LX003", 	"Illegal Newline inside quote: Line %d\n", lnNo);
			errorStatus ++;
			lnNo ++;
			cpos ++;
			continue;
		}
		if(fulltxt[cpos] == '\0'){
			reportError("LX004", 	"Illegal Null inside quote: Line %d\n", lnNo);
			errorStatus ++;
			cpos ++;
			continue;
		}

		buf[bufpos] = fulltxt[cpos];

		cpos ++;
		bufpos ++;

		if(cpos >= clen){
			reportError("LX005", 	"Quote Does Not End (EOF): Line %d\n", lnNo);
			errorStatus ++;
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
	pushStringToken(TL, LT_TEXT, lnNo, strToken);

	cpos ++;
	return LT_INIT;
}
