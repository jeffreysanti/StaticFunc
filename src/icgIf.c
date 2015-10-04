/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgIf.c
 *  Intermediate Code Generator : Handles If/Else Conditional
 *
 */

#include "icg.h"

ICGElm *icGenGeneralCondition(PTree *cond, ICGElm *prev, char *lblFalseBranch){
	prev = icGen(cond, prev);
	if(cond->typ == PTT_EQUAL){
		ICGElm *jnzPastTrue = newICGElm(prev, ICG_JNZ, typeToICGDataType(cond->finalType), cond);
		jnzPastTrue->op1 = newOpCopyData(prev->result->typ, prev->result->data);
		jnzPastTrue->result = newOpCopyData(ICGO_LABEL, lblFalseBranch);
		return jnzPastTrue;
	}

	ICGElm *jnzPastTrue = newICGElm(prev, ICG_JZ, typeToICGDataType(cond->finalType), cond);
	jnzPastTrue->op1 = newOpCopyData(prev->result->typ, prev->result->data);
	jnzPastTrue->result = newOpCopyData(ICGO_LABEL, lblFalseBranch);
	return jnzPastTrue;
}

ICGElm * icGenEquality(PTree *root, ICGElm *prev){
	if(root->child2 == NULL){ // object comparison
		PTree *check = (PTree*)root->child1;
		ICGElm *lhs = icGen((PTree*)check->child1, prev);
		ICGElm *rhs = icGen((PTree*)check->child2, lhs);
		prev = rhs;
		
		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));
		ICGElmOp *op1 = newOpCopyData(lhs->result->typ, lhs->result->data);
		ICGElmOp *op2 = newOpCopyData(rhs->result->typ, rhs->result->data);
		
		prev = newICGElm(prev, ICG_COMPOBJ, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;
		return prev;
	}else{ // numerical
		ICGElm *lhs = icGen((PTree*)root->child1, prev);
		ICGElm *rhs = icGen((PTree*)root->child2, lhs);
		prev = rhs;

		char *tempVar = newTempVariable(root->finalType);
		ICGElmOp *res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));
		ICGElmOp *op1 = newOpCopyData(lhs->result->typ, lhs->result->data);
		ICGElmOp *op2 = newOpCopyData(rhs->result->typ, rhs->result->data);

		prev = newICGElm(prev, ICG_SUB, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;
		return prev;
	}
}

ICGElm * icGenIf(PTree *root, ICGElm *prev){
	// step 1: Evaluate Condition
	PTree *cond = (PTree*)root->child1;
	char *lblPastTrue = newLabel("IfElse");
	prev = icGenGeneralCondition(cond, prev, lblPastTrue);

	// now gen accepting condition
	PTree *pred = (PTree*)root->child2;
	if(pred->typ == PTT_IFELSE_SWITCH){ // has an else condition
		PTree *predTrue = (PTree*)pred->child1;
		PTree *predFalse = (PTree*)pred->child2;
		char *lblPastFalse = newLabel("IfElseEnd");
		char *lblFakeIfStart = newLabel("IfTrue");

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if begin label
		prev->result = newOp(ICGO_LABEL, lblFakeIfStart);

		prev = icGenBlock(predTrue, prev); // true branch

		prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root); // jump to end if statemnt
		prev->result = newOp(ICGO_LABEL, lblPastFalse);

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // else label
		prev->result = newOp(ICGO_LABEL, lblPastTrue);

		prev = icGenBlock(predFalse, prev); // false branch

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // if end label
		prev->result = newOpCopyData(ICGO_LABEL, lblPastFalse);
	}else{ // falls through
		char *lblFakeIfEnd = newLabel("IfElseEnd");
		char *lblFakeIfStart = newLabel("IfTrue");

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if begin label
		prev->result = newOp(ICGO_LABEL, lblFakeIfStart);

		prev = icGenBlock(pred, prev); // true branch

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // else label
		prev->result = newOp(ICGO_LABEL, lblPastTrue);

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
		prev->result = newOp(ICGO_LABEL, lblFakeIfEnd);
	}
	return prev;
}

extern void icGenJump_print(ICGElm *elm, FILE* f){
	if(elm->typ == ICG_JMP){
		fprintf(f, "jmp %s", elm->result->data);
	}else if(elm->typ == ICG_JNZ){
		if(elm->op1->typ == ICGO_NUMERICLIT){
			fprintf(f, "jnz %s, %s", elm->result->data, elm->op1->data);
		}else{
			fprintf(f, "jnz %s, $%s", elm->result->data, elm->op1->data);
		}
	}else if(elm->typ == ICG_JZ){
		if(elm->op1->typ == ICGO_NUMERICLIT){
			fprintf(f, "jz %s, %s", elm->result->data, elm->op1->data);
		}else{
			fprintf(f, "jz %s, $%s", elm->result->data, elm->op1->data);
		}
	}
}

ICGElm * icGenCompObj_print(ICGElm *elm, FILE* f){
	fprintf(f, "compobj");
	fprintf(f, " $%s, ", elm->result->data);
	if(elm->op1->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s, ", elm->op1->data);
	}else{
		fprintf(f, "$%s, ", elm->op1->data);
	}
	if(elm->op2->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", elm->op2->data);
	}else{
		fprintf(f, "$%s", elm->op2->data);
	}
}
