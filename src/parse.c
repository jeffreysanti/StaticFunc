/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  parse.c
 *  Recursive Decent Parser
 *
 */

#include "parse.h"


void parse(LexicalTokenList *TL)
{
	PTree *root = newParseTree(PTT_EXPR);

	PState *ps = malloc(sizeof(PState));
	if(ps == NULL){
		fatalError("Out of Memory [parse: PState]\n");
	}
	ps->root = root;
	ps->tl = TL;
	ps->token = ps->tl->first;
	ps->err = false;

	if(ps->token == NULL){
		reportError("PS001", 	"Empty Token List\n");
		return;
	}

	if(!prodNumericExpr(ps)){
		reportError("PS002", 	"Parse is not numeric expression\n");
		return;
	}

	// is anything left
	if(ps->token->typ != LT_EOF){
		reportError("PS004", 	"Remaining Tokens Not Parsed\n");
		return;
	}

	freeParseTreeNode(root);
}

// Terminators
bool termMul(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "*")==0);
	if(ret)
		printf(" * ");
	return advanceToken(ps, ret);
}bool termAdd(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "+")==0);
	if(ret)
			printf(" + ");
	return advanceToken(ps, ret);
}bool termSub(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "-")==0);
	if(ret)
			printf(" - ");
	return advanceToken(ps, ret);
}bool termDiv(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "/+")==0);
	if(ret)
			printf(" / ");
	return advanceToken(ps, ret);
}bool termMod(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "%")==0);
	if(ret)
			printf(" % ");
	return advanceToken(ps, ret);
}bool termExp(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "^")==0);
	if(ret)
			printf(" ^ ");
	return advanceToken(ps, ret);
}bool termParenLeft(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "(")==0);
	return advanceToken(ps, ret);
}bool termParenRight(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ")")==0);
	return advanceToken(ps, ret);
}bool termIdentifier(PState *ps){
	bool ret = (ps->token->typ == LT_IDENTIFIER);
	if(ret)
				printf(" ID ");
	return advanceToken(ps, ret);
}bool termFloatLit(PState *ps){
	bool ret = (ps->token->typ == LT_FLOAT);
	if(ret)
				printf(" # ");
	return advanceToken(ps, ret);
}bool termIntLit(PState *ps){
	bool ret = (ps->token->typ == LT_INTEGER);
	if(ret)
				printf(" # ");
	return advanceToken(ps, ret);
}


// NumericExpression
bool prodNumericExpr(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericExprFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericExprFact1(PState *ps){
	return prodNumericExprA(ps);
}bool prodNumericExprA(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericExprAFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericExprAFact1(PState *ps){
	if(!prodNumericExprB(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termAdd(ps) && !termSub(ps)){
			break;
		}
		if(!prodNumericExprB(ps)){
			ps->token = tkStart;
			break;
		}
	}
	return true;
}bool prodNumericExprB(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericExprBFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericExprBFact1(PState *ps){
	if(!prodNumericExprC(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termMul(ps) && !termDiv(ps) && !termMod(ps)){
			break;
		}
		if(!prodNumericExprC(ps)){
			ps->token = tkStart;
			break;
		}
	}
	return true;
}bool prodNumericExprC(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericExprCFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodNumericExprCFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericExprCFact1(PState *ps){
	return prodNumericExprD(ps) && termExp(ps) && prodNumericExprC(ps);
}bool prodNumericExprCFact2(PState *ps){
	return prodNumericExprD(ps);
}bool prodNumericExprD(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericExprDFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodNumericExprDFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericExprDFact1(PState *ps){
	return termParenLeft(ps) && prodNumericExprA(ps) && termParenRight(ps);
}bool prodNumericExprDFact2(PState *ps){
	return prodNumericValue(ps);
}







// NumericVal
bool prodNumericValue(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericValueFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodNumericValueFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericValueFact1(PState *ps){
	return prodNumericLiteral(ps);
}bool prodNumericValueFact2(PState *ps){
	return termIdentifier(ps);
}

// NumericLiteral
bool prodNumericLiteral(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericLiteralFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodNumericLiteralFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericLiteralFact1(PState *ps){
	return termFloatLit(ps);
}bool prodNumericLiteralFact2(PState *ps){
	return termIntLit(ps);
}







