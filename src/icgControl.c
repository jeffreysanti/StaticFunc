/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgIf.c
 *  Intermediate Code Generator : Handles If/Else Conditional, and Loops
 *
 */

#include "icg.h"

extern ICGElm * icGenDecl(PTree *root, ICGElm *prev);

ICGElm *icGenGeneralCondition(PTree *cond, ICGElm *prev, char *lblFalseBranch){
	prev = icGen(cond, prev);
	if(cond->typ == PTT_EQUAL){
		ICGElm *jnzPastTrue = newICGElm(prev, ICG_JNZ, typeToICGDataType(cond->finalType), cond);
		jnzPastTrue->op1 = newOpCopy(prev->result);
		jnzPastTrue->result = newOpLabel_c(lblFalseBranch);
		return jnzPastTrue;
	}

	ICGElm *jnzPastTrue = newICGElm(prev, ICG_JZ, typeToICGDataType(cond->finalType), cond);
	jnzPastTrue->op1 = newOpCopy(prev->result);
	jnzPastTrue->result = newOpLabel_c(lblFalseBranch);
	return jnzPastTrue;
}

ICGElm * icGenEquality(PTree *root, ICGElm *prev){
	if(root->child2 == NULL){ // object comparison
		PTree *check = (PTree*)root->child1;
		ICGElm *lhs = icGen((PTree*)check->child1, prev);
		ICGElm *rhs = icGen((PTree*)check->child2, lhs);
		prev = rhs;

		Variable *tempVar = defineVariable(NULL, root->finalType);
		ICGElmOp *res = newOp(ICGO_REG, tempVar);
		ICGElmOp *op1 = newOpCopy(lhs->result);
		ICGElmOp *op2 = newOpCopy(rhs->result);

		prev = newICGElm(prev, ICG_COMPOBJ, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;
		return prev;
	}else{ // numerical
		ICGElm *lhs = icGen((PTree*)root->child1, prev);
		ICGElm *rhs = icGen((PTree*)root->child2, lhs);
		prev = rhs;

		Variable *tempVar = defineVariable(NULL, root->finalType);
		ICGElmOp *res = newOp(ICGO_REG, tempVar);
		ICGElmOp *op1 = newOpCopy(lhs->result);
		ICGElmOp *op2 = newOpCopy(rhs->result);

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
		prev->result = newOp(ICGO_LABEL, (Variable*)lblFakeIfStart);

		enterNewScope();
		prev = icGenBlock(predTrue, prev); // true branch
		prev = derefScope(prev); exitScope();
		
		
		prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root); // jump to end if statemnt
		prev->result = newOp(ICGO_LABEL, (Variable*)lblPastFalse);

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // else label
		prev->result = newOp(ICGO_LABEL, (Variable*)lblPastTrue);

		enterNewScope();
		prev = icGenBlock(predFalse, prev); // false branch
		prev = derefScope(prev); exitScope();
		
		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // if end label
		prev->result = newOpLabel_c(lblPastFalse);
	}else{ // falls through
		char *lblFakeIfEnd = newLabel("IfElseEnd");
		char *lblFakeIfStart = newLabel("IfTrue");

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if begin label
		prev->result = newOp(ICGO_LABEL, (Variable*)lblFakeIfStart);

		enterNewScope();
		prev = icGenBlock(pred, prev); // true branch
		prev = derefScope(prev); exitScope();
		
		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // else label
		prev->result = newOp(ICGO_LABEL, (Variable*)lblPastTrue);

		prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
		prev->result = newOp(ICGO_LABEL, (Variable*)lblFakeIfEnd);
	}
	return prev;
}

ICGElm * icGenWhile(PTree *root, ICGElm *prev){
  // step 1: Evaluate Condition
  PTree *cond = (PTree*)root->child1;
  char *lblExitWhile = newLabel("WhileLoopExit");
  char *lblWhileMarker = newLabel("WhileLoopTest");

  prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // test label
  prev->result = newOp(ICGO_LABEL, (Variable*)lblWhileMarker);

  prev = icGenGeneralCondition(cond, prev, lblExitWhile);

  
  // now gen accepting state
  PTree *pred = (PTree*)root->child2;

  enterNewScope(); // SCOPE OF LOOP BLOCK
  prev = icGenBlock(pred, prev); // true branch
  prev = derefScope(prev); exitScope(); //  SCOPE OF LOOP BLOCK
  
  // reloop
  prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root);
  prev->result = newOpLabel_c(lblWhileMarker);

  // exit label
  prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
  prev->result = newOp(ICGO_LABEL, (Variable*)lblExitWhile);

  return prev;
}

