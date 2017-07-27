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
#include "uthash/utarray.h"
#include "hashset/hashset.h"
#include "parsetree.h"

#define SYMLEV_GLOBAL 0


#define SCOPETYPE_FUNCTION 1
#define SCOPETYPE_NON_FUNCTION 2



typedef struct Variable{
  int uuid;
  char *refname; // name in code (null if temp)
  Type sig;
  bool disposedTemp;
  bool referencedExternally;

  struct Variable *next;
  struct Variable *prev;

  struct Scope *scope;
} Variable;

typedef struct {
  hashset_t variables; // list of variable names
  hashset_t spaces; // list of scopes in order from here-1 to global-1
} LambdaSpace;


typedef struct PackedLambdaData PackedLambdaData;
struct PTree;


typedef struct Scope{
  int uuid;
  int level;
  int type;
  bool globalScope;
  struct Variable *variables;

  LambdaSpace lambdaspace;
  PackedLambdaData *packedLambdaSpace;

  struct Scope *next;
  struct Scope *prev;

  struct Scope *parentScope;
} Scope;

void initScopeSystem();
void freeScopeSystem();

void generateLambdaContainers();

void enterGlobalSpace();

void enterNewScope(PTree *tree, int type);
void exitScope();

Scope *currentScope();
Scope *getMethodScope(Scope *scope);
int getMethodScopeDistance(Scope *parent, Scope *child);
Scope *prevMethodScope(Scope *scope);

// mutators
Variable *defineVariable(PTree *tree, char *nm, Type typ);
Variable *defineUnattachedVariable(Type typ);

// accessors
bool variableExistsCurrentScope(char *sym);
bool variableExists(char *sym);

Variable *getVariable(char *sym);
Type getNearbyVariableTypeOrErr(char *sym, int lineno);

char *getVariableUniqueName(Variable *);

void dumpSymbolTable(FILE *f);

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
