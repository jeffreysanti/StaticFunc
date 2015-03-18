/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  parse.c
 *  Recursive Decent Parser
 *
 */

#include "parse.h"


PTree *parse(LexicalTokenList *TL)
{
	PState *ps = preParse(TL);

	if(!prodStmtBlock(ps)){
		reportError("PS002", 	"Parse is not statement block\n");
		freeLexicalTokenList(TL);
		free(ps);
		return NULL;
	}

	ps->root = ps->child;

	// is anything left
	if(ps->token->typ != LT_EOF){
		reportError("PS004", 	"Remaining Tokens Not Parsed\n");
		freeLexicalTokenList(TL);
		free(ps);
		return NULL;
	}

	PTree *ret = ps->root;
	free(ps);
	return ret;
}

PTree *parseTypeDef(LexicalTokenList *TL)
{
	PState *ps = preParse(TL);

	if(!termIdentifier(ps)){
		freeLexicalTokenList(TL);
		free(ps);
		return NULL;
	}
	if(!prodDeclType(ps)){
		freeLexicalTokenList(TL);
		free(ps);
		return NULL;
	}
	if(!termStmtEnd(ps)){
		freeLexicalTokenList(TL);
		free(ps);
		return NULL;
	}

	// is anything left
	if(ps->token->typ != LT_EOF){
		freeLexicalTokenList(TL);
		free(ps);
		return NULL;
	}
	PTree * root = ps->child;
	free(ps);
	return root;
}

