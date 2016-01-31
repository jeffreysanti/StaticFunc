/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgArith.c
 *  Intermediate Code Generator : Arithmetic
 *
 */

#include "icg.h"

extern ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, char *reg);


static inline ICGElm * processChild(PTree *child1, ICGElm *prev, ICGElmOp **op){
	ICGElm *data1 = icGen(child1, prev);
	*op = NULL;
	if(data1->typ == ICG_LITERAL){
		freeICGElm(data1);
		*op = newOpCopyData(ICGO_NUMERICLIT, (char*)child1->tok->extra);
	}else if(data1->typ == ICG_IDENT){
		freeICGElm(data1);
		if(isTypeNumeric(child1->finalType)){
		  *op = newOp(ICGO_NUMERICREG, getVariableUniqueName(getNearbyVariable((char*)child1->tok->extra)));
		}else{
		  char *tmpreg = getVariableUniqueName(getNearbyVariable((char*)child1->tok->extra));
			prev = icGenCopyObject(child1, prev, tmpreg);
			free(tmpreg);
			*op = newOpCopyData(ICGO_NUMERICREG, prev->result->data);
		}
	}else{ // expression (other code before this)
		prev = data1;
		*op = newOpCopyData(ICGO_NUMERICREG, prev->result->data);
	}
	return prev;
}

ICGElm * icGenArith(PTree *root, ICGElm *prev){
	Type d = root->finalType;


	Variable *tmpvar = defineVariable(NULL, d);
	ICGElmOp *res = newOp(ICGO_NUMERICREG, getVariableUniqueName(tmpvar));

	ICGElmOp *op1, *op2;
	prev = processChild((PTree*)root->child1, prev, &op1);

	if(root->typ != PTT_NOT){
		prev = processChild((PTree*)root->child2, prev, &op2);
	}else{
		op2 = NULL;
	}

	ICGElmType arithType = ICG_NONE;
	if(root->typ == PTT_ADD) arithType = ICG_ADD;
	if(root->typ == PTT_SUB) arithType = ICG_SUB;
	if(root->typ == PTT_MULT) arithType = ICG_MUL;
	if(root->typ == PTT_DIV) arithType = ICG_DIV;
	if(root->typ == PTT_AND) arithType = ICG_AND;
	if(root->typ == PTT_OR) arithType = ICG_OR;
	if(root->typ == PTT_XOR) arithType = ICG_XOR;
	if(root->typ == PTT_NOT) arithType = ICG_NOT;
	if(root->typ == PTT_GT) arithType = ICG_GT;
	if(root->typ == PTT_LT) arithType = ICG_LT;
	if(root->typ == PTT_GTE) arithType = ICG_GTE;
	if(root->typ == PTT_LTE) arithType = ICG_LTE;
	if(root->typ == PTT_SHR) arithType = ICG_SHR;
	if(root->typ == PTT_SHL) arithType = ICG_SHL;
	if(root->typ == PTT_EXP) arithType = ICG_EXP;
	if(root->typ == PTT_MOD) arithType = ICG_MOD;

	prev = newICGElm(prev, arithType, typeToICGDataType(d), root);
	prev->result = res;
	prev->op1 = op1;
	prev->op2 = op2;

	return prev;
}


void icGenArith_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_ADD) fprintf(f, "add");
	if(elm->typ == ICG_MUL) fprintf(f, "mul");
	if(elm->typ == ICG_DIV) fprintf(f, "div");
	if(elm->typ == ICG_SUB) fprintf(f, "sub");
	if(elm->typ == ICG_AND) fprintf(f, "and");
	if(elm->typ == ICG_OR) fprintf(f, "or");
	if(elm->typ == ICG_XOR) fprintf(f, "xor");
	if(elm->typ == ICG_NOT) fprintf(f, "not");
	if(elm->typ == ICG_GT) fprintf(f, "cmpgt");
	if(elm->typ == ICG_LT) fprintf(f, "cmplt");
	if(elm->typ == ICG_GTE) fprintf(f, "cmpgte");
	if(elm->typ == ICG_LTE) fprintf(f, "cmplte");
	if(elm->typ == ICG_SHR) fprintf(f, "shr");
	if(elm->typ == ICG_SHL) fprintf(f, "shl");
	if(elm->typ == ICG_EXP) fprintf(f, "exp");
	if(elm->typ == ICG_MOD) fprintf(f, "mod");


	printICGTypeSuffix(elm, f);

	fprintf(f, " $%s, ", elm->result->data);

	ICGElmOp *op = elm->op1;
	if(op->typ == ICGO_NUMERICLIT){
		fprintf(f, "%s", op->data);
	}else if(op->typ == ICGO_RO_ADDR){
		fprintf(f, "%%%s", op->data);
	}else{
		fprintf(f, "$%s", op->data);
	}

	if(elm->op2 != NULL){
		fprintf(f, ", ");
		op = elm->op2;
		if(op->typ == ICGO_NUMERICLIT){
			fprintf(f, "%s", op->data);
		}else if(op->typ == ICGO_RO_ADDR){
			fprintf(f, "%%%s", op->data);
		}else{
			fprintf(f, "$%s", op->data);
		}
	}
}
