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

  enterGlobalSpace();
}

void freeVariable(Variable *v){
  Variable *next = (Variable*)v->next;
  freeType(v->sig);
  if(v->refname != NULL){
    free(v->refname);
  }
  free(v);
  if(next != NULL){
    freeVariable(next);
  }
}

void freeScope(Scope* s){
  Scope *next = (Scope*)s->next;
  if(s->variables != NULL){
    freeVariable((Variable*)s->variables);
  }
  free(s);
  if(next != NULL){
    freeScope(next);
  }
}

void freeScopeSystem(){
  for(int i=0; i<SCOPE_LEVELS; i++){
    if(S[i].next != NULL){
      freeScope((Scope*)S[i].next);
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


Scope *currentScope(){
  return SC;
}


Variable *defineVariable(char *sym, Type typ)
{
  Variable * sb = malloc(sizeof(Variable));
  sb->uuid = nextVariableID ++;
  sb->scope = (void*)SC;
  if(sym == NULL){
    sb->refname = NULL;
  }else{
    sb->refname = strdup(sym);
  }
  sb->sig = duplicateType(typ);
  sb->disposedTemp = false;
  sb->prev = NULL;
  sb->next = SC->variables;
  SC->variables = (void*)sb;
  return sb;
}


bool variableExistsCurrentScope(char *sym){
  Variable *ptr = (Variable*)SC->variables;
  while(ptr != NULL){
    if(ptr->refname != NULL && strcmp(sym, ptr->refname) == 0){
      return true;
    }
    ptr = (Variable*)ptr->next;
  }
  return false;
}

bool variableExists(char *sym){
  Scope *scope = SC;
  while(scope != NULL){
    Variable *ptr = (Variable*)scope->variables;
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
  Scope *scope = SC;
  while(scope != NULL){
    Variable *ptr = (Variable*)scope->variables;
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

char *getVariableUniqueName(Variable *v)
{
  if(v == NULL){
    return NULL;
  }
  if(v->refname == NULL){
    char *dest = calloc(24+1, 1);
    sprintf(dest, ".tmp%d", v->uuid);
    return dest;
  }else{
    char *dest = calloc(strlen(v->refname)+20+1, 1);
    sprintf(dest, "%s.%d", v->refname, v->uuid);
    return dest;
  }
}

void dumpSymbolTable(FILE *f){
  int scopeNo = 1;
  for(int i=0; i<SCOPE_LEVELS; i++){
    if(S[i].next == NULL)
      return;

    Scope *scope = (Scope*)S[i].next;
    while(scope != NULL){
      Variable *var = (Variable*)scope->variables;
      while(var != NULL){
	for(int x=0; x<=i; x++) fprintf(f, "##");
	char *nm = "";
	if(var->refname != NULL) nm = var->refname;
	char *sigStr = getTypeAsString(var->sig);
	fprintf(f, "%3d %s : %s\n", scopeNo, nm, sigStr);
	free(sigStr);
	var = (Variable*)var->next;
      }
      scopeNo ++;
      scope = (Scope*)scope->next;
    }
  }
  fprintf(f,"---SDUMP OVER---\n");
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






