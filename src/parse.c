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
	PState *ps = malloc(sizeof(PState));
	if(ps == NULL){
		fatalError("Out of Memory [parse: PState]\n");
	}
	ps->root = NULL;
	ps->tl = TL;
	ps->token = ps->tl->first;
	ps->err = 0;
	ps->child = NULL;


	if(ps->token == NULL){
		reportError("PS001", 	"Empty Token List\n");
		return;
	}

	if(!prodStmtBlock(ps)){
		reportError("PS002", 	"Parse is not statement block\n");
		return;
	}

	ps->root = ps->child;

	// is anything left
	if(ps->token->typ != LT_EOF){
		reportError("PS004", 	"Remaining Tokens Not Parsed\n");
		return;
	}

	dumpParseTree(ps->root, 0);

	freeParseTreeNode(ps->root);
	free(ps);
}

// Terminators
bool termMul(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "*")==0);
	return advanceToken(ps, ret);
}bool termAdd(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "+")==0);
	return advanceToken(ps, ret);
}bool termSub(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "-")==0);
	return advanceToken(ps, ret);
}bool termDiv(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "/")==0);
	return advanceToken(ps, ret);
}bool termMod(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "%")==0);
	return advanceToken(ps, ret);
}bool termExp(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "^")==0);
	return advanceToken(ps, ret);
}bool termParenLeft(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "(")==0);
	return advanceToken(ps, ret);
}bool termParenRight(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ")")==0);
	return advanceToken(ps, ret);
}bool termIdentifier(PState *ps){
	bool ret = (ps->token->typ == LT_IDENTIFIER);
	return advanceToken(ps, ret);
}bool termFloatLit(PState *ps){
	bool ret = (ps->token->typ == LT_FLOAT);
	return advanceToken(ps, ret);
}bool termIntLit(PState *ps){
	bool ret = (ps->token->typ == LT_INTEGER);
	return advanceToken(ps, ret);
}bool termStringLit(PState *ps){
	bool ret = (ps->token->typ == LT_TEXT);
	return advanceToken(ps, ret);
}bool termDot(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ".")==0);
	return advanceToken(ps, ret);
}bool termSqbrLeft(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "[")==0);
	return advanceToken(ps, ret);
}bool termSqbrRight(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "]")==0);
	return advanceToken(ps, ret);
}bool termFor(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "for")==0);
	return advanceToken(ps, ret);
}bool termIn(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "in")==0);
	return advanceToken(ps, ret);
}bool termWhere(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "where")==0);
	return advanceToken(ps, ret);
}bool termComma(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ",")==0);
	return advanceToken(ps, ret);
}bool termNot(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "!")==0);
	return advanceToken(ps, ret);
}bool termAnd(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "&")==0);
	return advanceToken(ps, ret);
}bool termXOr(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "~")==0);
	return advanceToken(ps, ret);
}bool termOr(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "|")==0);
	return advanceToken(ps, ret);
}bool termEqual(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "==")==0);
	return advanceToken(ps, ret);
}bool termNotEqual(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "!=")==0);
	return advanceToken(ps, ret);
}bool termGT(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ">")==0);
	return advanceToken(ps, ret);
}bool termLT(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "<")==0);
	return advanceToken(ps, ret);
}bool termGTE(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ">=")==0);
	return advanceToken(ps, ret);
}bool termLTE(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "<=")==0);
	return advanceToken(ps, ret);
}bool termShiftRight(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "shr")==0);
	return advanceToken(ps, ret);
}bool termShiftLeft(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "shl")==0);
	return advanceToken(ps, ret);
}bool termStmtEnd(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ";")==0);
	return advanceToken(ps, ret);
}bool termDeclMutable(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "mut")==0);
	return advanceToken(ps, ret);
}bool termColon(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, ":")==0);
	return advanceToken(ps, ret);
}bool termAssign(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "=")==0);
	return advanceToken(ps, ret);
}bool termBraceLeft(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "{")==0);
	return advanceToken(ps, ret);
}bool termBraceRight(PState *ps){
	bool ret = (ps->token->typ == LT_OP && strcmp(ps->token->extra, "}")==0);
	return advanceToken(ps, ret);
}





