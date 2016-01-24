/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgIf.c
 *  Intermediate Code Generator : Handles If/Else Conditional, and Loops
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

ICGElm * icGenWhile(PTree *root, ICGElm *prev){
	// step 1: Evaluate Condition
	PTree *cond = (PTree*)root->child1;
	char *lblExitWhile = newLabel("WhileLoopExit");
	char *lblWhileMarker = newLabel("WhileLoopTest");

	prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // test label
	prev->result = newOp(ICGO_LABEL, lblWhileMarker);

	prev = icGenGeneralCondition(cond, prev, lblExitWhile);

	// now gen accepting state
	PTree *pred = (PTree*)root->child2;

	prev = icGenBlock(pred, prev); // true branch

	// reloop
	prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root);
	prev->result = newOpCopyData(ICGO_LABEL, lblWhileMarker);

	// exit label
	prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
	prev->result = newOp(ICGO_LABEL, lblExitWhile);
	return prev;
}

ICGElm * icGenFor(PTree *root, ICGElm *prev){
	char *lblForEnd = newLabel("ForLoopExit");

	// step 1: Initialize holder variable & iteratable variable
	prev = icGenDecl(((PTree*)root->child1)->child1, prev);
	ICGElm *holderVar = prev;
	prev = icGen(((PTree*)root->child1)->child2, prev);
	ICGElm *iteratableVar = prev;

	// step 2: prepare iterator
	char *iteratorVar = newTempVariable(newBasicType(TB_NATIVE_VOID));
	prev = newICGElm(prev, ICG_ITER_INIT, ICGDT_NONE, root); // initalize iterator
	prev->result = newOp(ICGO_OBJREF, getSymbolUniqueName(iteratorVar));
	prev->op1 = newOpCopyData(iteratableVar->result->typ, iteratableVar->result->data);

	// step 3: loop marker
	char *lblForMarker = newLabel("ForLoopMarker");
	prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // test label
	prev->result = newOp(ICGO_LABEL, lblForMarker);

	// step 4: get next data
	prev = newICGElm(prev, ICG_ITER_NEXT, ICGDT_NONE, root);
	prev->result = newOpCopyData(holderVar->result->typ, holderVar->result->data);
	prev->op1 = newOp(ICGO_OBJREF, getSymbolUniqueName(iteratorVar));
	prev->op2 = newOpCopyData(ICGO_LABEL, lblForEnd);

	// step 6: run code & reloop
	prev = icGenBlock(root->child2, prev);
	prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root);
	prev->result = newOpCopyData(ICGO_LABEL, lblForMarker);

	// step 6: exit marker & close iterator
	prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
	prev->result = newOp(ICGO_LABEL, lblForEnd);
	prev = newICGElm(prev, ICG_ITER_CLOSE, ICGDT_NONE, root); // fake if end label
	prev->result = newOp(ICGO_OBJREF, getSymbolUniqueName(iteratorVar));

	return prev;
}

