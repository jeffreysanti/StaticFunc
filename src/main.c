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
#include "icg.h"

FILE *openOutputFile(char *outfl, char ext[3]){
	outfl[strlen(outfl)-3] = ext[0];
	outfl[strlen(outfl)-2] = ext[1];
	outfl[strlen(outfl)-1] = ext[2];
	FILE *ret = fopen(outfl, "w");
	return ret;
}

int main(int argc, char **argv){

	// Entry point to static_func compiler

	// initially just lex the input file

	if(argc < 2){
		fatalError("Error: No Input Files\n");
	}

	initTypeSystem();
	initFunctionSystem();
	initalizeBuiltInFunctions();
	initalizeScopeSystem();

	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){
		fatalError("Error: Could Not Open Input File\n");
	}

	char *outfl = malloc(strlen(argv[1]) +4 +1);
	strcpy(outfl, argv[1]);
	outfl[strlen(argv[1]) +4] = 0;
	outfl[strlen(argv[1])] = '.';
	outfl[strlen(argv[1])+1] = '.';
	outfl[strlen(argv[1])+2] = '.';
	outfl[strlen(argv[1])+3] = '.';

	LexicalTokenList *T = lexicalAnalyze(fp);
	fclose(fp);

	if(T != NULL){
		FILE *tmpout = openOutputFile(outfl, "lex");
		outputLexicalTokenList(T, tmpout);
		fclose(tmpout);

		// now parse
		PTree *tree = parse(T);
		if(tree != NULL){

			FILE *tmpout = openOutputFile(outfl, "ps1");
			dumpParseTreeDet(tree, 0, tmpout);
			fclose(tmpout);

			seperateFunctionsFromParseTree(&tree, false);

			if(semAnalyFunc(tree, true, getProgramReturnType())){
				freeOrResetScopeSystem();

				FILE *tmpout = openOutputFile(outfl, "ps2");
				dumpParseTreeDet(tree, 0, tmpout);
				fclose(tmpout);

				// now generate code :D
				icRunGen(tree, outfl);

				// now generate each used function version
				
			}
			freeParseTreeNode(tree);
		}
	}

	freeOrResetScopeSystem();
	freeTypeSystem();
	freeFunctionSystem();
	freeScopeSystem();

	if(T != NULL){
		freeLexicalTokenList(T);
	}

	free(outfl);

	return EXIT_SUCCESS;


}
