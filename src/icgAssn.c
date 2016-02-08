/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Assignments
 *
 */

#include "icg.h"

extern ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, Variable *src);
extern ICGElm * icGenSaveToDataStruct(PTree *root, ICGElm *prev);

ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, Variable *to, Type assignType, bool nullRegister){

  if(!isTypeNumeric(to->sig) && !nullRegister){
    prev = newICGElm(prev, ICG_DR, typeToICGDataType(to->sig), NULL);
    prev->result = newOp(ICGO_REG, to);
  }
  
	ICGElm *data = icGen(root, prev);
	if(data->typ == ICG_LITERAL){
		freeICGElm(data);

		ICGElmOp *result = newOp(ICGO_REG, to);
		ICGElmOp *op1 = newOpInt_sc((char*)root->tok->extra);

		prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
		prev->result = result;
		prev->op1 = op1;
	}else if(data->typ == ICG_IDENT){
		prev = data;

		if(isTypeNumeric(root->finalType)){ // numeric identifier
		  ICGElmOp *result = newOp(ICGO_REG, to);
		  ICGElmOp *op1 = newOp(ICGO_REG, getNearbyVariable((char*)root->tok->extra));

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}else{ // object identifier
		        if(prev->result->typ != ICGO_OBJREFNEW){
			  Variable *src = getNearbyVariable((char*)root->tok->extra);
			  prev = icGenCopyObject(root, prev, src);
			}
		        ((Variable*)prev->result->data)->disposedTemp = true;

			ICGElmOp *result = newOp(ICGO_OBJREFNEW, to);
			ICGElmOp *op1 = newOpCopy(prev->result);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}
	}else{
		prev = data;

		if(isTypeNumeric(root->finalType)){ // numerical expression
			ICGElmOp *result = newOp(ICGO_REG, to);
			ICGElmOp *op1 = newOpCopy(prev->result);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}else{ // string literal / array expr
		        if(prev->result->typ != ICGO_OBJREFNEW){ // need to copy obj first
				prev = icGenCopyObject(root, prev, prev->result->data);
			}
			((Variable*)prev->result->data)->disposedTemp = true;

			ICGElmOp *result = newOp(ICGO_OBJREFNEW, to);
			ICGElmOp *op1 = newOpCopy(prev->result);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}
	}
	return prev;
}

ICGElm * icGenAssn(PTree *root, ICGElm *prev){
	PTree *lhs = (PTree*)root->child1;
	PTree *rhs = (PTree*)root->child2;

	if(lhs->typ == PTT_IDENTIFIER){
	  prev = icGenAssnToX(rhs, prev, getNearbyVariable((char*)lhs->tok->extra), root->finalType, false);
	}else{
	  Variable *tmpvar = defineVariable(NULL, root->finalType);
	  tmpvar->disposedTemp = true;
	  prev = icGenAssnToX(rhs, prev, tmpvar, root->finalType, true);
	  prev = icGenSaveToDataStruct(lhs, prev);
	}
	return prev;
}

void icGenAssn_print(ICGElm *elm, FILE* f)
{
  fprintf(f, "mov");
  printICGTypeSuffix(elm, f);
  fprintf(f, " ");
  printOp(f, elm->result);
  fprintf(f, ", ");
  printOp(f, elm->op1);
}
