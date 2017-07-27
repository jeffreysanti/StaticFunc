/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgObjLit.c
 *  Intermediate Code Generator : Handle Object Literals (Strings / lists]
 *
 */

#include "icg.h"

extern ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, Variable *to, Type assignType, bool nullRegister);

ICGElm * icGenStringLit(PTree *root, ICGElm *prev){
	char *roName = newROStringLit((char*)root->tok->extra);
	Variable *tmpvar = defineUnattachedVariable(root->finalType);

	ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);
	ICGElmOp *op1 = newOpROA_c(roName);

	prev = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, root);
	prev->result = res;
	prev->op1 = op1;
	return prev;
}


ICGElm * icGenArray(PTree *root, ICGElm *prev){
	int elmCnt = 0;
	PTree *ptr = root;
	while(ptr != NULL){
		elmCnt ++;
		ptr = (PTree*)ptr->child2;
	}

	if(root->finalType.base == TB_VECTOR || root->finalType.base == TB_VECTOR){
	  Variable *tmpvar = defineUnattachedVariable(root->finalType);
		ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);

		Type childType = ((Type*)root->finalType.children)[0];
		ICGElmOp *op1 = bitSizeOp(childType);

		ICGElmOp *op2 = newOpInt(elmCnt);

		if(root->finalType.base == TB_VECTOR){
			prev = newICGElm(prev, ICG_NEWVEC, typeToICGDataType(root->finalType), root);
		}else{
			prev = newICGElm(prev, ICG_NEWSET, typeToICGDataType(root->finalType), root);
		}
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;
			Variable *elmTemp = defineUnattachedVariable(elm->finalType);
			prev = icGenAssnToX(elm, prev, elmTemp, elm->finalType, true);
			elmTemp->disposedTemp = true;

			if(root->finalType.base == TB_VECTOR){
				ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);
				ICGElmOp *op1 = newOpInt(i);
				ICGElmOp *op2 = newOpCopy(prev->result);

				prev = newICGElm(prev, ICG_VECSTORE, typeToICGDataType(elm->finalType), elm);
				prev->result = res;
				prev->op1 = op1;
				prev->op2 = op2;
			}else{
				ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);
				ICGElmOp *op1 = newOpCopy(prev->result);

				prev = newICGElm(prev, ICG_SETSTORE, typeToICGDataType(elm->finalType), elm);
				prev->result = res;
				prev->op1 = op1;
			}

			ptr = (PTree*)ptr->child2;
		}
	}else if(root->finalType.base == TB_DICT){
	  Variable *tmpvar = defineUnattachedVariable(root->finalType);
		ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);

		Type childTypeKey = ((Type*)root->finalType.children)[0];
		ICGElmOp *op1 = bitSizeOp(childTypeKey);

		Type childTypeVal = ((Type*)root->finalType.children)[1];
		ICGElmOp *op2 = bitSizeOp(childTypeVal);

		ICGElmOp *op3 = newOpInt(elmCnt);

		prev = newICGElm(prev, ICG_NEWDICT, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;
		prev->op3 = op3;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;

			PTree *key = (PTree*)elm->child1;
			Variable *elmKeyTemp = defineUnattachedVariable(key->finalType);
			elmKeyTemp->disposedTemp = true;
			prev = icGenAssnToX(key, prev, elmKeyTemp, key->finalType, true);
			ICGElmOpType optkey = prev->result->typ;

			PTree *val = (PTree*)elm->child2;
			Variable *elmValTemp = defineUnattachedVariable(val->finalType);
			elmValTemp->disposedTemp = true;
			prev = icGenAssnToX(val, prev, elmValTemp, val->finalType, true);
			ICGElmOpType optval = prev->result->typ;

			ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG,  tmpvar) ;
			ICGElmOp *op1 = newOp(optkey, elmKeyTemp);
			ICGElmOp *op2 = newOp(optval, elmValTemp);

			prev = newICGElm(prev, ICG_DICTSTORE, typeToICGDataType(elm->finalType), elm);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;

			ptr = (PTree*)ptr->child2;
		}
	}else if(root->finalType.base == TB_TUPLE){
	  Variable *tmpvar = defineUnattachedVariable(root->finalType);
		ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);

		ICGElmOp *op1 = newOpInt(elmCnt);
		ICGElmOp *op2 = bitSizeTupleOp(root->finalType);

		prev = newICGElm(prev, ICG_NEWTUPLE, typeToICGDataType(root->finalType), root);
		prev->result = res;
		prev->op1 = op1;
		prev->op2 = op2;

		int i;
		ptr = root;
		for(i=0; i<elmCnt; i++){
			PTree *elm = (PTree*)ptr->child1;

			Variable *elmTemp = defineUnattachedVariable(elm->finalType);
			elmTemp->disposedTemp = true;
			prev = icGenAssnToX(elm, prev, elmTemp, elm->finalType, true);

			ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tmpvar);
			ICGElmOp *op1 = newOpInt(i);
			ICGElmOp *op2 = newOpCopy(prev->result);

			prev = newICGElm(prev, ICG_TPLSTORE, typeToICGDataType(elm->finalType), elm);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;

			ptr = (PTree*)ptr->child2;
		}
	}
	return prev;
}

ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, Variable *src){

	if(prev->result->typ == /*ICGO_OBJREFNEW*/ ICGO_REG){
		fprintf(stderr, "icGenCopyObject called on new object!\n");
	}

	ICGElmOp *op1 = newOp(ICGO_REG, src);

	Variable *tempVar = defineUnattachedVariable(root->finalType);
	ICGElmOp *res = newOp(/*ICGO_OBJREFNEW*/ ICGO_REG, tempVar);

	ICGElm *ret = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, root);
	ret->result = res;
	ret->op1 = op1;

	return ret;
}

void icGenObjCpy_print(ICGElm *elm, FILE* f)
{
  fprintf(f, "ocpy ");
  printOp(f, elm->result);
  fprintf(f, ", ");
  printOp(f, elm->op1);
}

void icGenArray_print(ICGElm *elm, FILE* f)
{
  if(elm->typ == ICG_NEWVEC){
    fprintf(f, "newvec ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }if(elm->typ == ICG_NEWSET){
    fprintf(f, "newset ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }else if(elm->typ == ICG_NEWDICT){
    fprintf(f, "newdict ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }else if(elm->typ == ICG_NEWTUPLE){
    fprintf(f, "newtuple ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }else if(elm->typ == ICG_VECSTORE){
    fprintf(f, "vst");
    printICGTypeSuffix(elm, f);
    fprintf(f, " ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }else if(elm->typ == ICG_SETSTORE){
    fprintf(f, "sst");
    printICGTypeSuffix(elm, f);
    fprintf(f, " ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
  }else if(elm->typ == ICG_DICTSTORE){
    fprintf(f, "dst");
    printICGTypeSuffix(elm, f);
    fprintf(f, " ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }else if(elm->typ == ICG_TPLSTORE){
    fprintf(f, "tst");
    printICGTypeSuffix(elm, f);
    fprintf(f, " ");
    printOp(f, elm->result);
    fprintf(f, ", ");
    printOp(f, elm->op1);
    fprintf(f, ", ");
    printOp(f, elm->op2);
  }
}
