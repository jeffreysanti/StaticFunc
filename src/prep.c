/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  prep.c
 *  Preprocessor processor
 *
 */

#include "prep.h"
#include "types.h"


char parsePreprocessor(char *str, int len)
{
	if(len > 7 && strncmp(str, "import ", 7) == 0){
		return 1; // TODO: IMPORT
	}else if (len > 8 && strncmp(str, "typedef ", 8) == 0){
		LexicalTokenList *T = lexicalAnalyzeString((char*)str+8, len-8);
		if(T == NULL){
			reportError("PP001", 	"#typedef Failed to Tokenize");
			return false;
		}
		PTree * P = parseTypeDef(T);
		if(P == NULL){
			reportError("PP002", 	"#typedef Failed to Parse");
			return false;
		}
		//LexicalToken *tokNm = T->first;
		deduceType(P);
		freeLexicalTokenList(T);
		freeParseTreeNode(P);
		return 1;
	}else if (len > 12 && strncmp(str, "typelistdef ", 12) == 0){
		return 1;
	}else if (len > 12 && strncmp(str, "typelistadd ", 12) == 0){
		return 1;
	}
	return 0;
}

