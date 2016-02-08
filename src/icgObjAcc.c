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
  Variable *tmpvar = defineVariable(NULL, root->finalType);
  tmpvar->disposedTemp = true;
  ICGElmOp *res = newOp(ICGO_REG, tmpvar);

  char *ident = (char*)((PTree*)root->child2)->tok->extra;

  PTree *src = (PTree*)root->child1;
  Type tuple = src->finalType;
  int i = getTupleIndex(tuple, ident);

  ICGElmOp *op1 = newOpInt(i);

  ICGElm *tmp = icGen(src, prev);
  ICGElmOp *op2 = NULL;
  if(tmp->typ == ICG_IDENT){
    op2 = newOp(tmp->result->typ, getNearbyVariable((char*)src->tok->extra));
    freeICGElm(tmp);
  }else{ // expression (other code before this)
    prev = tmp;
    op2 = newOpCopy(prev->result);
  }

  ICGElm *ret = newICGElm(prev, ICG_TPLLOAD, typeToICGDataType(root->finalType), root);
  ret->result = res;
  ret->op1 = op1;
  ret->op2 = op2;

  prev = ret;
  return prev;
}

ICGElm * icGenArrAcc(PTree *root, ICGElm *prev){
  Variable *tmpvar = defineVariable(NULL, root->finalType);
  tmpvar->disposedTemp = true;
  ICGElmOp *res = NULL;
  ICGElmOp *op1 = NULL;
  ICGElmOp *op2 = NULL;
  res = newOp(ICGO_REG, tmpvar);

  ICGElm *genKey = icGen((PTree*)root->child2, prev); // for op1
  if(genKey->typ == ICG_LITERAL){
    char *keyval = (char*)((PTree*)root->child2)->tok->extra;
    op1 = newOpInt_sc(keyval);
    freeICGElm(genKey);
  }else{
    prev = genKey;
    op1 = newOpCopy(genKey->result);
  }

  prev = icGen((PTree*)root->child1, prev); // for op2
  op2 = newOpCopy(prev->result);

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
		prev->result = newOp(ICGO_REG, getNearbyVariable((char*)root->tok->extra));
		return prev;
	}
	if(root->typ == PTT_DOT){
		PTree *pTuple = (PTree*)root->child1;
		char *ident = (char*)((PTree*)root->child2)->tok->extra;
		prev = icGenSaveToDataStruct_aux(pTuple, prev, src, depth+1);
		if(depth == 0){
			ICGElmOp *res = newOpCopy(prev->result);

			int i = getTupleIndex(pTuple->finalType, ident);
			ICGElmOp *op1 = newOpInt(i);
			ICGElmOp *op2 = newOpCopy(src->result);

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
			ICGElmOp *res = newOpCopy(prev->result);

			// get access element
			ICGElm *genKey = icGen(pKey, prev);
			ICGElmOp *op1 = NULL;
			if(genKey->typ == ICG_LITERAL){
				char *keyval = (char*)pKey->tok->extra;
				op1 = newOpInt_sc(keyval);
				freeICGElm(genKey);
			}else{
				prev = genKey;
				op1 = newOpCopy(genKey->result);
				((Variable*)op1->data)->disposedTemp = true;
			}

			ICGElmOp *op2 = newOpCopy(src->result);

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
  
  ICGElmOp *res = NULL;
  ICGElmOp *op1 = NULL;
  ICGElmOp *op2 = NULL;

  if(root->typ != PTT_PUSH && root->typ != PTT_QUEUE){
    Variable *tmpvar = defineVariable(NULL, root->finalType);
    res = newOp(ICGO_REG, tmpvar);
  }
  
  
  // load pointer to object
  prev = icGen((PTree*)root->child1, prev); // for op1
  op1 = newOpCopy(prev->result);
  
  if(root->typ == PTT_QUEUE || root->typ == PTT_PUSH){
    prev = icGen((PTree*)((PTree*)root->child2)->child2, prev);
    op2 = newOpCopy(prev->result);

    if(op2->typ == ICGO_REG || op2->typ == ICGO_OBJREFNEW){
      ((Variable*)op2->data)->disposedTemp = true;
    }
  }

  if(root->typ == PTT_PUSH)
    prev = newICGElm(prev, ICG_VPUSH, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_POP)
    prev = newICGElm(prev, ICG_VPOP, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_QUEUE)
    prev = newICGElm(prev, ICG_VQUEUE, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_DEQUEUE)
    prev = newICGElm(prev, ICG_VDEQUEUE, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_SIZE)
    prev = newICGElm(prev, ICG_VSIZE, typeToICGDataType(root->finalType), root);
  else if(root->typ == PTT_KEYS)
    prev = newICGElm(prev, ICG_DKEYS, typeToICGDataType(root->finalType), root);
  else
    prev = newICGElm(prev, ICG_DVALS, typeToICGDataType(root->finalType), root);


  if(root->typ == PTT_KEYS || root->typ == PTT_VALUES){
    res->typ = ICGO_OBJREFNEW;
  }
  
  
  prev->result = res;
  prev->op1 = op1;
  prev->op2 = op2;
  
  return prev;
}



ICGElm * icGenRemoveContainsMethod(PTree *root, ICGElm *prev){
  Variable *tmpvar = defineVariable(NULL, root->finalType);
  ICGElmOp *res = NULL;
  ICGElmOp *op1 = NULL;
  ICGElmOp *op2 = NULL;

  res = newOp(ICGO_REG, tmpvar);
  
  // load pointer to object
  prev = icGen((PTree*)root->child1, prev); // for op1
  op1 = newOpCopy(prev->result);

  // key we're compareing
  prev = icGen((PTree*)((PTree*)root->child2)->child2, prev);
  op2 = newOpCopy(prev->result);
  

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
        fprintf(f, " ");
	printOp(f, elm->result);
	fprintf(f, ", ");
	printOp(f, elm->op1);
	fprintf(f, ", ");
	printOp(f, elm->op2);
}

void icGenArrAcc_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_DICTLOAD){
		fprintf(f, "dld");
	}else{
		fprintf(f, "vld");
	}
	printICGTypeSuffix(elm, f);
	fprintf(f, " ");
	printOp(f, elm->result);
	fprintf(f, ", ");
	printOp(f, elm->op1);
	fprintf(f, ", ");
	printOp(f, elm->op2);
}