int parseTypeList(LexicalTokenList *TL, PTree *** ret)
{
	int cnt = 1;
	LexicalToken *tk = TL->first;
	while(tk->next != NULL){
		if(tk->typ == LT_OP && strcmp(tk->extra, ",")==0)
			cnt ++;
		tk = (LexicalToken*)tk->next;
	}
	*ret = calloc(cnt, sizeof(PTree*));
	int i;
	for(i=0; i<cnt; i++){
		(*ret)[i] = newParseTree(PTT_NOTYPE);
	}
	PState *ps = preParse(TL);

	if(!termIdentifier(ps)){
		freeLexicalTokenList(TL);
		free(ps);
		for(i = 0; i<cnt; i++){
			freeParseTreeNode((*ret)[i]);
		}
		free(*ret);
		return 0;
	}
	if(!prodDeclType(ps)){
		freeLexicalTokenList(TL);
		free(ps);
		for(i = 0; i<cnt; i++){
			freeParseTreeNode((*ret)[i]);
		}
		free(*ret);
		return 0;
	}
	freeParseTreeNode((*ret)[0]);
	(*ret)[0] = ps->child;
	ps->child = NULL;

	i = 1;
	while(i < cnt){
		if(!termComma(ps) || !prodDeclType(ps)){
			int x;
			for(x = 0; x<cnt; x++){
				freeParseTreeNode((*ret)[x]);
			}
			freeLexicalTokenList(TL);
			free(*ret);
			free(ps);
			return 0;
		}
		freeParseTreeNode((*ret)[i]);
		(*ret)[i] = ps->child;
		ps->child = NULL;
		i++;
	}
	if(!termStmtEnd(ps)){
		freeLexicalTokenList(TL);
		for(i=0; i<cnt; i++){
			freeParseTreeNode((*ret)[i]);
		}
		free(ps);
		free(*ret);
		return 0;
	}

	// is anything left
	if(ps->token->typ != LT_EOF){
		freeLexicalTokenList(TL);
		for(i = 0; i<cnt; i++){
			freeParseTreeNode((*ret)[i]);
		}
		free(ps);
		free(*ret);
		return 0;
	}
	free(ps);
	return cnt;
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
}bool termIf(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "if")==0);
	return advanceToken(ps, ret);
}bool termElse(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "else")==0);
	return advanceToken(ps, ret);
}bool termWhile(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "while")==0);
	return advanceToken(ps, ret);
}bool termReturn(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "return")==0);
	return advanceToken(ps, ret);
}bool termFunction(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "def")==0);
	return advanceToken(ps, ret);
}bool termLambda(PState *ps){
	bool ret = (ps->token->typ == LT_KEYWORD && strcmp(ps->token->extra, "lambda")==0);
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
			setSecondLastParseNodeToken(root, (LexicalToken*)tkStart->prev);
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
			root->tok = (LexicalToken*)tkStart->prev;
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
	root->tok = (LexicalToken*)tkStart->prev;
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
	root->tok = (LexicalToken*)ps->token->prev;
	if(!termIdentifier(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS011", "Element Access Not Identifier: Line %d\n", ps->token->lineNo);
	}
	PTree *op2 = newParseTree(PTT_IDENTIFIER); // op2 (identifier)
	op2->tok = (LexicalToken*)ps->token->prev;
	setParseNodeChild(root, op2, PC_RIGHT);
	ps->child = root;
	return prodVarValue(ps);
}bool prodVarValueFact2(PState *ps){ // op1[op2]
	if(!termSqbrLeft(ps))
		return false;
	PTree *root = newParseTree(PTT_ARR_ACCESS);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	root->tok = (LexicalToken*)ps->token->prev;
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
	PTree *root = newParseTree(PTT_PARAM_CONT);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	root->tok = (LexicalToken*)ps->token->prev;
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
			ret = prodValueFact4(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodValueFact1(PState *ps){
	return prodNumericLiteral(ps);
}bool prodValueFact2(PState *ps){
	if(termStringLit(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_STRING);
		ps->child->tok = (LexicalToken*)ps->token->prev;
		return true;
	}
	return false;
}bool prodValueFact3(PState *ps){
	if(termIdentifier(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_IDENTIFIER);
		ps->child->tok = (LexicalToken*)ps->token->prev;
		return prodVarValue(ps);
	}
	return false;
}bool prodValueFact4(PState *ps){ // Lambdas :D
	LexicalToken *tkStart = ps->token;
	if(termLambda(ps)){
		if(!termParenLeft(ps))
			return reportParseError(ps, "PS067", "Lambda Expected Parens: Line %d\n", tkStart->lineNo);
		if(!prodDeclType(ps)){
			return reportParseError(ps, "PS070", "Lambda Declaration Expected Type: Line %d\n", tkStart->lineNo);
		}
		PTree *rootType = newParseTree(PTT_FUNCTION_TYPE);
		setParseNodeChild(rootType, storeAndNullChildNode(ps), PC_LEFT); // rettype
		PTree *root = newParseTree(PTT_LAMBDA);
		setParseNodeChild(root, rootType, PC_LEFT);
		root->tok = tkStart;
		if(!termParenLeft(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS071", "Lambda Declaration Expected Internal Left Paren: Line %d\n", tkStart->lineNo);
		}
		if(prodDecl(ps)){
			PTree *rootParams = newParseTree(PTT_PARAM_CONT);
			insertParseNodeFromList(rootParams, PTT_PARAM_CONT, storeAndNullChildNode(ps)); // first param
			setParseNodeChild(rootType, rootParams, PC_RIGHT); // params
			while(true){
				LexicalToken *tkStart = ps->token;
				if(!termComma(ps)){
					break;
				}
				if(!prodDecl(ps)){
					resetChildNode(ps, root);
					return reportParseError(ps, "PS072", "Lambda Paramater Not Declaration: Line %d\n", tkStart->lineNo);
				}
				insertParseNodeFromList(rootParams, PTT_PARAM_CONT, storeAndNullChildNode(ps)); // first param
			}
		}
		if(!termParenRight(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS073", "Lambda Declaration Expected Internal Closing Paren: Line %d\n", tkStart->lineNo);
		}
		if(!prodStmt(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS074", "Lambda Declaration Expected Statement: Line %d\n", tkStart->lineNo);
		}
		setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // body
		if(!termParenRight(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS069", "Lambda Expected Closing Paren: Line %d\n", tkStart->lineNo);
		}
		ps->child = root;
		return true;
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
		ps->child->tok = (LexicalToken*)ps->token->prev;
		return true;
	}
	return false;
}bool prodNumericLiteralFact2(PState *ps){
	if(termIntLit(ps)){
		resetChildNode(ps, NULL);
		ps->child = newParseTree(PTT_INT);
		ps->child->tok = (LexicalToken*)ps->token->prev;
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
	LexicalToken *tkStart = ps->token;
	if(termColon(ps)){
		PTree *lhs = storeAndNullChildNode(ps);
		if(!prodExpr(ps)){
			resetChildNode(ps, lhs);
			return reportParseError(ps, "PS070", "List Found Colon, but no following expr: Line %d\n", tkStart->lineNo);
		}
		PTree *rhs = storeAndNullChildNode(ps);
		ps->child = newParseTree(PTT_ARRAY_ELM_PAIR);
		((PTree*)ps->child)->child1 = (void*)lhs;
		((PTree*)ps->child)->child2 = (void*)rhs;
	}
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
	LexicalToken *tkStart = ps->token;
	PTree *opBuild = extractIndependentLeftParseNodeLeaveChild(ps->child);
	resetChildNode(ps, NULL);
	if(!prodDecl(ps)){
		resetChildNode(ps, opBuild);
		return reportParseError(ps, "PS050", "List Comprehension Expecting declaration: Line %d\n", tkStart->lineNo);
	}
	PTree *rootIn = newParseTree(PTT_ARRAY_COMP_IN);
	setParseNodeChild(rootIn, storeAndNullChildNode(ps), PC_LEFT); // identifer variable
	if(!termIn(ps)){
		resetChildNode(ps, rootIn);
		resetChildNode(ps, opBuild);
		return reportParseError(ps, "PS051", "List Comprehension Expecting in: Line %d\n", tkStart->lineNo);
	}
	if(!prodExpr(ps)){
		resetChildNode(ps, rootIn);
		resetChildNode(ps, opBuild);
		return reportParseError(ps, "PS052", "List Comprehension Missing Expression: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(rootIn, storeAndNullChildNode(ps), PC_RIGHT); // range expression
	PTree *rootOut = newParseTree(PTT_ARRAY_COMP_OUT);
	setParseNodeChild(rootOut, opBuild, PC_LEFT); // build expression
	if(termWhere(ps)){
		if(!prodExpr(ps)){
			resetChildNode(ps, rootIn);
			resetChildNode(ps, rootOut);
			return reportParseError(ps, "PS053", "List Comprehension Missing Expression After Where: Line %d\n", tkStart->lineNo);
		}
		setParseNodeChild(rootOut, storeAndNullChildNode(ps), PC_RIGHT); // range expression
	}
	PTree *root = newParseTree(PTT_ARRAY_COMP);
	setParseNodeChild(root, rootIn, PC_LEFT);
	setParseNodeChild(root, rootOut, PC_RIGHT);
	ps->child = root;
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
		tkStart = ps->token;
		if(termColon(ps)){
			PTree *lhs = storeAndNullChildNode(ps);
			if(!prodExpr(ps)){
				resetChildNode(ps, lhs);
				return reportParseError(ps, "PS070", "List Found Colon, but no following expr: Line %d\n", tkStart->lineNo);
			}
			PTree *rhs = storeAndNullChildNode(ps);
			ps->child = newParseTree(PTT_ARRAY_ELM_PAIR);
			((PTree*)ps->child)->child1 = (void*)lhs;
			((PTree*)ps->child)->child2 = (void*)rhs;
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
			ret = prodStmtFact5(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact6(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact7(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact8(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact9(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtFact10(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodStmtFact1(PState *ps){
	LexicalToken *tkStart = ps->token;
	if(termBraceLeft(ps) && prodStmtBlock(ps) && termBraceRight(ps)){
		ps->child->tok = tkStart;
		return true;
	}
	return false;
}bool prodStmtFact2(PState *ps){
	return prodFuncDefn(ps);
}bool prodStmtFact3(PState *ps){
	LexicalToken *tkStart = ps->token;
	if(prodDecl(ps)){
		if(!termStmtEnd(ps)){
			resetChildNode(ps, NULL);
			return reportParseError(ps, "PS028", "Declaration Semicolon Missing: Line %d\n", tkStart->lineNo);
		}
		return true;
	}
	return false;
}bool prodStmtFact4(PState *ps){
	return prodAssign(ps);
}bool prodStmtFact5(PState *ps){
	return prodCond(ps);
}bool prodStmtFact6(PState *ps){
	return prodWhile(ps);
}bool prodStmtFact7(PState *ps){
	return prodFor(ps);
}bool prodStmtFact8(PState *ps){
	if(termReturn(ps)) return prodReturnVal(ps);
	return false;
}bool prodStmtFact9(PState *ps){
	return prodExpr(ps) && termStmtEnd(ps);
}bool prodStmtFact10(PState *ps){
	return termStmtEnd(ps);
}

// Statement Block
bool prodStmtBlock(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodStmtBlockFact1(ps); if(!ret) ps->token = start; else return true;
			ret = prodStmtBlockFact2(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodStmtBlockFact1(PState *ps){
	if(!prodStmt(ps)) return false;
	PTree *root = newParseTree(PTT_STMTBLOCK);
	root->tok = ps->token;
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
}bool prodStmtBlockFact2(PState *ps){
	PTree *root = newParseTree(PTT_STMTBLOCK);
	root->tok = ps->token;
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
	root->tok = (LexicalToken*)ps->token->prev; // name of declaration
	LexicalToken *tkStart = (LexicalToken*)ps->token->prev;
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
	root->tok = (LexicalToken*)ps->token->prev; // type identifier
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
		((PTree*)ps->child)->tok = (LexicalToken*)ps->token->prev;
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
		lastRightInternalParseNode(root)->tok = (LexicalToken*)ps->token->prev;
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
			lastRightInternalParseNode(root)->tok = (LexicalToken*)ps->token->prev;
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
	ps->child->tok = (LexicalToken*)ps->token->prev;
	prodVarValue(ps);
	PTree *root = newParseTree(PTT_ASSIGN);
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	if(!termAssign(ps))
		return resetChildNode(ps, root);
	LexicalToken *tkStart = ps->token;
	root->tok = (LexicalToken*)tkStart->prev;
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


// Conditionals
bool prodCond(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodCondFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodCondFact1(PState *ps){
	if(!termIf(ps)) return false;
	LexicalToken *tkStart = ps->token;
	if(!termParenLeft(ps)){
		return reportParseError(ps, "PS035", "Conditional Requires Parens: Line %d\n", tkStart->lineNo);
	}
	if(!prodExpr(ps)){
		return reportParseError(ps, "PS036", "Conditional Missing Expression: Line %d\n", tkStart->lineNo);
	}
	PTree *root = newParseTree(PTT_IF);
	root->tok = (LexicalToken*)tkStart->prev;
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	if(!termParenRight(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS037", "Conditional Closing Paren Missing: Line %d\n", tkStart->lineNo);
	}
	if(!prodStmt(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS038", "Conditional Missing Following Stmt: Line %d\n", tkStart->lineNo);
	}
	PTree *opTrue = storeAndNullChildNode(ps);
	if(termElse(ps)){
		tkStart = ps->token;
		if(!prodStmt(ps)){
			resetChildNode(ps, root);
			return reportParseError(ps, "PS039", "Conditional Invalid Else Stmt: Line %d\n", tkStart->lineNo);
		}
		PTree *opFalse = storeAndNullChildNode(ps);
		PTree *rootSwitch = newParseTree(PTT_IFELSE_SWITCH);
		setParseNodeChild(rootSwitch, opTrue, PC_LEFT); // true
		setParseNodeChild(rootSwitch, opFalse, PC_RIGHT); // true
		setParseNodeChild(root, rootSwitch, PC_RIGHT); // op2
	}else{
		setParseNodeChild(root, opTrue, PC_RIGHT); // op2
	}
	ps->child = root;
	return true;
}


bool prodWhile(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodWhileFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodWhileFact1(PState *ps){
	if(!termWhile(ps)) return false;
	LexicalToken *tkStart = ps->token;
	if(!termParenLeft(ps)){
		return reportParseError(ps, "PS040", "While Requires Parens: Line %d\n", tkStart->lineNo);
	}
	if(!prodExpr(ps)){
		return reportParseError(ps, "PS041", "While Missing Expression: Line %d\n", tkStart->lineNo);
	}
	PTree *root = newParseTree(PTT_WHILE);
	root->tok = (LexicalToken*)tkStart->prev;
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT); // op1
	if(!termParenRight(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS042", "While Closing Paren Missing: Line %d\n", tkStart->lineNo);
	}
	if(!prodStmt(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS043", "While Missing Following Stmt: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // op2
	ps->child = root;
	return true;
}


bool prodFor(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodForFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodForFact1(PState *ps){
	if(!termFor(ps)) return false;
	LexicalToken *tkStart = ps->token;
	if(!termParenLeft(ps)){
		return reportParseError(ps, "PS044", "For Requires Parens: Line %d\n", tkStart->lineNo);
	}
	PTree *rootCond = newParseTree(PTT_FOR_COND);
	if(!prodDecl(ps)){
		return reportParseError(ps, "PS045", "For Expecting declaration: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(rootCond, storeAndNullChildNode(ps), PC_LEFT); // identifer variable
	if(!termIn(ps)){
		resetChildNode(ps, rootCond);
		return reportParseError(ps, "PS046", "For Expecting in: Line %d\n", tkStart->lineNo);
	}
	if(!prodExpr(ps)){
		resetChildNode(ps, rootCond);
		return reportParseError(ps, "PS047", "For Missing Expression: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(rootCond, storeAndNullChildNode(ps), PC_RIGHT); // range expression
	PTree *root = newParseTree(PTT_FOR);
	root->tok = (LexicalToken*)tkStart->prev;
	setParseNodeChild(root, rootCond, PC_LEFT);
	root->tok = (LexicalToken*)tkStart->prev;
	if(!termParenRight(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS048", "For Closing Paren Missing: Line %d\n", tkStart->lineNo);
	}
	if(!prodStmt(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS049", "For Missing Following Stmt: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // op2
	ps->child = root;
	return true;
}


// return value
bool prodReturnVal(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodReturnValFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodReturnValFact1(PState *ps){
	LexicalToken *tkStart = (LexicalToken*)ps->token->prev;
	if(prodExpr(ps)){
		if(!termStmtEnd(ps)){
			resetChildNode(ps, NULL);
			return reportParseError(ps, "PS060", "Return Semicolon Missing: Line %d\n", tkStart->lineNo);
		}
		PTree *root = newParseTree(PTT_RETURN);
		root->tok = tkStart;
		setParseNodeChild(root, storeAndNullChildNode(ps), PC_LEFT);
		ps->child = root;
		return true;
	}
	return false;
}


bool prodFuncDefn(PState *ps){
	LexicalToken *start = ps->token;
	bool 	ret = prodFuncDefnFact1(ps); if(!ret) ps->token = start; else return true;
	return false;
}bool prodFuncDefnFact1(PState *ps){
	LexicalToken *tkStart = ps->token;
	if(!termFunction(ps)) return false;
	if(!prodDeclType(ps)){
		resetChildNode(ps, NULL);
		return reportParseError(ps, "PS061", "Function Declaration Expected Type: Line %d\n", tkStart->lineNo);
	}
	PTree *rootType = newParseTree(PTT_FUNCTION_TYPE);
	setParseNodeChild(rootType, storeAndNullChildNode(ps), PC_LEFT); // rettype
	if(!termIdentifier(ps)){
		resetChildNode(ps, rootType);
		return reportParseError(ps, "PS062", "Function Declaration Expected Identifier Name: Line %d\n", tkStart->lineNo);
	}
	PTree *root = newParseTree(PTT_FUNCTION);
	root->tok = (LexicalToken*)ps->token->prev;
	setParseNodeChild(root, rootType, PC_LEFT);
	if(!termParenLeft(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS063", "Function Declaration Expected Left Paren: Line %d\n", tkStart->lineNo);
	}
	if(prodDecl(ps)){
		PTree *rootParams = newParseTree(PTT_PARAM_CONT);
		insertParseNodeFromList(rootParams, PTT_PARAM_CONT, storeAndNullChildNode(ps)); // first param
		setParseNodeChild(rootType, rootParams, PC_RIGHT); // params
		while(true){
			LexicalToken *tkStart = ps->token;
			if(!termComma(ps)){
				break;
			}
			if(!prodDecl(ps)){
				resetChildNode(ps, root);
				return reportParseError(ps, "PS064", "Function Paramater Not Declaration: Line %d\n", tkStart->lineNo);
			}
			insertParseNodeFromList(rootParams, PTT_PARAM_CONT, storeAndNullChildNode(ps)); // first param
		}
	}
	if(!termParenRight(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS065", "Function Declaration Expected Closing Paren: Line %d\n", tkStart->lineNo);
	}
	if(!prodStmt(ps)){
		resetChildNode(ps, root);
		return reportParseError(ps, "PS066", "Function Declaration Expected Statement: Line %d\n", tkStart->lineNo);
	}
	setParseNodeChild(root, storeAndNullChildNode(ps), PC_RIGHT); // body
	ps->child = root;
	return true;
}




