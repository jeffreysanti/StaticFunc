/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  parse.h
 *  Recursive Decent Parser
 *
 */

#ifndef STATICFUNC_SRC_PARSE_H_
#define STATICFUNC_SRC_PARSE_H_

#include "lextokens.h"
#include "parsetree.h"

typedef char bool;
#define true 1
#define false 0

void parse(LexicalTokenList *TL);


typedef struct{
	PTree *root;
	LexicalTokenList *tl;
	LexicalToken *token;
	bool err;
}PState;

static bool advanceToken(PState *ps, bool ret){
	if(!ret)
		return false;
	if(ps->token->next == NULL){
		reportError("PS003", 	"Out of tokens\n");
		ps->err = true;
	}else{
		ps->token = ps->token->next;
	}
	return ret;
}


bool termMultiply(PState *ps);
bool termAdd(PState *ps);
bool termSub(PState *ps);
bool termDiv(PState *ps);
bool termMod(PState *ps);
bool termExp(PState *ps);
bool termParenLeft(PState *ps);
bool termParenRight(PState *ps);
bool termIdentifier(PState *ps);
bool termFloatLit(PState *ps);
bool termIntLit(PState *ps);



bool prodNumericLiteral(PState *ps);
bool prodNumericLiteralFact1(PState *ps);
bool prodNumericLiteralFact2(PState *ps);

bool prodNumericValue(PState *ps);
bool prodNumericValueFact1(PState *ps);
bool prodNumericValueFact2(PState *ps);




bool prodNumericExpr(PState *ps);
bool prodNumericExprFact1(PState *ps);
bool prodNumericExprA(PState *ps);
bool prodNumericExprAFact1(PState *ps);
bool prodNumericExprB(PState *ps);
bool prodNumericExprBFact1(PState *ps);
bool prodNumericExprC(PState *ps);
bool prodNumericExprCFact1(PState *ps);
bool prodNumericExprCFact2(PState *ps);
bool prodNumericExprD(PState *ps);
bool prodNumericExprDFact1(PState *ps);
bool prodNumericExprDFact2(PState *ps);




#endif /* STATICFUNC_SRC_PARSE_H_ */
