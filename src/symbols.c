/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  symbols.c
 *  Symbols Table
 *
 */

#include "symbols.h"
#include "scopeprocessor.h"

#define SCOPE_LEVELS 128

Scope S[SCOPE_LEVELS];
int nextScopeID = 0;
int scopeLevel = 0;
int nextVariableID = 1;

Scope *SC = NULL;
UT_array *temp_vars_tofree = NULL;



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
  global->lambdaspace.variables = hashset_create();
  global->lambdaspace.spaces = hashset_create();
  global->packedLambdaSpace = NULL;
  global->type = SCOPETYPE_NON_FUNCTION;
  global->globalScope = true;

  utarray_new(temp_vars_tofree, &ut_ptr_icd);

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
  hashset_destroy(s->lambdaspace.variables);
  hashset_destroy(s->lambdaspace.spaces);
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

  Variable **p = NULL;
	while ( (p=(Variable**)utarray_next(temp_vars_tofree,p))) {
		freeVariable(*p);
	}
	utarray_free(temp_vars_tofree);
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


void enterNewScope(PTree *tree, int type){
  scopeLevel ++;

  Scope *s = malloc(sizeof(Scope));
  s->uuid = nextScopeID++;
  s->type = type;
  s->level = scopeLevel;
  s->variables = NULL;
  s->next = S[scopeLevel].next;
  s->prev = (void*)&S[scopeLevel];
  s->parentScope = (void*)SC;

  s->lambdaspace.variables = hashset_create();
  s->lambdaspace.spaces = hashset_create();
  s->packedLambdaSpace = NULL;

  if(SC->globalScope && type != SCOPETYPE_FUNCTION){
	s->globalScope = true;
  }else{
	s->globalScope = false;
  }
  

  tree->scope = (struct Scope*)s;

  S[scopeLevel].next = (void*)s;

  SC = s;
}


Scope *currentScope(){
  return SC;
}

Scope *getMethodScope(Scope *scope){
  while(scope->type != SCOPETYPE_FUNCTION){
    scope = (Scope*)scope->parentScope;

    if(scope == NULL){
      return NULL;
    }
  }
  return scope;
}

int getMethodScopeDistance(Scope *parent, Scope *child){
	if(parent == child){
		return 0;
	}
	int cnt = 0;
	while(parent != child){
		if(child->type == SCOPETYPE_FUNCTION){
			cnt ++;
		}
		child = (Scope*)child->parentScope;

		if(child == NULL){
			return 0;
		}
	}
	return cnt;
}

Scope *prevMethodScope(Scope *scope){
	scope = getMethodScope(scope);
	if(scope == NULL){
		return NULL;
	}
	scope = scope->prev;
	if(scope == NULL){
		return NULL;
	}
	return getMethodScope(scope);
}


Variable *defineVariable(PTree *tree, char *sym, Type typ)
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
  sb->referencedExternally = false;
  sb->prev = NULL;
  sb->next = SC->variables;
  SC->variables = (void*)sb;

  tree->var = sb;

  return sb;
}

Variable *defineUnattachedVariable(Type typ){
  Variable * sb = malloc(sizeof(Variable));
  sb->uuid = nextVariableID ++;
  sb->refname = NULL;
  sb->sig = duplicateType(typ);
  sb->disposedTemp = false;
  sb->referencedExternally = false;
  sb->prev = NULL;
  sb->next = NULL;

  utarray_push_back(temp_vars_tofree, &sb);

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


Variable *getVariable(char *sym){
  Scope *scope = SC;
  Scope *accessMethodScope = getMethodScope(scope);
  
  while(scope != NULL){
    Variable *ptr = (Variable*)scope->variables;
    while(ptr != NULL){
      if(ptr->refname != NULL && strcmp(sym, ptr->refname) == 0){
        Scope *rootMethodScope = getMethodScope(scope);

        if(accessMethodScope != NULL && rootMethodScope != NULL && accessMethodScope != rootMethodScope){
          ptr->referencedExternally = true; // needs to be on heap

          //also pack this scope reference into the lamda container
           hashset_add(accessMethodScope->lambdaspace.spaces, rootMethodScope);
           hashset_add(rootMethodScope->lambdaspace.variables, ptr);
        }
	      return ptr;
      }
      ptr = (Variable*)ptr->next;
    }
    scope = (Scope*)scope->parentScope;
  }
  return NULL;
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




void generateLambdaContainers(){
  // Each functional scope needs an associated lambda container
  // This lambda container will hold all variables in that scope which must be
  // held in the heap.

  for(int i=0; i<SCOPE_LEVELS; i++){
    Scope *scope = (Scope*)S[i].next;
    while(scope != NULL){

      if(scope->type == SCOPETYPE_FUNCTION){
        scope->packedLambdaSpace = processMethodScope(scope);
      }else if(scope->globalScope){
		  // if a route to the global scope exists without reaching a function
		  // then the variables in this group are global and must be packed into static data
		Variable *ptr = (Variable*)scope->variables;
		while(ptr != NULL){
			processSystemGlobal(ptr);
			ptr = (Variable*)ptr->next;
		}
	  }

      scope = (Scope*)scope->next;
    }
  }
}













