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
	ps->err = 0;
	ps->child = newChildNode(root);


	if(ps->token == NULL){
		reportError("PS001", 	"Empty Token List\n");
		return;
	}

	if(!prodExpr(ps)){
		reportError("PS002", 	"Parse is not numeric expression\n");
		return;
	}

	// is anything left
	if(ps->token->typ != LT_EOF){
		reportError("PS004", 	"Remaining Tokens Not Parsed\n");
		return;
	}

	dumpParseTree(ps->root, 0);

	freeParseTreeNode(root);
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
	PTree *root = ps->child;
	PTree *op1 = newParseTree(PTT_NOTYPE);
	ps->child = op1;
	if(!nextProd(ps)){
		freeParseTreeNode(op1);
		return resetChildNode(ps, root);
	}
	int count = 0;
	int i;
	while(true){
		bool done = false;
		for(i=0; i<num; i++){ // for each type
			if(!A[i].term(ps))
				continue;
			if(count == 0)
				insertParseNodeFromList(root, PTT_NOTYPE, op1);
			PTree *op2 = newParseTree(PTT_NOTYPE);
			ps->child = op2;
			LexicalToken *tkStart = ps->token;
			if(!nextProd(ps)){
				resetChildNode(ps, root);
				freeParseTreeNode(op2);
				return reportParseError(ps, "PS017", "Expression Missing Right Operand: Line %d", tkStart->lineNo);
			}
			insertParseNodeFromList(root, A[i].typ, op2);
			count ++;
			done = true;
			break;
		}
		if(!done)
			break;
	}
	if(count > 0){
		mergeEndParseNodes(root);
	}else{
		op1->parent = root->parent;
		freeParseTreeNode(root->child1);
		freeParseTreeNode(root->child2);
		memcpy(root, op1, sizeof(PTree));
		free(op1);
		ps->child = root; //removeParentParseNodeLeaveLChild(root); // return data-containing node
	}
	return true;
}