ICGElm * icGenFor(PTree *root, ICGElm *prev){
	char *lblForEnd = newLabel("ForLoopExit");

	// step 1: Initialize holder variable & iteratable variable
	prev = icGenDecl((PTree*)((PTree*)root->child1)->child1, prev);
	ICGElm *holderVar = prev;
	prev = icGen((PTree*)((PTree*)root->child1)->child2, prev);
	ICGElm *iteratableVar = prev;

	// step 2: prepare iterator
	Variable *iteratorVar = defineVariable(NULL, newBasicType(TB_NATIVE_VOID));
	iteratorVar->disposedTemp = true;
	prev = newICGElm(prev, ICG_ITER_INIT, ICGDT_NONE, root); // initalize iterator
	prev->result = newOp(ICGO_REG, iteratorVar);
	prev->op1 = newOpCopy(iteratableVar->result);

	// step 3: loop marker
	char *lblForMarker = newLabel("ForLoopMarker");
	prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // test label
	prev->result = newOp(ICGO_LABEL, (Variable*)lblForMarker);

	// step 4: get next data
	prev = newICGElm(prev, ICG_ITER_NEXT, ICGDT_NONE, root);
	prev->result = newOpCopy(holderVar->result);
	prev->op1 = newOp(ICGO_REG, iteratorVar);
	prev->op2 = newOpLabel_c(lblForEnd);

	// step 6: run code & reloop
	prev = icGenBlock((PTree*)root->child2, prev);
	prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root);
	prev->result = newOpLabel_c(lblForMarker);

	// step 6: exit marker & close iterator
	prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
	prev->result = newOp(ICGO_LABEL, (Variable*)lblForEnd);
	prev = newICGElm(prev, ICG_ITER_CLOSE, ICGDT_NONE, root); // fake if end label
	prev->result = newOp(ICGO_REG, iteratorVar);

	return prev;
}

