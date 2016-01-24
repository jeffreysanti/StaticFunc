/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgObjAcc.c
 *  Intermediate Code Generator : Handle Object Access (. / [])
 *
 */

#include "icg.h"


static inline int getTupleIndex(Type tuple, char *ident){
	int i;
	for(i=0; i<tuple.numchildren; i++){
		if(((Type*)tuple.children)[i].altName == NULL)
			continue;
		if(strcmp(((Type*)tuple.children)[i].altName, ident)==0){
			break;
		}
	}
	return i;
}

ICGElm * icGenDot(PTree *root, ICGElm *prev){
	char *tempVar = newTempVariable(root->finalType);
	ICGElmOp *res = NULL;
	if(isTypeNumeric(root->finalType)){
		res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));
	}else{
		res = newOp(ICGO_OBJREF, getSymbolUniqueName(tempVar));
	}

	char *ident = (char*)((PTree*)root->child2)->tok->extra;

	PTree *src = (PTree*)root->child1;
	Type tuple = src->finalType;
	int i = getTupleIndex(tuple, ident);

	ICGElmOp *op1 = newOpInt(ICGO_NUMERICLIT, i);

	ICGElm *tmp = icGen(src, prev);
	ICGElmOp *op2 = NULL;
	if(tmp->typ == ICG_IDENT){
		op2 = newOp(tmp->result->typ, getSymbolUniqueName((char*)src->tok->extra));
		freeICGElm(tmp);
	}else{ // expression (other code before this)
		prev = tmp;
		op2 = newOpCopyData(tmp->result->typ, prev->result->data);
	}

	ICGElm *ret = newICGElm(prev, ICG_TPLLOAD, typeToICGDataType(root->finalType), root);
	ret->result = res;
	ret->op1 = op1;
	ret->op2 = op2;

	prev = ret;
	return prev;
}

ICGElm * icGenArrAcc(PTree *root, ICGElm *prev){
	char *tempVar = newTempVariable(root->finalType);
	ICGElmOp *res = NULL;
	ICGElmOp *op1 = NULL;
	ICGElmOp *op2 = NULL;
	if(isTypeNumeric(root->finalType)){
		res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));
	}else{
		res = newOp(ICGO_OBJREF, getSymbolUniqueName(tempVar));
	}

	ICGElm *genKey = icGen((PTree*)root->child2, prev); // for op1
	if(genKey->typ == ICG_LITERAL){
		char *keyval = (char*)((PTree*)root->child2)->tok->extra;
		op1 = newOpCopyData(ICGO_NUMERICLIT, keyval);
		freeICGElm(genKey);
	}else{
		prev = genKey;
		op1 = newOpCopyData(genKey->result->typ, genKey->result->data);
	}

	prev = icGen((PTree*)root->child1, prev); // for op2
	op2 = newOpCopyData(prev->result->typ, prev->result->data);

	if(((PTree*)root->child1)->finalType.base == TB_DICT){
		prev = newICGElm(prev, ICG_DICTLOAD, typeToICGDataType(root->finalType), root);
	}else{
		prev = newICGElm(prev, ICG_VECLOAD, typeToICGDataType(root->finalType), root);
	}

	prev->result = res;
	prev->op1 = op1;
	prev->op2 = op2;

	return prev;
}


ICGElm * icGenSaveToDataStruct_aux(PTree *root, ICGElm *prev, ICGElm *src, int depth){
	if(root->typ == PTT_IDENTIFIER){
		prev = newICGElm(prev, ICG_IDENT, ICGDT_NONE, root);
		prev->result = newOp(ICGO_OBJREF, getSymbolUniqueName((char*)root->tok->extra));
		return prev;
	}
	if(root->typ == PTT_DOT){
		PTree *pTuple = (PTree*)root->child1;
		char *ident = (char*)((PTree*)root->child2)->tok->extra;
		prev = icGenSaveToDataStruct_aux(pTuple, prev, src, depth+1);
		if(depth == 0){
			ICGElmOp *res = newOpCopyData(ICGO_OBJREF, prev->result->data);

			int i = getTupleIndex(pTuple->finalType, ident);
			ICGElmOp *op1 = newOpInt(ICGO_NUMERICLIT, i);
			ICGElmOp *op2 = newOpCopyData(src->result->typ, src->result->data);

			prev = newICGElm(prev, ICG_TPLSTORE, typeToICGDataType(root->finalType), root);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;
		}else{
			prev = icGenDot(root, prev);
		}
	}else if(root->typ == PTT_ARR_ACCESS){
		PTree *pArray = (PTree*)root->child1;
		PTree *pKey = (PTree*)root->child2;
		prev = icGenSaveToDataStruct_aux(pArray, prev, src, depth+1);
		if(depth == 0){
			ICGElmOp *res = newOpCopyData(ICGO_OBJREF, prev->result->data);

			// get access element
			ICGElm *genKey = icGen(pKey, prev);
			ICGElmOp *op1 = NULL;
			if(genKey->typ == ICG_LITERAL){
				char *keyval = (char*)pKey->tok->extra;
				op1 = newOpCopyData(ICGO_NUMERICLIT, keyval);
				freeICGElm(genKey);
			}else{
				prev = genKey;
				op1 = newOpCopyData(genKey->result->typ, genKey->result->data);
			}

			ICGElmOp *op2 = newOpCopyData(src->result->typ, src->result->data);

			if(pArray->finalType.base == TB_DICT){
			  prev = newICGElm(prev, ICG_DICTSTORE, typeToICGDataType(root->finalType), root);
			}else{ // vector
			  prev = newICGElm(prev, ICG_VECSTORE, typeToICGDataType(root->finalType), root);
			}
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;
		}else{
			prev = icGenArrAcc(root, prev);
		}
	}
	return prev;
}

