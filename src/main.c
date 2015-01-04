/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  main.c
 *  Compiler Entry Point
 *
 */




#include <stdio.h>
#include <stdlib.h>

#include "lextokens.h"
#include "io.h"

int main(int argc, char **argv){

	// Entry point to static_func compiler

	// initially just lex the input file

	if(argc < 2){
		fatalError("Error: No Input Files\n");
	}


	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){
		fatalError("Error: Could Not Open Input File\n");
	}
	LexicalTokenList *T = lexicalAnalyze(fp);
	fclose(fp);


	LexicalTokenList *TL = createLexicalTokenList();
	int i;
	for(i=0; i<20; i++){
		pushBasicToken(TL, LT_ADD);
	}
	outputLexicalTokenList(TL);
	freeLexicalTokenList(TL);


	return EXIT_SUCCESS;


}


