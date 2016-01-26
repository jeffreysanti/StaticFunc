/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  symbols.c
 *  Symbols Table
 *
 */

#include "symbols.h"

#define SCOPE_LEVELS 128

Scope S[SCOPE_LEVELS];
int nextScopeID = 0;
int scopeLevel = 0;
int nextVariableID = 1;

Scope *SC = NULL;



void initScopeSystem(){
  for(int i=0; i<SCOPE_LEVELS; i++){
    S[i].uuid = -1;
    S[i].level = i;
    S[i].variables = NULL;
    S[i].next = NULL;
    S[i].prev = NULL;
    S[i].parentScope = NULL;
  }

  nextScopeID = 1;

  // initialize global scope
  Scope *global = malloc(sizeof(Scope));
  global->uuid = 0;
  global->level = 0;
  global->variables = NULL;
  global->next = NULL;
  global->prev = (void*)&S[0];
  global->parentScope = NULL;
  S[0].next = (void*)global;
}

void freeScopeSystem(){
  for(int i=0; i<SCOPE_LEVELS; i++){
    if(S[i].next != NULL){
      free(S[i].next); // TODO: Obviously not correct
    }
  }
}


void enterGlobalSpace(){
  scopeLevel = 0;
  SC = (Scope*)S[0].next;
}

void exitScope(){
  if(scopeLevel > 0){
    scopeLevel --;
    SC = (Scope*)SC->parentScope;
  }
}


void enterNewScope(){
  scopeLevel ++;

  Scope *s = malloc(sizeof(Scope));
  s->uuid = nextScopeID++;
  s->level = scopeLevel;
  s->variables = NULL;
  s->next = S[scopeLevel].next;
  s->prev = (void*)&S[scopeLevel];
  s->parentScope = (void*)SC;
  S[scopeLevel].next = (void*)s;

  SC = s;
}


Variable *defineVariable(char *sym, Type typ)
{
  Variable * sb = malloc(sizeof(Variable));
  sb->uuid = nextVariableID ++;
  sb->scope = (void*)SC;
  sb->refname = strdup(sym);
  sb->sig = duplicateType(typ);
  sb->prev = NULL;
  sb->next = SC->variables;
  SC->variables = (void*)sb;
  return sb;
}


bool variableExistsCurrentScope(char *sym){
  Variable *ptr = SC->variables;
  while(ptr != NULL){
    if(ptr->refname != NULL && strcmp(sym, ptr->refname) == 0){
      return true;
    }
    ptr = (Variable*)ptr->next;
  }
  return false;
}

bool variableExists(char *sym){
  Variable *ptr = SC->variables;
  Scope *scope = SC;
  while(scope != NULL){
    while(ptr != NULL){
      if(ptr->refname != NULL && strcmp(sym, ptr->refname) == 0){
	return true;
      }
      ptr = (Variable*)ptr->next;
    }
    scope = (Scope*)scope->parentScope;
  }
  return false;
}


Variable *getNearbyVariable(char *sym){
  Variable *ptr = SC->variables;
  Scope *scope = SC;
  while(scope != NULL){
    while(ptr != NULL){
      if(ptr->refname != NULL && strcmp(sym, ptr->refname) == 0){
	return ptr;
      }
      ptr = (Variable*)ptr->next;
    }
    scope = (Scope*)scope->parentScope;
  }
  return NULL;
}

Type getNearbyVariableTypeOrErr(char *sym, int lineno){
  Variable *v = getNearbyVariable(sym);
  if(v == NULL){
    reportError("SS001", "Symbol %s Not Found: Line %d", sym, lineno);
    return newBasicType(TB_ERROR);
  }
  return v->sig;
}




/*
Symbol *S = NULL;
long long id = 0;
int level = 1;

void enterScope()
{
	level ++;
}

void exitScope()
{
	level --;
	while(S != NULL && S->level > level){
		if(S->prev != NULL){
			Symbol *sOld = S;
			S = (Symbol*)S->prev;
			freeType(sOld->sig);
			free(sOld);
		}else{
			freeType(S->sig);
			free(S);
			S = NULL;
		}
	}
}




Type getSymbolType(char *sym, int lineno)
{
	Symbol *ptr = S;
	while(ptr != NULL){
		if(strcmp(sym, ptr->nm) == 0){
			return ptr->sig;
		}
		ptr = (Symbol*)ptr->prev;
	}
	reportError("SS001", "Symbol %s Not Found: Line %d", sym, lineno);
	return newBasicType(TB_ERROR);
}

char *getSymbolUniqueName(char *sym)
{
	Symbol *ptr = S;
	while(ptr != NULL){
		if(strcmp(sym, ptr->nm) == 0){
			char *dest = calloc(strlen(sym)+20+1, 1);
			sprintf(dest, "%s.%ld", sym, ptr->id);
			return dest;
		}
		ptr = (Symbol*)ptr->prev;
	}
	return NULL;
}

void freeOrResetScopeSystem()
{
	while(S != NULL){
		exitScope();
	}
}

bool symbolExists(char *sym)
{
	Symbol *ptr = S;
	while(ptr != NULL){
		if(strcmp(sym, ptr->nm) == 0){
			return true;
		}
		ptr = (Symbol*)ptr->prev;
	}
	return false;
}

bool symbolExistsCurrentLevel(char *sym)
{
	Symbol *ptr = S;
	while(ptr != NULL){
		if(strcmp(sym, ptr->nm) == 0 && ptr->level == level){
			return true;
		}
		ptr = (Symbol*)ptr->prev;
	}
	return false;
}

Symbol *lastSymbol()
{
	return S;
}


void dumpSymbolTable(FILE *fp){
  if(S == NULL){
    fprintf(fp, "No Symbols");
  }else{
    Symbol *sym = S;
    while(sym != NULL){
      for(int i=0; i<sym->level; i++){
	fprintf(fp, "*");
      }
      fprintf(fp, " -> %s\n", sym->nm);
      sym = sym->prev;
    }
  }
  fprintf(fp, "----\n");
}


*/






