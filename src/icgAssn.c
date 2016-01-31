/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Assignments
 *
 */

#include "icg.h"

extern ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, char *reg);
extern ICGElm * icGenSaveToDataStruct(PTree *root, ICGElm *prev);

ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, Variable *to, Type assignType){

	ICGElm *data = icGen(root, prev);
	if(data->typ == ICG_LITERAL){
		freeICGElm(data);

		ICGElmOp *result = newOp(ICGO_NUMERICREG, getVariableUniqueName(to));
		ICGElmOp *op1 = newOpCopyData(ICGO_NUMERICLIT, (char*)root->tok->extra);

		prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
		prev->result = result;
		prev->op1 = op1;
	}else if(data->typ == ICG_IDENT){
		prev = data;

		if(isTypeNumeric(root->finalType)){
		  ICGElmOp *result = newOp(ICGO_NUMERICREG, getVariableUniqueName(to));
		  ICGElmOp *op1 = newOp(ICGO_NUMERICREG, getVariableUniqueName(getNearbyVariable((char*)root->tok->extra)));

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}else{
			if(prev->result->typ == ICGO_OBJREF){
			  char *src = getVariableUniqueName(getNearbyVariable((char*)root->tok->extra));
				prev = icGenCopyObject(root, prev, src);
				free(src);
			}

			ICGElmOp *result = newOp(ICGO_OBJREFNEW, getVariableUniqueName(to));
			ICGElmOp *op1 = newOpCopyData(ICGO_OBJREF, prev->result->data);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}
	}else{
		prev = data;

		if(isTypeNumeric(root->finalType)){
			ICGElmOp *result = newOp(ICGO_NUMERICREG, getVariableUniqueName(to));
			ICGElmOp *op1 = newOpCopyData(ICGO_NUMERICREG, prev->result->data);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}else{
			if(prev->result->typ == ICGO_OBJREF){ // need to copy obj first
				prev = icGenCopyObject(root, prev, prev->result->data);
			}

			ICGElmOp *result = newOp(ICGO_OBJREFNEW, getVariableUniqueName(to));
			ICGElmOp *op1 = newOpCopyData(ICGO_OBJREF, prev->result->data);

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
	  prev = icGenAssnToX(rhs, prev, getNearbyVariable((char*)lhs->tok->extra), root->finalType);
	}else{
	  Variable *tmpvar = defineVariable("", root->finalType);
	  prev = icGenAssnToX(rhs, prev, tmpvar, root->finalType);
	  prev = icGenSaveToDataStruct(lhs, prev);
	}
	return prev;
}

void icGenAssn_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "mov");
	printICGTypeSuffix(elm, f);
	fprintf(f, " $%s, ", elm->result->data);

	ICGElmOp *op1 = elm->op1;
	if(op1->typ == ICGO_NUMERICLIT){
		fprintf(f, "%s", op1->data);
	}else if(op1->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", op1->data);
	}else{
		fprintf(f, "$%s", op1->data);
	}
}
