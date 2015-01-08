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


#define ENTER(x) printf("Entered %s\n", x);
#define EXIT(x, y) { if(y) printf("CONFIRMED %s\n", x); else printf("Exited %s\n", x); return y; }

void parse(LexicalTokenList *TL);


typedef struct{
	PTree *root;
	LexicalTokenList *tl;
	LexicalToken *token;
	int err;

	PTree *child;
}PState;

typedef struct{
	bool (*term)(PState*);
	PTType typ;
}ArtithmeticParseParam;

static bool advanceToken(PState *ps, bool ret){
	if(!ret)
		return false;
	if(ps->token->next == NULL){
		reportError("PS003", 	"Out of tokens\n");
		ps->err ++;
	}else{
		ps->token = ps->token->next;
	}
	return ret;
}

static bool resetChildNode(PState *ps, PTree *root){
	freeParseTreeNode(ps->child);
	freeParseTreeNode(root);
	ps->child = NULL;
	return false;
}

static PTree *storeAndNullChildNode(PState *ps){
	PTree *child = ps->child;
	ps->child = NULL;
	return child;
}

bool helperPossibleFunctionCalls(PState *ps);
bool helperArithmetic(PState *ps, bool (*nextProd)(PState*), ArtithmeticParseParam A[], int num);


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
bool termStringLit(PState *ps);
bool termDot(PState *ps);
bool termSqbrLeft(PState *ps);
bool termSqbrRight(PState *ps);
bool termFor(PState *ps);
bool termIn(PState *ps);
bool termWhere(PState *ps);
bool termComma(PState *ps);
bool termNot(PState *ps);
bool termAnd(PState *ps);
bool termXOr(PState *ps);
bool termOr(PState *ps);
bool termEqual(PState *ps);
bool termNotEqual(PState *ps);
bool termGT(PState *ps);
bool termLT(PState *ps);
bool termGTE(PState *ps);
bool termLTE(PState *ps);
bool termShiftRight(PState *ps);
bool termShiftLeft(PState *ps);
bool termStmtEnd(PState *ps);
bool termDeclMutable(PState *ps);
bool termColon(PState *ps);
bool termAssign(PState *ps);
bool termBraceLeft(PState *ps);
bool termBraceRight(PState *ps);
bool termIf(PState *ps);
bool termElse(PState *ps);
bool termWhile(PState *ps);



bool prodNumericLiteral(PState *ps);
bool prodNumericLiteralFact1(PState *ps);
bool prodNumericLiteralFact2(PState *ps);

bool prodVarValue(PState *ps);
bool prodVarValueFact1(PState *ps);
bool prodVarValueFact2(PState *ps);
bool prodVarValueFact3(PState *ps);
bool prodVarValueFact4(PState *ps);

bool prodValue(PState *ps);
bool prodValueFact1(PState *ps);
bool prodValueFact2(PState *ps);
bool prodValueFact3(PState *ps);



bool prodExpr(PState *ps);
bool prodExprFact1(PState *ps);
bool prodExprA(PState *ps);
bool prodExprAFact1(PState *ps);
bool prodExprB(PState *ps);
bool prodExprBFact1(PState *ps);
bool prodExprC(PState *ps);
bool prodExprCFact1(PState *ps);
bool prodExprD(PState *ps);
bool prodExprDFact1(PState *ps);
bool prodExprE(PState *ps);
bool prodExprEFact1(PState *ps);
bool prodExprF(PState *ps);
bool prodExprFFact1(PState *ps);
bool prodExprG(PState *ps);
bool prodExprGFact1(PState *ps);
bool prodExprH(PState *ps);
bool prodExprHFact1(PState *ps);
bool prodExprI(PState *ps);
bool prodExprIFact1(PState *ps);
bool prodExprIFact2(PState *ps);
bool prodExprJ(PState *ps);
bool prodExprJFact1(PState *ps);
bool prodExprJFact2(PState *ps);
bool prodExprK(PState *ps);
bool prodExprKFact1(PState *ps);
bool prodExprKFact2(PState *ps);
bool prodExprKFact3(PState *ps);


bool prodArrayExpr(PState *ps);
bool prodArrayExprFact1(PState *ps);
bool prodArrayExprFact2(PState *ps);

bool prodArrayExprAux(PState *ps);
bool prodArrayExprAuxFact1(PState *ps);
bool prodArrayExprAuxFact2(PState *ps);

bool prodParamExpr(PState *ps);
bool prodParamExprFact1(PState *ps);
bool prodParamExprFact2(PState *ps);


bool prodStmt(PState *ps);
bool prodStmtFact1(PState *ps);
bool prodStmtFact2(PState *ps);
bool prodStmtFact3(PState *ps);
bool prodStmtFact4(PState *ps);
bool prodStmtFact5(PState *ps);
bool prodStmtFact6(PState *ps);
bool prodStmtFact7(PState *ps);

bool prodStmtBlock(PState *ps);
bool prodStmtBlockFact1(PState *ps);
bool prodStmtBlockFact2(PState *ps);

bool prodDecl(PState *ps);
bool prodDeclFact1(PState *ps);

bool prodDeclType(PState *ps);
bool prodDeclTypeFact1(PState *ps);
bool prodDeclMod(PState *ps);
bool prodDeclModFact1(PState *ps);
bool prodDeclModFact2(PState *ps);
bool prodDeclAux(PState *ps);
bool prodDeclAuxFact1(PState *ps);
bool prodDeclAuxFact2(PState *ps);


bool prodAssign(PState *ps);
bool prodAssignFact1(PState *ps);

bool prodCond(PState *ps);
bool prodCondFact1(PState *ps);

bool prodWhile(PState *ps);
bool prodWhileFact1(PState *ps);

bool prodFor(PState *ps);
bool prodForFact1(PState *ps);


#endif /* STATICFUNC_SRC_PARSE_H_ */