ICGElm * icGenSaveToDataStruct(PTree *root, ICGElm *prev){
	prev = icGenSaveToDataStruct_aux(root, prev, prev, 0);
	return prev;
}


ICGElm * icGenVecMethod(PTree *root, ICGElm *prev){
  
  char *tempVar = newTempVariable(root->finalType);
  ICGElmOp *res = NULL;
  ICGElmOp *op1 = NULL;
  ICGElmOp *op2 = NULL;

  if(root->typ != PTT_PUSH && root->typ != PTT_QUEUE){
     if(isTypeNumeric(root->finalType)){
	     res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));
     }else{
	     res = newOp(ICGO_OBJREF, getSymbolUniqueName(tempVar));
     }
  }
  
  
  // load pointer to object
  prev = icGen((PTree*)root->child1, prev); // for op1
  op1 = newOpCopyData(prev->result->typ, prev->result->data);
  
  if(root->typ == PTT_QUEUE || root->typ == PTT_PUSH){
    prev = icGen((PTree*)((PTree*)root->child2)->child2, prev);
    op2 = newOpCopyData(prev->result->typ, prev->result->data);
  }

  if(root->typ == PTT_PUSH)
    prev = newICGElm(prev, ICG_VPUSH, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_POP)
    prev = newICGElm(prev, ICG_VPOP, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_QUEUE)
    prev = newICGElm(prev, ICG_VQUEUE, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_DEQUEUE)
    prev = newICGElm(prev, ICG_VDEQUEUE, typeToICGDataType(root->finalType), root);
  else
    prev = newICGElm(prev, ICG_VSIZE, typeToICGDataType(root->finalType), root);

  prev->result = res;
  prev->op1 = op1;
  prev->op2 = op2;
  
  return prev;
}



ICGElm * icGenRemoveContainsMethod(PTree *root, ICGElm *prev){
  char *tempVar = newTempVariable(root->finalType);
  ICGElmOp *res = NULL;
  ICGElmOp *op1 = NULL;
  ICGElmOp *op2 = NULL;

  res = newOp(ICGO_NUMERICREG, getSymbolUniqueName(tempVar));  
  
  // load pointer to object
  prev = icGen((PTree*)root->child1, prev); // for op1
  op1 = newOpCopyData(prev->result->typ, prev->result->data);

  // key we're compareing
  prev = icGen((PTree*)((PTree*)root->child2)->child2, prev);
  op2 = newOpCopyData(prev->result->typ, prev->result->data);
  

  if(((PTree*)root->child1)->finalType.base == TB_DICT){
    if(root->typ == PTT_REMOVE)
      prev = newICGElm(prev, ICG_DREMOVE, typeToICGDataType(root->finalType), root);
    else
      prev = newICGElm(prev, ICG_DCONTAINS, typeToICGDataType(root->finalType), root);
  }else{
    if(root->typ == PTT_REMOVE)
      prev = newICGElm(prev, ICG_VREMOVE, typeToICGDataType(root->finalType), root);
    else
      prev = newICGElm(prev, ICG_VCONTAINS, typeToICGDataType(root->finalType), root);
  }
  prev->result = res;
  prev->op1 = op1;
  prev->op2 = op2;
  
  return prev;
}






void icGenDot_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "tld");
	printICGTypeSuffix(elm, f);
	fprintf(f, " $%s, %s, $%s", elm->result->data, elm->op1->data, elm->op2->data);
}

void icGenArrAcc_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_DICTLOAD){
		fprintf(f, "dld");
	}else{
		fprintf(f, "vld");
	}
	printICGTypeSuffix(elm, f);
	fprintf(f, " $%s, ", elm->result->data);
	if(elm->op1->typ == ICGO_NUMERICLIT){
		fprintf(f, "%s, $%s", elm->op1->data, elm->op2->data);
	}else{
		fprintf(f, "$%s, $%s", elm->op1->data, elm->op2->data);
	}
}


void icGenVecMethod_print(ICGElm *elm, FILE* f) {

  if(elm->typ == ICG_VPUSH) fprintf(f, "vpush");
  if(elm->typ == ICG_VPOP) fprintf(f, "vpop");
  if(elm->typ == ICG_VQUEUE) fprintf(f, "vqueue");
  if(elm->typ == ICG_VDEQUEUE) fprintf(f, "vdequeue");
  if(elm->typ == ICG_VSIZE) fprintf(f, "vsize");

  if(elm->result != NULL){
    	if(elm->result->typ == ICGO_NUMERICLIT){
		fprintf(f, " %s, ", elm->result->data);
	}else{
		fprintf(f, " $%s, ", elm->result->data);
	}
  }else{
    fprintf(f, " nil, ");
  }
  fprintf(f, "$%s", elm->op1->data);
  if(elm->op2 != NULL){
    	if(elm->op2->typ == ICGO_NUMERICLIT){
		fprintf(f, ", %s", elm->op2->data);
	}else{
		fprintf(f, ", $%s", elm->op2->data);
	}
  }
}


void icGenRemoveContainsMethod_print(ICGElm *elm, FILE* f) {

  if(elm->typ == ICG_VREMOVE) fprintf(f, "vrem");
  if(elm->typ == ICG_VCONTAINS) fprintf(f, "vcont");
  if(elm->typ == ICG_DREMOVE) fprintf(f, "drem");
  if(elm->typ == ICG_DCONTAINS) fprintf(f, "dcont");

  fprintf(f, " %s, ", elm->result->data);
  fprintf(f, "$%s", elm->op1->data);
  if(elm->op2->typ == ICGO_NUMERICLIT){
    fprintf(f, ", %s", elm->op2->data);
  }else{
    fprintf(f, ", $%s", elm->op2->data);
  }
}

