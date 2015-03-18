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


void errParse(){
	reportError("PP002", 	"#typedef Failed to Parse");
}
void errLex(){
	reportError("PP001", 	"#typedef Failed to Tokenize");
}
void errTypeDed(){
	reportError("PP003", 	"Type Deduction Failed");
}

bool parsePreprocessor(char *str, int len)
{
	if(len > 7 && strncmp(str, "import ", 7) == 0){
		return 1; // TODO: IMPORT
	}else if (len > 8 && strncmp(str, "typedef ", 8) == 0){
		LexicalTokenList *T = lexicalAnalyzeString((char*)str+8, len-8);
		if(T == NULL){
			errLex();
			return false;
		}
		PTree * P = parseTypeDef(T);
		if(P == NULL){
			freeLexicalTokenList(T);
			errParse();
			return false;
		}
		LexicalToken *tokNm = T->first;
		Type typ = deduceTypeDeclType(P);
		if(typ.base == TB_ERROR || typ.base == TB_TYPELIST){
			freeLexicalTokenList(T);
			freeParseTreeNode(P);
			errTypeDed();
			return false;
		}
		if(isTypeRegistered((char*)tokNm->extra)){
			freeLexicalTokenList(T);
			freeParseTreeNode(P);
			freeType(typ);
			reportError("PP004", 	"Type Name Already Registered");
			return false;
		}
		registerType((char*)tokNm->extra, typ);
		freeLexicalTokenList(T);
		freeParseTreeNode(P);
		return true;
	}else if (len > 12 && strncmp(str, "typelistadd ", 12) == 0){
		LexicalTokenList *T = lexicalAnalyzeString((char*)str+12, len-12);
		if(T == NULL){
			errLex();
			return false;
		}
		PTree ** treeArr;
		int cnt = parseTypeList(T, &treeArr);
		if(cnt == 0){
			freeLexicalTokenList(T);
			errParse();
			return false;
		}
		LexicalToken *tokNm = T->first;
		int i;
		for(i=0; i<cnt; i++){
			Type typ = deduceTypeDeclType(treeArr[i]);
			if(typ.base == TB_ERROR || typ.base == TB_TYPELIST){
				int x;
				for(x=0; x<cnt; x++){
					freeParseTreeNode(treeArr[x]);
				}
				free(treeArr);
				freeLexicalTokenList(T);
				errTypeDed();
				return false;
			}
			addToTypeList((char*)tokNm->extra, typ);
		}
		int x;
		for(x=0; x<cnt; x++){
			freeParseTreeNode(treeArr[x]);
		}
		free(treeArr);
		freeLexicalTokenList(T);

		return true;
	}
	return false;
}