void icGenVecMethod_print(ICGElm *elm, FILE* f) {

  if(elm->typ == ICG_VPUSH) fprintf(f, "vpush ");
  if(elm->typ == ICG_VPOP) fprintf(f, "vpop ");
  if(elm->typ == ICG_VQUEUE) fprintf(f, "vqueue ");
  if(elm->typ == ICG_VDEQUEUE) fprintf(f, "vdequeue ");
  if(elm->typ == ICG_VSIZE) fprintf(f, "vsize ");
  if(elm->typ == ICG_DKEYS) fprintf(f, "dkeys ");
  if(elm->typ == ICG_DVALS) fprintf(f, "dvals ");
  

  if(elm->result != NULL){
    printOp(f, elm->result);
    fprintf(f, ", ");
  }else{
    fprintf(f, " nil, ");
  }
  printOp(f, elm->op1);
  if(elm->op2 != NULL){
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }
}


void icGenRemoveContainsMethod_print(ICGElm *elm, FILE* f) {

  if(elm->typ == ICG_VREMOVE) fprintf(f, "vrem ");
  if(elm->typ == ICG_VCONTAINS) fprintf(f, "vcont ");
  if(elm->typ == ICG_DREMOVE) fprintf(f, "drem ");
  if(elm->typ == ICG_DCONTAINS) fprintf(f, "dcont ");

  printOp(f, elm->result);
  fprintf(f, ", ");
  printOp(f, elm->op1);
  fprintf(f, ", ");
  printOp(f, elm->op2);
}