bool prodExprAFact1(PState *ps){
	/*PTree *root = ps->child;
	PTree *op1 = newParseTree(PTT_NOTYPE);
	ps->child = op1;
	insertParseNodeFromList(root, PTT_OR, op1);
	if(!prodExprB(ps)){
		return resetChildNode(ps, root);
	}
	int count = 0;
	while(true){
		if(!termOr(ps)) break;
		PTree *op2 = newParseTree(PTT_NOTYPE);
		ps->child = op2;
		LexicalToken *tkStart = ps->token;
		if(!prodExprB(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS017", "OR Missing Right Operand: Line %d", tkStart->lineNo);
		}
		insertParseNodeFromList(root, PTT_OR, op2);
		count ++;
	}
	if(count == 0)
		resetChildNode(ps, root);
	else
		mergeEndParseNodes(root);
	return true;*/
	ArtithmeticParseParam A[1];
	A[0].term = &termOr; A[0].typ = PTT_OR;
	return helperArithmetic(ps, &prodExprB, A, 1);

}bool prodExprB(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprBFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprBFact1(PState *ps){
	if(!prodExprC(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termXOr(ps)) break;
		if(!prodExprC(ps)){
			return reportParseError(ps, "PS018", "XOR Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;
}bool prodExprC(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprCFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprCFact1(PState *ps){
	if(!prodExprD(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termAnd(ps)) break;
		if(!prodExprD(ps)){
			return reportParseError(ps, "PS019", "AND Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;
}bool prodExprD(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprDFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprDFact1(PState *ps){
	if(!prodExprE(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termEqual(ps) && !termNotEqual(ps)){
			break;
		}
		if(!prodExprE(ps)){
			return reportParseError(ps, "PS020", "==/!= Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;
}bool prodExprE(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprEFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprEFact1(PState *ps){
	if(!prodExprF(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termLT(ps) && !termGT(ps) && !termGTE(ps) && !termLTE(ps)){
			break;
		}
		if(!prodExprF(ps)){
			return reportParseError(ps, "PS021", ">/>=/</<= Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;
}bool prodExprF(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprFFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprFFact1(PState *ps){
	if(!prodExprG(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termShiftLeft(ps) && !termShiftRight(ps)){
			break;
		}
		if(!prodExprG(ps)){
			return reportParseError(ps, "PS022", "Binary Shift Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;
}bool prodExprG(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprGFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprGFact1(PState *ps){
	/*if(!prodExprH(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termAdd(ps) && !termSub(ps)){
			break;
		}
		if(!prodExprH(ps)){
			return reportParseError(ps, "PS023", "+/- Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;*/
	ArtithmeticParseParam A[2];
	A[0].term = &termAdd; A[0].typ = PTT_ADD;
	A[1].term = &termSub; A[1].typ = PTT_SUB;
	return helperArithmetic(ps, &prodExprH, A, 2);
}bool prodExprH(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodExprHFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodExprHFact1(PState *ps){
	/*if(!prodExprI(ps)) return false;
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termMul(ps) && !termDiv(ps) && !termMod(ps)){
			break;
		}
		if(!prodExprI(ps)){
			return reportParseError(ps, "PS024", "* % / Missing Right Operand: Line %d", tkStart->lineNo);
		}
	}
	return true;*/
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
	if(prodExprJ(ps) && termExp(ps)){
		LexicalToken *tkStart = ps->token;
		if(!prodExprI(ps))
			return reportParseError(ps, "PS025", "Exponent Missing Right Operand: Line %d", tkStart->lineNo);
		return true;
	}
	return false;
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
	if(!prodExprK(ps))
		return reportParseError(ps, "PS026", "! Missing Right Operand: Line %d", tkStart->lineNo);
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
bool helperPossibleFunctionCalls(PState *ps)
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
}


// VarValue
bool prodVarValue(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodVarValueFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodVarValueFact1(PState *ps){
	if(!termIdentifier(ps)) return false;
	helperPossibleFunctionCalls(ps);
	bool arrayAccessed = false;
	while(true){ // Array Access
		LexicalToken *tkStart = ps->token;
		if(!termSqbrLeft(ps)){
			break;
		}
		if(!prodExpr(ps)){
			return reportParseError(ps, "PS005", "Tokens inside square bracket do not form expression: Line %d\n", ps->token->lineNo);
		}
		if(!termSqbrRight(ps)){
			return reportParseError(ps, "PS006", "Square bracket not closed after expression: Line %d\n", ps->token->lineNo);
		}
		arrayAccessed = true;
	}
	helperPossibleFunctionCalls(ps);
	while(true){ // Element Access [Tuples]
		LexicalToken *tkStart = ps->token;
		if(!termDot(ps)){
			break;
		}
		if(!prodVarValue(ps)){
			return reportParseError(ps, "PS011", "Element Access Not Identifier: Line %d\n", ps->token->lineNo);
		}
	}
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
	return prodVarValue(ps);
}bool prodValueFact2(PState *ps){
	return prodNumericLiteral(ps);
}bool prodValueFact3(PState *ps){
	return termStringLit(ps);
}

// NumericLiteral
bool prodNumericLiteral(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodNumericLiteralFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodNumericLiteralFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodNumericLiteralFact1(PState *ps){
	if(termFloatLit(ps)){
		ps->child->typ = PTT_FLOAT;
		ps->child->extra = ps->token->prev;
		return true;
	}
	return false;
}bool prodNumericLiteralFact2(PState *ps){
	if(termIntLit(ps)){
		ps->child->typ = PTT_INT;
		ps->child->extra = ps->token->prev;
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
	return prodExpr(ps) && prodArrayExprAux(ps);
}bool prodArrayExprFact2(PState *ps){
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
	LexicalToken *tkStart = ps->token;
	if(!(prodVarValue(ps) && termIn(ps) && prodExpr(ps))){
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
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termComma(ps)){
			break;
		}
		if(!prodExpr(ps)){
			return reportParseError(ps, "PS012", "Element Listed in Array Not Expression: Line %d\n", tkStart->lineNo);
		}
	}
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
	while(true){
		LexicalToken *tkStart = ps->token;
		if(!termComma(ps)){
			break;
		}
		if(!prodExpr(ps)){
			return reportParseError(ps, "PS015", "Element Paramater Not Expression: Line %d\n", tkStart->lineNo);
		}
	}
	return true;
}bool prodParamExprFact2(PState *ps){
	return true; // no params
}