ICGElm * icGenArrayComp(PTree *root, ICGElm *prev){
  // prepare output variable:
  char *outVar = newTempVariable(root->finalType);
  ICGElmOp *outRes;
  if(root->finalType.base == TB_VECTOR){
    outRes = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(outVar));
    Type childType = ((Type*)root->finalType.children)[0];
    ICGElmOp *op1 = bitSizeOp(childType);
    ICGElmOp *op2 = newOpInt(ICGO_NUMERICLIT, 0);
    prev = newICGElm(prev, ICG_NEWVEC, typeToICGDataType(root->finalType), root);
    prev->result = outRes;
    prev->op1 = op1;
    prev->op2 = op2;
  }else{ // dict
    outRes = newOp(ICGO_OBJREFNEW, getSymbolUniqueName(outVar));
    Type childTypeKey = ((Type*)root->finalType.children)[0];
    ICGElmOp *op1 = bitSizeOp(childTypeKey);
    Type childTypeVal = ((Type*)root->finalType.children)[1];
    ICGElmOp *op2 = bitSizeOp(childTypeVal);
    ICGElmOp *op3 = newOpInt(ICGO_NUMERICLIT, 0);
    prev = newICGElm(prev, ICG_NEWDICT, typeToICGDataType(root->finalType), root);
    prev->result = outRes;
    prev->op1 = op1;
    prev->op2 = op2;
    prev->op3 = op3;
  }
  char *lblCompEnd = newLabel("CompLoopExit");

  
  // step 1: Initialize holder variable & iteratable variable
  prev = icGenDecl(((PTree*)root->child1)->child1, prev);
  ICGElm *holderVar = prev;
  prev = icGen(((PTree*)root->child1)->child2, prev);
  ICGElm *iteratableVar = prev;

  // step 2: prepare iterator
  char *iteratorVar = newTempVariable(newBasicType(TB_NATIVE_VOID));
  prev = newICGElm(prev, ICG_ITER_INIT, ICGDT_NONE, root); // initalize iterator
  prev->result = newOp(ICGO_OBJREF, getSymbolUniqueName(iteratorVar));
  prev->op1 = newOpCopyData(iteratableVar->result->typ, iteratableVar->result->data);

  // step 3: loop marker
  char *lblCompMarker = newLabel("CompLoopMarker");
  prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // test label
  prev->result = newOp(ICGO_LABEL, lblCompMarker);

  // step 4: get next data
  prev = newICGElm(prev, ICG_ITER_NEXT, ICGDT_NONE, root);
  prev->result = newOpCopyData(holderVar->result->typ, holderVar->result->data);
  prev->op1 = newOp(ICGO_OBJREF, getSymbolUniqueName(iteratorVar));
  prev->op2 = newOpCopyData(ICGO_LABEL, lblCompEnd);

  // step 5: check whether condition holds for each element
  if(((PTree*)root->child2)->child2 != NULL){ // conditional
    prev = icGenGeneralCondition(((PTree*)root->child2)->child2, prev, lblCompMarker);
  }

  // step 6: push element into object
  if(((PTree*)((PTree*)root->child2)->child1)->typ == PTT_ARRAY_ELM_PAIR){ // dict
    // TODO
    fprintf(stderr, "Dict Comprehension ICG Not Implemented Yet!!!\n");
  }else{ // vector
    prev = icGen(((PTree*)root->child2)->child1, prev);
    ICGElmOp *op2 = newOpCopyData(prev->result->typ, prev->result->data);
    ICGElmOp *op1 = newOpCopyData(outRes->typ, outRes->data);
    prev = newICGElm(prev, ICG_VPUSH, typeToICGDataType(root->finalType), root);
    prev->op1 = op1;
    prev->op2 = op2;
  }

  // step 7: reloop
  prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root);
  prev->result = newOpCopyData(ICGO_LABEL, lblCompMarker);

  // step 8: exit marker & close iterator
  prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
  prev->result = newOp(ICGO_LABEL, lblCompEnd);
  prev = newICGElm(prev, ICG_ITER_CLOSE, ICGDT_NONE, root); // fake if end label
  prev->result = newOp(ICGO_OBJREF, getSymbolUniqueName(iteratorVar));

  // to present new object to next icg
  prev = newICGElm(prev, ICG_IDENT, typeToICGDataType(root->finalType), root);
  prev->result = newOpCopyData(outRes->typ, outRes->data);
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

void icGenFor_print(ICGElm *elm, FILE* f){
	if(elm->typ == ICG_ITER_INIT){
		fprintf(f, "iteri $%s, $%s", elm->result->data, elm->op1->data);
	}else if(elm->typ == ICG_ITER_CLOSE){
		fprintf(f, "iterf $%s", elm->result->data);
	}else if(elm->typ == ICG_ITER_NEXT){
		fprintf(f, "itern $%s, $%s, %s", elm->result->data, elm->op1->data, elm->op2->data);
	}
}