ICGElm * icGenArrayComp(PTree *root, ICGElm *prev){
  // prepare output variable:
  Variable *outVar = defineVariable(NULL, root->finalType);

  enterNewScope(); // SCOPE OF COMP GENERATION
  
  ICGElmOp *outRes;
  if(root->finalType.base == TB_VECTOR){
    outRes = newOp(ICGO_OBJREFNEW, outVar);
    Type childType = ((Type*)root->finalType.children)[0];
    ICGElmOp *op1 = bitSizeOp(childType);
    ICGElmOp *op2 = newOpInt(0);
    prev = newICGElm(prev, ICG_NEWVEC, typeToICGDataType(root->finalType), root);
    prev->result = outRes;
    prev->op1 = op1;
    prev->op2 = op2;
  }else{ // dict
    outRes = newOp(ICGO_OBJREFNEW, outVar);
    Type childTypeKey = ((Type*)root->finalType.children)[0];
    ICGElmOp *op1 = bitSizeOp(childTypeKey);
    Type childTypeVal = ((Type*)root->finalType.children)[1];
    ICGElmOp *op2 = bitSizeOp(childTypeVal);
    ICGElmOp *op3 = newOpInt(0);
    prev = newICGElm(prev, ICG_NEWDICT, typeToICGDataType(root->finalType), root);
    prev->result = outRes;
    prev->op1 = op1;
    prev->op2 = op2;
    prev->op3 = op3;
  }
  char *lblCompEnd = newLabel("CompLoopExit");

  
  // step 1: Initialize holder variable & iteratable variable
  prev = icGenDecl((PTree*)((PTree*)root->child1)->child1, prev);
  ICGElm *holderVar = prev;
  prev = icGen((PTree*)((PTree*)root->child1)->child2, prev);
  ICGElm *iteratableVar = prev;

  // clear the holder object if it's not a primative
  if(holderVar->result->typ == ICGO_REG || holderVar->result->typ == ICGO_OBJREFNEW){
    ICGElmOp *o = newOpCopy(holderVar->result);
    prev = newICGElm(prev, ICG_DR, typeToICGDataType(((Variable*)o->data)->sig), NULL);
      prev->result = o;
  }

  // step 2: prepare iterator
  Variable *iteratorVar = defineVariable(NULL, newBasicType(TB_NATIVE_VOID));
  iteratorVar->disposedTemp = true;
  prev = newICGElm(prev, ICG_ITER_INIT, ICGDT_NONE, root); // initalize iterator
  prev->result = newOp(ICGO_REG, iteratorVar);
  prev->op1 = newOpCopy(iteratableVar->result);

  // step 3: loop marker
  char *lblCompMarker = newLabel("CompLoopMarker");
  prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // test label
  prev->result = newOp(ICGO_LABEL, (Variable*)lblCompMarker);

  // step 4: get next data
  prev = newICGElm(prev, ICG_ITER_NEXT, ICGDT_NONE, root);
  prev->result = newOpCopy(holderVar->result);
  prev->op1 = newOp(ICGO_REG, iteratorVar);
  prev->op2 = newOpLabel_c(lblCompEnd);

  // step 5: check whether condition holds for each element
  if(((PTree*)root->child2)->child2 != NULL){ // conditional
    prev = icGenGeneralCondition((PTree*)((PTree*)root->child2)->child2, prev, lblCompMarker);
  }

  // step 6: push element into object
  if(((PTree*)((PTree*)root->child2)->child1)->typ == PTT_ARRAY_ELM_PAIR){ // dict
    PTree *pair = (PTree*)((PTree*)root->child2)->child1;
    ICGElmOp *res = newOpCopy(outRes);

    prev = icGen((PTree*)pair->child1, prev); // key
    ICGElmOp *op1 = newOpCopy(prev->result);
    if(op1->typ == ICGO_REG || op1->typ == ICGO_OBJREFNEW)
      ((Variable*)op1->data)->disposedTemp = true;
    prev = icGen((PTree*)pair->child2, prev); // value
    ICGElmOp *op2 = newOpCopy(prev->result);
    if(op2->typ == ICGO_REG || op2->typ == ICGO_OBJREFNEW)
      ((Variable*)op2->data)->disposedTemp = true;
    
    prev = newICGElm(prev, ICG_DICTSTORE, typeToICGDataType(root->finalType), root);
    prev->op1 = op1;
    prev->op2 = op2;
    prev->result = res;
  }else{ // vector
    prev = icGen((PTree*)((PTree*)root->child2)->child1, prev);
    ICGElmOp *op2 = newOpCopy(prev->result);
    if(op2->typ == ICGO_REG || op2->typ == ICGO_OBJREFNEW)
      ((Variable*)op2->data)->disposedTemp = true;
    ICGElmOp *op1 = newOpCopy(outRes);
    prev = newICGElm(prev, ICG_VPUSH, typeToICGDataType(root->finalType), root);
    prev->op1 = op1;
    prev->op2 = op2;
  }

  // step 7: reloop
  prev = newICGElm(prev, ICG_JMP, ICGDT_NONE, root);
  prev->result = newOpLabel_c(lblCompMarker);

  // step 8: exit marker & close iterator
  prev = newICGElm(prev, ICG_LBL, ICGDT_NONE, root); // fake if end label
  prev->result = newOp(ICGO_LABEL, (Variable*)lblCompEnd);
  prev = newICGElm(prev, ICG_ITER_CLOSE, ICGDT_NONE, root); // fake if end label
  prev->result = newOp(ICGO_REG, iteratorVar);

  prev = derefScope(prev); exitScope(); // SCOPE OF COMP GENERATION
  
  // to present new object to next icg
  prev = newICGElm(prev, ICG_IDENT, typeToICGDataType(root->finalType), root);
  prev->result = newOpCopy(outRes);
  
  return prev;
}

extern void icGenJump_print(ICGElm *elm, FILE* f){
  if(elm->typ == ICG_JMP){
    fprintf(f, "jmp ");
    printOp(f, elm->result);
  }else if(elm->typ == ICG_JNZ){
    fprintf(f, "jnz ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
  }else if(elm->typ == ICG_JZ){
    fprintf(f, "jz ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
  }
}

ICGElm * icGenCompObj_print(ICGElm *elm, FILE* f){
  fprintf(f, "compobj ");
  printOp(f, elm->result);
  fprintf(f, ", ");
  printOp(f, elm->op1);
  fprintf(f, ", ");
  printOp(f, elm->op2);
}

void icGenFor_print(ICGElm *elm, FILE* f){
  if(elm->typ == ICG_ITER_INIT){
    fprintf(f, "iteri ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
		
  }else if(elm->typ == ICG_ITER_CLOSE){
    fprintf(f, "iterf ");
    printOp(f, elm->result);
  }else if(elm->typ == ICG_ITER_NEXT){
    fprintf(f, "itern ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }
}
