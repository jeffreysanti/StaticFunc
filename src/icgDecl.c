/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Declarations
 *
 */

#include "icg.h"

extern ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, Variable *to, Type assignType, bool nullRegister);


ICGElm *initVar(Variable *var, ICGElm *prev){
	Variable *resultTemp = defineUnattachedVariable(var->sig);
  	ICGElmOp *result = newOp(ICGO_REG, resultTemp);
			
  if(isTypeNumeric(var->sig)){
    ICGElmOp *op1 = newOpInt(0);
    prev = newICGElm(prev, ICG_MOV, typeToICGDataType(var->sig), NULL);
    prev->result = result;
    prev->op1 = op1;
    return prev;
  }
  if(var->sig.base == TB_NATIVE_STRING){
    ICGElmOp *op1 = newOpROA_c(emptyROString);
    prev = newICGElm(prev, ICG_OBJCOPY, ICGDT_PTR, NULL);
    prev->result = result;
    prev->op1 = op1;
    return prev;
  }
  if(var->sig.base == TB_VECTOR){
    Type childType = ((Type*)var->sig.children)[0];
    ICGElmOp *op1 = bitSizeOp(childType);
    ICGElmOp *op2 = newOpInt(0);
    prev = newICGElm(prev, ICG_NEWVEC, typeToICGDataType(var->sig), NULL);
    prev->result = result;
    prev->op1 = op1;
    prev->op2 = op2;
    return prev;
  }else if(var->sig.base == TB_DICT){
    Type keyType = ((Type*)var->sig.children)[0];
    Type valType = ((Type*)var->sig.children)[1];
    ICGElmOp *op1 = bitSizeOp(keyType);
    ICGElmOp *op2 = bitSizeOp(valType);
    ICGElmOp *op3 = newOpInt(0);
    prev = newICGElm(prev, ICG_NEWDICT, typeToICGDataType(var->sig), NULL);
    prev->result = result;
    prev->op1 = op1;
    prev->op2 = op2;
    prev->op3 = op3;
    return prev;
  }else if(var->sig.base == TB_TUPLE){
    ICGElmOp *op1 = newOpInt(var->sig.numchildren);
    ICGElmOp *op2 = bitSizeTupleOp(var->sig);
    prev = newICGElm(prev, ICG_NEWTUPLE, typeToICGDataType(var->sig), NULL);
    prev->result = result;
    prev->op1 = op1;
    prev->op2 = op2;

    for(int i=0; i<var->sig.numchildren; i++){
      Variable *elmTemp = defineUnattachedVariable(((Type*)var->sig.children)[i]);
      prev = initVar(elmTemp, prev);
      elmTemp->disposedTemp = true;
      
      ICGElmOp *res = newOp(ICGO_REG, var);
      ICGElmOp *op1 = newOpInt(i);
      ICGElmOp *op2 = newOpCopy(prev->result);

      prev = newICGElm(prev, ICG_TPLSTORE, typeToICGDataType(elmTemp->sig), NULL);
      prev->result = res;
      prev->op1 = op1;
      prev->op2 = op2;
    }
    return prev;
  }else if(var->sig.base == TB_FUNCTION){
    prev = newICGElm(prev, ICG_INITNULLFUNC, typeToICGDataType(var->sig), NULL);
    prev->result = result;
    return prev;
  }

  return NULL;
}

ICGElm * icGenDecl(PTree *root, ICGElm *prev){
	Type d = root->finalType;
  Variable *var = root->var;

	if(!isTypeNumeric(var->sig)){ // need to initialize non-primative variable
	  prev = initVar(var, prev);
	}
	
	if(root->child2 != NULL){
	  prev = icGenAssnToX((PTree*)root->child2, prev, var, d, false);
	}else{
		//prev = initVar(var, prev);
		prev = icGenAssnToX(NULL, prev, var, d, false);
	}
	return prev;
}

void icGenDecl_print(ICGElm *elm, FILE* f)
{
  if(elm->typ == ICG_INITNULLFUNC){
    fprintf(f, "nullfunc ");
    printOp(f, elm->result);
  }
}

