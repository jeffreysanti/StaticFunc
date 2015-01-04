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

int main(int argc, char **argv){

	// Entry point to static_func compiler

	// initially just lex the input file

	if(argc < 2){
		printf("Error: No Input Files\n");
		return EXIT_FAILURE;
	}

	printf("File: %s", argv[1]);


	return EXIT_SUCCESS;


}


