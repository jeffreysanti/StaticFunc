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

#include "lex.h"
#include "parse.h"
#include "io.h"
#include "types.h"
#include "functions.h"
#include "funcAnaly.h"
#include "nativeFunctions.h"

int main(int argc, char **argv){

	// Entry point to static_func compiler

	// initially just lex the input file

	if(argc < 2){
		fatalError("Error: No Input Files\n");
	}

	initTypeSystem();
	initFunctionSystem();
	initalizeBuiltInFunctions();

	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){
		fatalError("Error: Could Not Open Input File\n");
	}
	LexicalTokenList *T = lexicalAnalyze(fp);
	fclose(fp);

	if(T != NULL){
		//outputLexicalTokenList(T);

		// now parse
		PTree *tree = parse(T);
		if(tree != NULL){
			//dumpParseTreeDet(tree, 0);

			seperateFunctionsFromParseTree(&tree, false);

			dumpParseTreeDet(tree, 0);

			semAnalyFunc(tree, true, getProgramReturnType());

			freeParseTreeNode(tree);
		}
	}

	freeOrResetScopeSystem();
	freeTypeSystem();
	freeFunctionSystem();

	if(T != NULL){
		freeLexicalTokenList(T);
	}

	return EXIT_SUCCESS;


}


