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


ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, char *to, Type assignType){

	ICGElmOp *result = newOp(ICGO_IDENT, to);
	ICGElmOp *resultb = newOp(ICGO_REG, getSymbolUniqueName(to));

	ICGElm *data = icGen(root, prev);
	if(data->typ == ICG_LITERAL){
		freeICGElm(data);

		ICGElmOp *op1 = newOpCopyData(ICGO_LIT, (char*)root->tok->extra);

		prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
		prev->result = result;
		prev->resultb = resultb;
		prev->op1 = op1;
	}else if(data->typ == ICG_IDENT){
		freeICGElm(data);

		if(isTypeNumeric(root->finalType)){
			ICGElmOp *op1 = newOp(ICGO_IDENT, (char*)root->tok->extra);
			ICGElmOp *op1b = newOp(ICGO_REG, getSymbolUniqueName((char*)root->tok->extra));

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->resultb = resultb;
			prev->op1 = op1;
			prev->op1b = op1b;
		}else{
			char *tmpreg = getSymbolUniqueName((char*)root->tok->extra);
			prev = icGenCopyObject(root, prev, tmpreg);
			free(tmpreg);

			ICGElmOp *op1 = newOpCopyData(ICGO_REG, prev->result->data);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->resultb = resultb;
			prev->op1 = op1;
		}
	}else{
		prev = data;
		ICGElmOp *op1 = newOpCopyData(ICGO_REG, prev->result->data);

		prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
		prev->result = result;
		prev->resultb = resultb;
		prev->op1 = op1;
	}
	return prev;
}

ICGElm * icGenAssn(PTree *root, ICGElm *prev){
	// TODO: Cannot assume lhs is just identifier
	PTree *lhs = (PTree*)root->child1;
	PTree *rhs = (PTree*)root->child2;

	if(lhs->typ == PTT_IDENTIFIER){
		prev = icGenAssnToX(rhs, prev, (char*)lhs->tok->extra, root->finalType);
	}
	return prev;
}

void icGenAssn_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "mov");
	printICGTypeSuffix(elm, f);
	fprintf(f, " $%s, ", elm->resultb->data);

	ICGElmOp *op1 = elm->op1;
	if(op1->typ == ICGO_IDENT)
		op1 = elm->op1b;

	if(op1->typ == ICGO_LIT){
		fprintf(f, "%s", op1->data);
	}else if(op1->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", op1->data);
	}else{
		fprintf(f, "$%s", op1->data);
	}
}
