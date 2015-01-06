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

#include "parse.h"
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

	if(T != NULL){
		outputLexicalTokenList(T);

		// now parse
		parse(T);

		freeLexicalTokenList(T);
	}



	return EXIT_SUCCESS;


}


