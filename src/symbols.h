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
  int uuid;
  char *refname; // name in code (null if temp)
  Type sig;

  struct Variable *next;
  struct Variable *prev;

  struct Scope *scope;
} Variable;

typedef struct{
  int uuid;
  int level;
  struct Variable *variables;

  struct Scope *next;
  struct Scope *prev;

  struct Scope *parentScope;
} Scope;


void initScopeSystem();
void freeScopeSystem();

void enterGlobalSpace();

void enterNewScope();
void exitScope();

Scope *currentScope();

// mutators
Variable *defineVariable(char *nm, Type typ);


// accessors
bool variableExistsCurrentScope(char *sym);
bool variableExists(char *sym);

Variable *getNearbyVariable(char *sym);
Type getNearbyVariableTypeOrErr(char *sym, int lineno);

char *getVariableUniqueName(Variable *);

/*



typedef struct{
	int level;
	Type sig;
	char *nm;

	long id;
	long long scopeID;

	struct Symbol *prev;
} Symbol;






void addSymbol(char *sym, Type typ);
Type getSymbolType(char *sym, int lineno);

char *getSymbolUniqueName(char *sym);


bool symbolExists(char *sym);


void freeOrResetScopeSystem();

Symbol *lastSymbol();

void dumpSymbolTable(FILE *fp);

*/


#endif /* STATICFUNC_SRC_SYMBOLS_H_ */