// Expression
bool prodExpr(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprFact1(PState *ps){
	return prodExprA(ps);
}bool prodExprA(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprAFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}


bool helperArithmetic(PState *ps, bool (*nextProd)(PState*), ArtithmeticParseParam A[], int num)
{
	//dumpParseTree(ps->root, 0);
	PTree *root = newParseTree(PTT_NOTYPE);
	if(!nextProd(ps)){
		return resetChildNode(ps, root);
	}
	insertParseNodeFromList(root, PTT_NOTYPE, storeAndNullChildNode(ps));
	int count = 0, i;
	while(true){
		bool done = false;
		for(i=0; i<num; i++){ // for each type
			if(!A[i].term(ps))
				continue;
			LexicalToken *tkStart = ps->token;
			if(!nextProd(ps)){
				resetChildNode(ps, root);
				return reportParseError(ps, "PS017", "Expression Missing Right Operand: Line %d", tkStart->lineNo);
			}
			insertParseNodeFromList(root, A[i].typ, storeAndNullChildNode(ps));
			setSecondLastParseNodeToken(root, tkStart->prev);
			count ++;
			done = true;
			break;
		}
		if(!done)
			break;
	}
	resetChildNode(ps, NULL);
	if(count > 0){
		mergeEndParseNodes(root);
		ps->child = root;
	}else{
		ps->child = extractIndependentLeftParseNodeLeaveChild(root); // remove association with arithmetic operation
		freeParseTreeNode(root);
	}
	return true;
}

bool prodExprAFact1(PState *ps){
	ArtithmeticParseParam A[1];
	A[0].term = &termOr; A[0].typ = PTT_OR;
	return helperArithmetic(ps, &prodExprB, A, 1);

}bool prodExprB(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprBFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprBFact1(PState *ps){
	ArtithmeticParseParam A[1];
	A[0].term = &termXOr; A[0].typ = PTT_XOR;
	return helperArithmetic(ps, &prodExprC, A, 1);
}bool prodExprC(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprCFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprCFact1(PState *ps){
	ArtithmeticParseParam A[1];
	A[0].term = &termAnd; A[0].typ = PTT_AND;
	return helperArithmetic(ps, &prodExprD, A, 1);
}bool prodExprD(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprDFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprDFact1(PState *ps){
	ArtithmeticParseParam A[2];
	A[0].term = &termEqual; A[0].typ = PTT_EQUAL;
	A[1].term = &termNotEqual; A[1].typ = PTT_NOT_EQUAL;
	return helperArithmetic(ps, &prodExprE, A, 2);
}bool prodExprE(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprEFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprEFact1(PState *ps){
	ArtithmeticParseParam A[4];
	A[0].term = &termLT; 	A[0].typ = PTT_LT;
	A[1].term = &termGT; 	A[1].typ = PTT_GT;
	A[2].term = &termGTE; 	A[2].typ = PTT_GTE;
	A[3].term = &termLTE; 	A[3].typ = PTT_LTE;
	return helperArithmetic(ps, &prodExprF, A, 4);
}bool prodExprF(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprFFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprFFact1(PState *ps){
	ArtithmeticParseParam A[2];
	A[0].term = &termShiftLeft;  A[0].typ = PTT_SHL;
	A[1].term = &termShiftRight; A[1].typ = PTT_SHR;
	return helperArithmetic(ps, &prodExprG, A, 2);
}bool prodExprG(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprGFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprGFact1(PState *ps){
	ArtithmeticParseParam A[2];
	A[0].term = &termAdd; A[0].typ = PTT_ADD;
	A[1].term = &termSub; A[1].typ = PTT_SUB;
	return helperArithmetic(ps, &prodExprH, A, 2);
}bool prodExprH(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprHFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprHFact1(PState *ps){
	ArtithmeticParseParam A[3];
	A[0].term = &termMul; A[0].typ = PTT_MULT;
	A[1].term = &termDiv; A[1].typ = PTT_DIV;
	A[2].term = &termMod; A[2].typ = PTT_MOD;
	return helperArithmetic(ps, &prodExprI, A, 3);
}bool prodExprI(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprIFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodExprIFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprIFact1(PState *ps){
	if(prodExprJ(ps)){
		PTree *op1 = storeAndNullChildNode(ps);
		if(termExp(ps)){ // exponential function applied
			LexicalToken *tkStart = ps->token;
			PTree *root = newParseTree(PTT_EXP);
			root->tok = tkStart->prev;
			setParseNodeChild(root, op1, PC_LEFT);
			if(!prodExprI(ps)){
				resetChildNode(ps, root);
				return reportParseError(ps, "PS025", "Exponent Missing Right Operand: Line %d", tkStart->lineNo);
			}
			setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT);
			ps->child = root;
			return true;
		}else{
			return resetChildNode(ps, op1);
		}
	}else{
		return resetChildNode(ps, NULL);
	}
}bool prodExprIFact2(PState *ps){
	return prodExprJ(ps);
}bool prodExprJ(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprJFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodExprJFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprJFact1(PState *ps){
	if(!termNot(ps)) return false;
	LexicalToken *tkStart = ps->token;
	if(!prodExprK(ps)){
		resetChildNode(ps, NULL);
		return reportParseError(ps, "PS026", "! Missing Right Operand: Line %d", tkStart->lineNo);
	}
	PTree *root = newParseTree(PTT_NOT);
	root->tok = tkStart->prev;
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT);
	ps->child = root;
	return true;
}bool prodExprJFact2(PState *ps){
	return prodExprK(ps);
}bool prodExprK(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprKFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodExprKFact2(ps); if(!ret) ps->token = start; else return true;
			ret = prodExprKFact3(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprKFact1(PState *ps){
	LexicalToken *tokStart = ps->token;
	if(!termParenLeft(ps)) return false;
	if(!prodExprA(ps)){
		return reportParseError(ps, "PS007", "Tokens inside parens do not form expression: Line %d\n", tokStart->lineNo);
	}
	if(!termParenRight(ps)){
		return reportParseError(ps, "PS008", "Parens not closed after expression: Line %d\n", tokStart->lineNo);
	}
	return true;
}bool prodExprKFact2(PState *ps){
	LexicalToken *tokStart = ps->token;
	if(!termSqbrLeft(ps)) return false;
	if(!prodArrayExpr(ps)){
		return reportParseError(ps, "PS009", "Tokens inside brackets do not form valid array expression: Line %d\n", tokStart->lineNo);
	}
	if(!termSqbrRight(ps)){
		return reportParseError(ps, "PS010", "Brackets do not closed after array expression: Line %d\n", tokStart->lineNo);
	}
	return true;
}bool prodExprKFact3(PState *ps){
	return prodValue(ps);
}


// function calls
/*bool helperPossibleFunctionCalls(PState *ps)
{
	bool foundOne = false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(termParenLeft(ps)){ // function call
			foundOne = true;
			prodParamExpr(ps);
			if(!termParenRight(ps)){
				return reportParseError(ps, "PS016", "Paren not closed after function call: Line %d\n", tkStart->lineNo);
			}
		}else{
			ps->token = tkStart;
			return foundOne;
		}
	}
}*/


// VarValue
bool prodVarValue(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodVarValueFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodVarValueFact2(ps); if(!ret) ps->token = start; else return true;
			ret = prodVarValueFact3(ps); if(!ret) ps->token = start; else return true;
			ret = prodVarValueFact4(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodVarValueFact1(PState *ps){ // op1.op2
	if(!termDot(ps))
		return false;
	PTree *root = newParseTree(PTT_DOT);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	root->tok = ps->token->prev;
	if(!termIdentifier(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS011", "Element Access Not Identifier: Line %d\n", ps->token->lineNo);
	}
	PTree *op2 = newParseTree(PTT_IDENTIFIER); // op2 (identifier)
	op2->tok = ps->token->prev;
	setParseNodeChild(root, op2, PC_RIGHT);
	ps->child = root;
	return prodVarValue(ps);
}bool prodVarValueFact2(PState *ps){ // op1[op2]
	if(!termSqbrLeft(ps))
		return false;
	PTree *root = newParseTree(PTT_ARR_ACCESS);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	root->tok = ps->token->prev;
	if(!prodExpr(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS005", "Tokens inside square bracket do not form expression: Line %d\n", ps->token->lineNo);
	}
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // op2
	if(!termSqbrRight(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS006", "Square bracket not closed after expression: Line %d\n", ps->token->lineNo);
	}
	ps->child = root;
	return prodVarValue(ps);
}bool prodVarValueFact3(PState *ps){ // op1(op2)
	if(!termParenLeft(ps))
		return false;
	PTree *root = newParseTree(PTT_ARR_ACCESS);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	root->tok = ps->token->prev;
	if(!prodParamExpr(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS025", "Tokens inside parens do not form paramater expression: Line %d\n", ps->token->lineNo);
	}
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // op2
	if(!termParenRight(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS016", "Paren not closed after function call: Line %d\n", ps->token->lineNo);
	}
	ps->child = root;
	return prodVarValue(ps);
}bool prodVarValueFact4(PState *ps){
	return true;
}




// Value
bool prodValue(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodValueFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodValueFact2(ps); if(!ret) ps->token = start; else return true;
			ret = prodValueFact3(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodValueFact1(PState *ps){
	return prodNumericLiteral(ps);
}bool prodValueFact2(PState *ps){
	if(termStringLit(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_STRING);
		ps->child->tok = ps->token->prev;
		return true;
	}
	return false;
}bool prodValueFact3(PState *ps){
	if(termIdentifier(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_IDENTIFIER);
		ps->child->tok = ps->token->prev;
		return prodVarValue(ps);
	}
	return false;
}

// NumericLiteral
bool prodNumericLiteral(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericLiteralFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodNumericLiteralFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericLiteralFact1(PState *ps){
	if(termFloatLit(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_FLOAT);
		ps->child->tok = ps->token->prev;
		return true;
	}
	return false;
}bool prodNumericLiteralFact2(PState *ps){
	if(termIntLit(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_INT);
		ps->child->tok = ps->token->prev;
		return true;
	}
	return false;
}




// ArrayExpression
bool prodArrayExpr(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodArrayExprFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodArrayExprFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodArrayExprFact1(PState *ps){
	if(!prodExpr(ps))
		return false;
	PTree *root = newParseTree(PTT_ARRAY_ELM);
	insertParseNodeFromList(root, PTT_ARRAY_ELM, storeAndNullChildNode(ps)); // first param
	ps->child = root;
	return prodArrayExprAux(ps);
}bool prodArrayExprFact2(PState *ps){
	resetChildNode(ps, NULL);
	//ps->child = newParseTree(PTT_ARRAY_ELM);
	return true; // empty array
}
// AUX
bool prodArrayExprAux(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodArrayExprAuxFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodArrayExprAuxFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodArrayExprAuxFact1(PState *ps){ // list comprehension
	if(!termFor(ps)) return false;

	// TODO:
	fatalError("List Comprehensions Not Implemented!\n");

	LexicalToken *tkStart = ps->token;
	if(!(termIdentifier(ps) && prodVarValue(ps) && termIn(ps) && prodExpr(ps))){
		return reportParseError(ps, "PS013", "Invalid List Comprehension: Line %d\n", tkStart->lineNo);
	}
	tkStart = ps->token;
	if(termWhere(ps)){
		if(!prodExpr(ps)){
			return reportParseError(ps, "PS014", "Invalid List Comprehension Where Clause: Line %d\n", tkStart->lineNo);
		}
	}
	return true;
}bool prodArrayExprAuxFact2(PState *ps){
	PTree *root = storeAndNullChildNode(ps);
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termComma(ps)){
			break;
		}
		if(!prodExpr(ps)){
			resetChildNode(ps, NULL);
			return reportParseError(ps, "PS012", "Element Listed in Array Not Expression: Line %d\n", tkStart->lineNo);
		}
		insertParseNodeFromList(root, PTT_ARRAY_ELM, storeAndNullChildNode(ps));
	}
	ps->child = root;
	return true;
}




// ParamExpression
bool prodParamExpr(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodParamExprFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodParamExprFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodParamExprFact1(PState *ps){
	if(!prodExpr(ps)) return false;
	PTree *root = newParseTree(PTT_PARAM_CONT);
	insertParseNodeFromList(root, PTT_PARAM_CONT, storeAndNullChildNode(ps)); // first param
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termComma(ps)){
			break;
		}
		if(!prodExpr(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS015", "Element Paramater Not Expression: Line %d\n", tkStart->lineNo);
		}
		insertParseNodeFromList(root, PTT_PARAM_CONT, storeAndNullChildNode(ps)); // first param
	}
	resetChildNode(ps, NULL);
	ps->child = root;
	return true;
}bool prodParamExprFact2(PState *ps){
	resetChildNode(ps, NULL);
	//ps->child = newParseTree(PTT_PARAM_CONT);
	return true; // no params
}

// Statement
bool prodStmt(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodStmtFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact2(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact3(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact4(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodStmtFact1(PState *ps){
	LexicalToken *tkStart = ps->token;
	if(termBraceLeft(ps) && prodStmtBlock(ps) && termBraceRight(ps)){
		ps->child->tok = tkStart;
		return true;
	}
	return false;
}bool prodStmtFact2(PState *ps){
	return prodDecl(ps);
}bool prodStmtFact3(PState *ps){
	return prodAssign(ps);
}bool prodStmtFact4(PState *ps){
	return false;
}


// Statement Block
bool prodStmtBlock(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodStmtBlockFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodStmtBlockFact1(PState *ps){
	if(!prodStmt(ps)) return false;
	PTree *root = newParseTree(PTT_STMTBLOCK);
	insertParseNodeFromList(root, PTT_STMTBLOCK, storeAndNullChildNode(ps));
	while(true){
		int errPre = ps->err;
		if(!prodStmt(ps)){
			if(ps->err > errPre){ // error mode: Eat Tokens Until Semicolon
				while(!termStmtEnd(ps)){
					advanceToken(ps, true);
				}
				continue;
			}else{
				break;
			}
		}
		insertParseNodeFromList(root, PTT_STMTBLOCK, storeAndNullChildNode(ps));
	}
	resetChildNode(ps, NULL);
	ps->child = root;
	return true;
}


// Declaration
bool prodDecl(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodDeclFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodDeclFact1(PState *ps){
	if(!prodDeclType(ps)) return false;// && termIdentifier(ps))) return false;
	PTree *root = newParseTree(PTT_DECL);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // type
	if(!termIdentifier(ps))
		return resetChildNode(ps, root);
	root->tok = ps->token->prev; // name of declaration
	LexicalToken *tkStart = ps->token->prev;
	if(termParenLeft(ps)){
		if(!prodExpr(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS026", "Invalid Declaration Initializer: Line %d\n", tkStart->lineNo);
		}
		setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // intitial value
		if(!termParenRight(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS027", "Declaration Initializer Missing Closing Paren: Line %d\n", tkStart->lineNo);
		}
	}
	if(!termStmtEnd(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS028", "Declaration Semicolon Missing: Line %d\n", tkStart->lineNo);
	}
	ps->child = root;
	return true;
}bool prodDeclType(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodDeclTypeFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodDeclTypeFact1(PState *ps){
	prodDeclMod(ps);
	PTree *root = newParseTree(PTT_DECL_TYPE);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // modifiers
	if(!termIdentifier(ps))
		return resetChildNode(ps, root);
	root->tok = ps->token->prev; // type identifier
	if(!prodDeclAux(ps))
		return resetChildNode(ps, root);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // params
	ps->child = root;
	return true;
}bool prodDeclMod(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodDeclModFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodDeclModFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodDeclModFact1(PState *ps){
	resetChildNode(ps, NULL);
	if(termDeclMutable(ps)){
		ps->child = newParseTree(PTT_DECL_MOD);
		((PTree*)ps->child)->tok = ps->token->prev;
		return true;
	}
	return false;
}bool prodDeclModFact2(PState *ps){
	resetChildNode(ps, NULL);
	return true; // empty modifier list
}bool prodDeclAux(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodDeclAuxFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodDeclAuxFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodDeclAuxFact1(PState *ps){
	if(!termLT(ps)) return false;
	LexicalToken *tkStart = ps->token;
	if(!prodDeclType(ps)){
		resetChildNode(ps, NULL);
		return reportParseError(ps, "PS029", "Empty type paramaters: Line %d\n", tkStart->lineNo);
	}
	PTree *root = newParseTree(PTT_DECL_PARAM);
	insertParseNodeFromList(root, PTT_DECL_PARAM, storeAndNullChildNode(ps));
	if(termColon(ps)){ // named paramater
		if(!termIdentifier(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS031", "Invalid type paramater name: Line %d\n", tkStart->lineNo);
		}
		lastRightInternalParseNode(root)->tok = ps->token->prev;
	}
	while(true){
		tkStart = ps->token;
		if(!termComma(ps)) break;
		if(!prodDeclType(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS030", "Invalid type paramater after comma: Line %d\n", tkStart->lineNo);
		}
		insertParseNodeFromList(root, PTT_DECL_PARAM, storeAndNullChildNode(ps));
		if(termColon(ps)){ // named paramater
			if(!termIdentifier(ps)){
				resetChildNode(ps, root);
				return reportParseError(ps, "PS031", "Invalid type paramater name: Line %d\n", tkStart->lineNo);
			}
			lastRightInternalParseNode(root)->tok = ps->token->prev;
		}
	}
	if(!termGT(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS032", "Type Params Missing >: Line %d\n", tkStart->lineNo);
	}
	ps->child = root;
	return true;
}bool prodDeclAuxFact2(PState *ps){
	return true;
}


// Assignment
bool prodAssign(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodAssignFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodAssignFact1(PState *ps){
	if(!termIdentifier(ps)) return false;
	resetChildNode(ps, NULL);
	ps->child = newParseTree(PTT_IDENTIFIER);
	ps->child->tok = ps->token->prev;
	prodVarValue(ps);
	PTree *root = newParseTree(PTT_ASSIGN);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	if(!termAssign(ps))
		return resetChildNode(ps, root);
	LexicalToken *tkStart = ps->token;
	root->tok = tkStart->prev;
	if(!prodExpr(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS033", "Assignment Missing Expression: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // op2
	if(!termStmtEnd(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS034", "Declaration Semicolon Missing: Line %d\n", tkStart->lineNo);
	}
	ps->child = root;
	return true;
}









