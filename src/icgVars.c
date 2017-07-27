/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Assignments
 *
 */

#include "icg.h"
#include "scopeprocessor.h"

extern ICGElm * icGenCopyObject(PTree *root, ICGElm *prev, Variable *src);
extern ICGElm * icGenSaveToDataStruct(PTree *root, ICGElm *prev);


typedef struct LambdaOffsetAddressReturn{
	ICGElm *prev;
	Variable *memWriteBaseAddr;
	int memWriteOffset;
}LambdaOffsetAddressReturn;

LambdaOffsetAddressReturn getLambdaOffsetAddress(PTree *root, ICGElm *prev, Variable *to){
	LambdaOffsetAddressReturn ret;
	
	// needs to be placed on the heap
	PackedLambdaOffset offset = findVariableLambdaOffset(to, getNodeScope(root));

	// load address of current methodscope
	Variable *tmp = defineUnattachedVariable(newBasicType(TB_NATIVE_INT));
	prev = newICGElm(prev, ICG_METHOD_PTR, ICGDT_PTR, NULL);
	prev->result = newOp(ICGO_REG, tmp);

	if(offset.offsetIndirect == 0){ // direct access
		ret.memWriteBaseAddr = tmp;
		ret.memWriteOffset = offset.offsetDirect;
	}else{
		// need to load offsetDirect first
		prev = newICGElm(prev, ICG_LOADH, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, tmp);
		prev->op1 = newOp(ICGO_REG, tmp);
		prev->op2 = newOpInt(offset.offsetDirect); // 8 bytes into it, is the previous entry offset

		// now we'll access via offset indirect
		ret.memWriteBaseAddr = tmp;
		ret.memWriteOffset = offset.offsetIndirect;
	}

	ret.prev = prev;
	return ret;
}



ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, Variable *to, Type assignType, bool nullRegister){

	// We'll have to come back to memory management later
	// For now, we need to know where the data has to be stored.
	// Either static, local, or heap based memory

	if(root != NULL){
		prev = icGen(root, prev);
	}
	ICGElmOp *op1 = newOpCopy(prev->result);

	if(!to->referencedExternally && !to->scope->globalScope){ // Basic reg write
		ICGElmOp *result = newOp(ICGO_REG, to);
		prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), root);
		prev->result = result;
		prev->op1 = op1;
	}else if(!to->referencedExternally){
		// save to static
		prev = newICGElm(prev, ICG_STORES, ICGDT_PTR, NULL);
		prev->result = newOpInt(findGlobalVariableOffset(to));
		prev->op1 = op1;
	}else{

		// get address
		LambdaOffsetAddressReturn memWrite = getLambdaOffsetAddress(root, prev, to);
		prev = memWrite.prev;

		// save to memory
		prev = newICGElm(prev, ICG_STOREH, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, memWrite.memWriteBaseAddr);
		prev->op1 = op1;
		prev->op2 = newOpInt(memWrite.memWriteOffset);
	}


	// dereference the old object (we overwrote it)
  /*if(!isTypeNumeric(to->sig) && !nullRegister){
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
		  ICGElmOp *op1 = newOpCopy(prev->result);

			prev = newICGElm(prev, ICG_MOV, typeToICGDataType(assignType), (PTree*)root->parent);
			prev->result = result;
			prev->op1 = op1;
		}else{ // object identifier
			if(prev->result->typ != ICGO_OBJREFNEW){
				Variable *src = (Variable*)prev->result->data;
				prev = icGenCopyObject(root, prev, src);
			}
			// Not really sure about this:
			//((Variable*)prev->result->data)->disposedTemp = true;

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
	}*/
	return prev;
}

ICGElm * icGenAssn(PTree *root, ICGElm *prev){
	PTree *lhs = (PTree*)root->child1;
	PTree *rhs = (PTree*)root->child2;

	if(lhs->typ == PTT_IDENTIFIER){
	  prev = icGenAssnToX(rhs, prev, lhs->var, root->finalType, false);
	}else{
	  Variable *tmpvar = defineUnattachedVariable(root->finalType);
	  tmpvar->disposedTemp = true;
	  prev = icGenAssnToX(rhs, prev, tmpvar, root->finalType, true);
	  prev = icGenSaveToDataStruct(lhs, prev);
	}
	return prev;
}


ICGElm * icGenIdent(PTree *root, ICGElm *prev){

	// An identifier has three primary types
	// 1) Local Register : We can read/write directly (ie: locals, and args)
	// 2) Global variable : static (must reference this)
	// 3) Heap : We need a load/store (ie: captured variables, globals)

	if(root->var->referencedExternally){
		LambdaOffsetAddressReturn memRead = getLambdaOffsetAddress(root, prev, root->var);
		prev = memRead.prev;

		// load From memory
		Variable *tmpvar = defineUnattachedVariable(root->var->sig);
		prev = newICGElm(prev, ICG_LOADH, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, tmpvar);
		prev->op1 = newOp(ICGO_REG, memRead.memWriteBaseAddr);
		prev->op2 = newOpInt(memRead.memWriteOffset);
	}else if(root->var->scope->globalScope){

		// load from static
		Variable *tmp = defineUnattachedVariable(root->var->sig);
		prev = newICGElm(prev, ICG_LOADS, ICGDT_PTR, NULL);
		prev->result = newOp(ICGO_REG, tmp);
		prev->op1 = newOpInt(findGlobalVariableOffset(root->var));

	}else{
		prev = newICGElm(prev, ICG_IDENT, typeToICGDataType(root->finalType), root);
		prev->result = newOp(ICGO_REG, root->var);
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

void icGenStoreLoad_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_STOREH){
		fprintf(f, "storeh ");
		printOp(f, elm->result);
		fprintf(f, "(");
		printOp(f, elm->op2);
		fprintf(f, "), ");
		printOp(f, elm->op1);
	}else if(elm->typ == ICG_LOADH){
		fprintf(f, "loadh ");
		printOp(f, elm->result);
		fprintf(f, ", ");
		printOp(f, elm->op1);
		fprintf(f, ", (");
		printOp(f, elm->op2);
		fprintf(f, ")");
	}else if(elm->typ == ICG_ALLOC){
		fprintf(f, "alloc ");
		printOp(f, elm->result);
		fprintf(f, ", ");
		printOp(f, elm->op1);
	}else if(elm->typ == ICG_LOADS){
		fprintf(f, "loads ");
		printOp(f, elm->result);
		fprintf(f, ", ");
		printOp(f, elm->op1);
	}else if(elm->typ == ICG_STORES){
		fprintf(f, "stores ");
		printOp(f, elm->result);
		fprintf(f, ", ");
		printOp(f, elm->op1);
	}
	
  /*fprintf(f, "mov");
  printICGTypeSuffix(elm, f);
  fprintf(f, " ");
  printOp(f, elm->result);
  fprintf(f, ", ");
  printOp(f, elm->op1);*/
}

