/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  symbols.h
 *  Symbols Table
 *
 */

#ifndef STATICFUNC_SRC_SYMBOLS_H_
#define STATICFUNC_SRC_SYMBOLS_H_

#include "types.h"

#define SYMLEV_GLOBAL 0

typedef struct{
	int level;
	Type sig;
	char *nm;

	long long id;

	struct Symbol *prev;
} Symbol;


void enterScope();
void exitScope();

void enterGlobalSpace();

void addSymbol(char *sym, Type typ);
Type getSymbolType(char *sym, int lineno);

bool symbolExists(char *sym);
bool symbolExistsCurrentLevel(char *sym);

void freeOrResetScopeSystem();

#endif /* STATICFUNC_SRC_SYMBOLS_H_ */
