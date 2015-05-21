/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgObjAcc.c
 *  Intermediate Code Generator : Handle Object Access (. / [])
 *
 */

#include "icg.h"


inline int getTupleIndex(Type tuple, char *ident){
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

	dumpParseTreeDet(root, 0, stdout);

	char *tempVar = newTempVariable(root->finalType);
	ICGElmOp *res = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

	char *ident = (char*)((PTree*)root->child2)->tok->extra;

	PTree *src = (PTree*)root->child1;
	Type tuple = src->finalType;
	int i = getTupleIndex(tuple, ident);

	ICGElmOp *op1 = newOpInt(ICGO_LIT, i);

	ICGElm *tmp = icGen(src, prev);
	ICGElmOp *op2 = NULL;
	if(tmp->typ == ICG_IDENT){
		freeICGElm(tmp);
		op2 = newOp(ICGO_REG, getSymbolUniqueName((char*)src->tok->extra));
	}else{ // expression (other code before this)
		prev = tmp;
		op2 = newOpCopyData(ICGO_REG, prev->result->data);
	}

	ICGElm *ret = newICGElm(prev, ICG_TPLLOAD, typeToICGDataType(root->finalType), root);
	ret->result = res;
	ret->op1 = op1;
	ret->op2 = op2;

	prev = ret;
	return prev;
}


ICGElm * icGenSaveToDataStruct_aux(PTree *root, ICGElm *prev, char *tempVar, int depth){
	if(root->typ == PTT_IDENTIFIER){
		prev = newICGElm(prev, ICG_IDENT, ICGDT_NONE, root);
		prev->result = newOp(ICGO_REG, getSymbolUniqueName((char*)root->tok->extra));
		return prev;
	}
	if(root->typ == PTT_DOT){
		PTree *pTuple = (PTree*)root->child1;
		char *ident = (char*)((PTree*)root->child2)->tok->extra;
		prev = icGenSaveToDataStruct_aux(pTuple, prev, tempVar, depth+1);
		if(depth == 0){
			ICGElmOp *res = newOpCopyData(ICGO_REG, prev->result->data);

			int i = getTupleIndex(pTuple->finalType, ident);
			ICGElmOp *op1 = newOpInt(ICGO_LIT, i);
			ICGElmOp *op2 = newOp(ICGO_REG, getSymbolUniqueName(tempVar));

			prev = newICGElm(prev, ICG_TPLSTORE, typeToICGDataType(root->finalType), root);
			prev->result = res;
			prev->op1 = op1;
			prev->op2 = op2;
		}else{
			prev = icGenDot(root, prev);
		}
	}
	return prev;
}

ICGElm * icGenSaveToDataStruct(PTree *root, ICGElm *prev, char *tempVar){
	prev = icGenSaveToDataStruct_aux(root, prev, tempVar, 0);
	return prev;
}




void icGenDot_print(ICGElm *elm, FILE* f)
{
	fprintf(f, "tld");
	printICGTypeSuffix(elm, f);
	fprintf(f, " $%s, %s, $%s", elm->result->data, elm->op1->data, elm->op2->data);
}



