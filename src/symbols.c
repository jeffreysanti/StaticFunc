/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  symbols.c
 *  Symbols Table
 *
 */

#include "symbols.h"

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
			S = (Symbol*)S->prev;
			free(S->next);
			S->next = NULL;
		}else{
			free(S);
			S = NULL;
		}
	}
}

void enterGlobalSpace()
{
	while(level > SYMLEV_GLOBAL){
		exitScope();
	}
}

void addSymbol(char *sym, Type typ)
{
	Symbol * sb = malloc(sizeof(Symbol));
	sb->id = id;
	id ++;
	sb->level = level;
	sb->nm = sym;
	sb->sig = typ;
	sb->prev = (void*)S;
	if(S == NULL){
		S = sb;
	}else{
		S->next = (void*)sb;
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




