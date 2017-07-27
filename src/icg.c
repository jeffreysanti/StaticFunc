/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icg.c
 *  Intermediate Code Generator
 *
 */

#include "icg.h"
#include "scopeprocessor.h"

static long long lastICGID = 0;
static long lastTempVar = 0;
static long lastROSL = 0;
static long lastLabel = 0;

static struct ROStringLit *ROSL = NULL;

// all external code generators
extern ICGElm * icGenDecl(PTree *, ICGElm *);
extern void icGenDecl_print(ICGElm *, FILE*);

extern ICGElm * icGenAssn(PTree *, ICGElm *);
extern void icGenAssn_print(ICGElm *, FILE*);
extern void icGenStoreLoad_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenIdent(PTree *root, ICGElm *prev);

extern ICGElm * icGenStringLit(PTree *root, ICGElm *prev);
extern void icGenObjCpy_print(ICGElm *, FILE*);

extern ICGElm * icGenArray(PTree *root, ICGElm *prev);
void icGenArray_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenArith(PTree *, ICGElm *);
extern void icGenArith_print(ICGElm *, FILE*);

extern ICGElm * icGenDot(PTree *root, ICGElm *prev);
extern void icGenDot_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenArrAcc(PTree *root, ICGElm *prev);
extern void icGenArrAcc_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenIf(PTree *root, ICGElm *prev);
extern void icGenJump_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenEquality(PTree *root, ICGElm *prev);
extern void icGenCompObj_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenWhile(PTree *root, ICGElm *prev);
extern ICGElm * icGenFor(PTree *root, ICGElm *prev);
extern void icGenFor_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenVecMethod(PTree *root, ICGElm *prev);
extern void icGenVecMethod_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenRemoveContainsMethod(PTree *root, ICGElm *prev);
extern void icGenRemoveContainsMethod_print(ICGElm *elm, FILE* f);

extern ICGElm * icGenArrayComp(PTree *root, ICGElm *prev);

extern ICGElm * icGenCall(PTree *root, ICGElm *prev);
extern ICGElm * icGenRet(PTree *root, ICGElm *prev);
extern ICGElm * icGenLambda(PTree *root, ICGElm *prev);
extern void icGenRetCall_print(ICGElm *elm, FILE* f);



char *emptyROString="**EMPTYSTRING**";


ICGElm *newICGElm(ICGElm *parent, ICGElmType typ, ICGDataType dt, PTree *ref)
{
	ICGElm *ret = malloc(sizeof(ICGElm));
	if(ret == NULL){
		fatalError("Out of Memory [newICGElm]\n");
	}
	ret->id = lastICGID ++;
	ret->typ = typ;

	ret->result = NULL;
	ret->op1 = NULL;
	ret->op2 = NULL;
	ret->op3 = NULL;

	ret->dataType = dt;

	ret->next = NULL;
	ret->prev = (void*)parent;
	if(parent != NULL){
		parent->next = (void*)ret;
	}
	ret->ref = ref;
	return ret;
}

void freeICGElm(ICGElm *elm)
{
	if(elm->prev != NULL){
		((ICGElm*)elm->prev)->next = NULL;
		elm->prev = NULL;
	}
	if(elm->next != NULL){
		((ICGElm*)elm->next)->prev = NULL;
		freeICGElm((ICGElm*)elm->next);
		elm->next = NULL;
	}

	/*fprintf(stderr, "freeing: ");
	printSingleICGElm(elm, stderr);
	fprintf(stderr, "\n");*/

	if(elm->result != NULL){
	  if(elm->result->typ == ICGO_NUMERICLIT || elm->result->typ == ICGO_RO_ADDR
	      || elm->result->typ == ICGO_LABEL)
	    free(elm->result->data);
	  free(elm->result);
	}
	if(elm->op1 != NULL){
	  if(elm->op1->typ == ICGO_NUMERICLIT || elm->op1->typ == ICGO_RO_ADDR
	      || elm->op1->typ == ICGO_LABEL)
	    free(elm->op1->data);
	  free(elm->op1);
	}
	if(elm->op2 != NULL){
	  if(elm->op2->typ == ICGO_NUMERICLIT || elm->op2->typ == ICGO_RO_ADDR
	      || elm->op2->typ == ICGO_LABEL)
	    free(elm->op2->data);
	  free(elm->op2);
	}
	if(elm->op3 != NULL){
	  if(elm->op3->typ == ICGO_NUMERICLIT || elm->op3->typ == ICGO_RO_ADDR
	      || elm->op3->typ == ICGO_LABEL)
	    free(elm->op3->data);
	  free(elm->op3);
	}

	free(elm);
}

void printSingleICGElm(ICGElm *elm, FILE *f){
	if(elm->typ == ICG_NONE || elm->typ == ICG_IDENT){
	  //fprintf(f, "nop");
	}else if(elm->typ == ICG_DB){
		fprintf(f, "db %s", (char*)elm->result->data);
	}else if(elm->typ == ICG_INITNULLFUNC){
		icGenDecl_print(elm, f);
	}else if(elm->typ == ICG_MOV){
		icGenAssn_print(elm, f);
	}else if(elm->typ == ICG_OBJCOPY){
		icGenObjCpy_print(elm, f);
	}else if(elm->typ == ICG_ADD || elm->typ == ICG_SUB || elm->typ == ICG_MUL ||
			elm->typ == ICG_DIV || elm->typ == ICG_AND || elm->typ == ICG_OR || elm->typ == ICG_XOR || elm->typ == ICG_NOT  ||
			elm->typ == ICG_GT || elm->typ == ICG_LT || elm->typ == ICG_GTE || elm->typ == ICG_LTE ||
			elm->typ == ICG_SHR || elm->typ == ICG_SHL || elm->typ == ICG_EXP || elm->typ == ICG_MOD){
		icGenArith_print(elm, f);
	}else if(elm->typ == ICG_NEWVEC || elm->typ == ICG_VECSTORE || elm->typ == ICG_NEWDICT ||
			elm->typ == ICG_DICTSTORE || elm->typ == ICG_NEWTUPLE || elm->typ == ICG_TPLSTORE ||
			elm->typ == ICG_NEWSET || elm->typ == ICG_SETSTORE){
		icGenArray_print(elm, f);
	}else if(elm->typ == ICG_VPUSH || elm->typ == ICG_VPOP || elm->typ == ICG_VQUEUE || elm->typ == ICG_VDEQUEUE ||
		 elm->typ == ICG_VSIZE || elm->typ == ICG_DKEYS || elm->typ == ICG_DVALS){
	  icGenVecMethod_print(elm, f);
	}else if(elm->typ == ICG_VREMOVE || elm->typ == ICG_VCONTAINS || elm->typ == ICG_DREMOVE || elm->typ == ICG_DCONTAINS){
	  icGenRemoveContainsMethod_print(elm, f);
	}else if(elm->typ == ICG_TPLLOAD){
		icGenDot_print(elm, f);
	}else if(elm->typ == ICG_DICTLOAD || elm->typ == ICG_VECLOAD){
		icGenArrAcc_print(elm, f);
	}else if(elm->typ == ICG_JMP || elm->typ == ICG_JNZ || elm->typ == ICG_JZ || elm->typ == ICG_JAL){
		icGenJump_print(elm, f);
	}else if(elm->typ == ICG_COMPOBJ){
		icGenCompObj_print(elm, f);
	}else if(elm->typ == ICG_ITER_INIT || elm->typ == ICG_ITER_NEXT || elm->typ == ICG_ITER_CLOSE){
		icGenFor_print(elm, f);
	}else if(elm->typ == ICG_DR){
	  fprintf(f, "dr ");
	  printOp(f, elm->result);
	}else if(elm->typ == ICG_RET || elm->typ == ICG_LOADRET || elm->typ == ICG_LABEL_ADDR || elm->typ == ICG_METHOD_PTR || 
				elm->typ == ICG_WRITE_METHOD_PTR || elm->typ == ICG_POP || elm->typ == ICG_PUSH){
		icGenRetCall_print(elm, f);
	}else if(elm->typ == ICG_STOREH || elm->typ == ICG_LOADH || elm->typ == ICG_ALLOC || elm->typ == ICG_LOADS || elm->typ == ICG_STORES){
		icGenStoreLoad_print(elm, f);
	}else{
		fprintf(f, "???");
	}
}

void printICG(ICGElm *root, FILE *f, bool address)
{
	while(root != NULL){
		if(root->typ == ICG_LBL){
			fprintf(f, "  %s : ", root->result->data);
			fprintf(f, "\n");
		}else if(root->typ == ICG_LITERAL || root->typ == ICG_IDENT || root->typ == ICG_NONE){

		}else{
			if(address){
				fprintf(f, "%16lx: ", root->id);
			}else{
				fprintf(f, "      : ");
			}
			printSingleICGElm(root, f);
			fprintf(f, "\n");
		}
		root = (ICGElm*)root->next;
	}
}

ICGElmOp *newOpCopyData(ICGElmOpType typ, char *data){
	char *newData = malloc(strlen(data)+1);
	strcpy(newData, data);
	return newOp(typ, (Variable*)newData);
}

ICGElm *icGen(PTree *root, ICGElm *prev)
{
	// for every type of statement:
	if(root->typ == PTT_INT || root->typ == PTT_FLOAT){
		prev = newICGElm(prev, ICG_LITERAL, typeToICGDataType(root->finalType), root);
		prev->result = newOpCopyData(ICGO_NUMERICLIT, root->tok->extra);
	}else if(root->typ == PTT_IDENTIFIER){
		prev = icGenIdent(root, prev);
	}else if(root->typ == PTT_DECL){
		prev = icGenDecl(root, prev);
	}else if(root->typ == PTT_ASSIGN){
		prev = icGenAssn(root, prev);
	}else if(root->typ == PTT_STRING){
		prev = icGenStringLit(root, prev);
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT ||
			root->typ == PTT_DIV || root->typ == PTT_AND || root->typ == PTT_OR || root->typ == PTT_XOR || root->typ == PTT_NOT ||
			root->typ == PTT_GT || root->typ == PTT_LT || root->typ == PTT_GTE || root->typ == PTT_LTE ||
			root->typ == PTT_SHR || root->typ == PTT_SHL || root->typ == PTT_EXP || root->typ == PTT_MOD){
		prev = icGenArith(root, prev);
	}else if(root->typ == PTT_ARRAY_ELM){
		prev = icGenArray(root, prev);
	}else if(root->typ == PTT_POP || root->typ == PTT_PUSH || root->typ == PTT_QUEUE || root->typ == PTT_DEQUEUE ||
		 root->typ == PTT_SIZE || root->typ == PTT_KEYS || root->typ == PTT_VALUES){
	  prev = icGenVecMethod(root, prev);
	}else if(root->typ == PTT_REMOVE || root->typ == PTT_CONTAINS){
	  prev = icGenRemoveContainsMethod(root,prev);
	}else if(root->typ == PTT_DOT){
		prev = icGenDot(root, prev);
	}else if(root->typ == PTT_ARR_ACCESS){
		prev = icGenArrAcc(root, prev);
	}else if(root->typ == PTT_IF){
		prev = icGenIf(root, prev);
	}else if(root->typ == PTT_EQUAL){
		prev = icGenEquality(root, prev);
	}else if(root->typ == PTT_WHILE){
		prev = icGenWhile(root, prev);
	}else if(root->typ == PTT_FOR){
		prev = icGenFor(root, prev);
	}else if(root->typ == PTT_ARRAY_COMP){
	  prev = icGenArrayComp(root, prev);
	}else if(root->typ == PTT_PARAM_CONT){
	  prev = icGenCall(root, prev);
	}else if(root->typ == PTT_RETURN){
	  prev = icGenRet(root, prev);
	}else if(root->typ == PTT_LAMBDA){
		prev = icGenLambda(root, prev);
	}/*else if(root->typ == PTT_FUNCTION){
	  // start of function handled elsewhere
	}*/else{
		//fatalError("ICG Code GEN: Unknown Tree Expression: %s", getParseNodeName(root));
		fprintf(stderr, "ICG Code GEN: Unknown Tree Expression: %s\n", getParseNodeName(root));
	}
	return prev;
}

ICGElm * icGenBlock(PTree *root, ICGElm *prev){
  if(root->typ == PTT_FUNCTION || root->typ == PTT_LAMBDA){
    root = (PTree*)root->child2;
  }
	if(root->typ != PTT_STMTBLOCK){
		prev = icGen(root, prev);
		return prev;
	}
	while(root != NULL && root->child1 != NULL && root->typ == PTT_STMTBLOCK){
		PTree *gen = (PTree*)root->child1;
		prev = icGen(gen, prev);
		root = (PTree*)root->child2;
	}
	return prev;
}

int funcNo = 1;
UT_array *GeneratedFunctions;
UT_array *icRunGen(PTree *root, FILE *outfl)
{
	utarray_new(GeneratedFunctions, &ut_ptr_icd);

	// First make space for global variables
	ICGElm* staticSpace = newICGElm(NULL, ICG_DB, ICGDT_NONE, NULL); // fake if begin label
  	staticSpace->result = newOpInt(getStaticVarSize());
  	utarray_push_back(GeneratedFunctions, &staticSpace);


	// make a pass through all used functions to make labels for them
	FunctionVersion **fver = NULL;
	int fcount = 0;
	getAllUsedFunctionVersions(&fver, &fcount);
	for(int i=0; i<fcount; i++){
	  FunctionVersion *v = fver[i];
	  char *str = calloc(10+strlen(v->funcName), 1);
      sprintf(str, "func%d_%s", funcNo++, v->funcName);
	  v->icgEntryLabel = newLabel(str);
	  free(str);
	}

	icRunGenFunction(root, NULL, NULL); // .Main entry

	// now all used functions
	for(int i=0; i<fcount; i++){
	  FunctionVersion *v = fver[i];
	  icRunGenFunction(v->defRoot, v->funcName, v);
	}
	free(fver);
	
	// print everything out
	ICGElm **p;
	for(p=(ICGElm**)utarray_front(GeneratedFunctions); p!=NULL; p=(ICGElm**)utarray_next(GeneratedFunctions,p)) {
		printICG(*p, stdout, true);
		printICG(*p, outfl, false);
	}

	return GeneratedFunctions;
}

void freeICGSystem(){
	struct ROStringLit *tmp1, *tmp2;
	HASH_ITER(hh, ROSL, tmp1, tmp2) {
		HASH_DEL(ROSL,tmp1);
		free(tmp1->rodata);
		free(tmp1->varname);
		free(tmp1);
	}

	ICGElm **p;
	for(p=(ICGElm**)utarray_front(GeneratedFunctions); p!=NULL; p=(ICGElm**)utarray_next(GeneratedFunctions,p)) {
		freeICGElm(*p);
	}

	utarray_free(GeneratedFunctions);
}

char *icRunGenFunction(PTree *root, char *funcName, FunctionVersion *fv){
  char *str = NULL;
  char *lbl = NULL;
  if(fv == NULL){
	if(funcName == NULL){
		str = calloc(10, 1);
		sprintf(str, ".Main");
	}else if(funcName == FUNCNAME_LAMBDA){
			str = calloc(10, 1);
		sprintf(str, "lambda_%d", funcNo++, funcName);
	}
	lbl = newLabel(str);
  }else{
	lbl = fv->icgEntryLabel;
  }

  ICGElm* ptr = newICGElm(NULL, ICG_LBL, ICGDT_NONE, root); // fake if begin label
  ptr->result = newOp(ICGO_LABEL, (Variable*)lbl);

  utarray_push_back(GeneratedFunctions, &ptr);

  
  // pop all params
  if(root->typ == PTT_FUNCTION || root->typ == PTT_LAMBDA){
	PTree *params = (PTree*)((PTree*)root->child1)->child2;
	while(params != NULL){
		PTree *desc = (PTree*)params->child1;
		ptr = newICGElm(ptr, ICG_POP, typeToICGDataType(desc->finalType), NULL);
		ptr->result = newOp(ICGO_REG, desc->var);
		
		params = (PTree*)params->child2;
	}
  }

  
  ptr = icGenBlock(root, ptr);
  free(str);
  return lbl;
}


ICGElmOp *newOp(ICGElmOpType typ, Variable *data)
{
	ICGElmOp *ret = malloc(sizeof(ICGElmOp));
	if(ret == NULL){
		fatalError("Out of Memory [newOp]\n");
	}
	ret->data = data;
	ret->typ = typ;
	return ret;
}

ICGElmOp *newOpInt(int val)
{
	char *str = calloc(20, 1);
	sprintf(str, "%d", val);
	return newOp(ICGO_NUMERICLIT, (Variable*)str);
}

ICGElmOp *newOpInt_s(char* val)
{
  return newOp(ICGO_NUMERICLIT, (Variable*)val);
}

ICGElmOp *newOpInt_sc(char* val)
{
  char *newData = malloc(strlen(val)+1);
  strcpy(newData, val);
  return newOp(ICGO_NUMERICLIT, (Variable*)newData);
}

ICGElmOp *newOpROA_c(char *val){
  ICGElmOp *ret = newOpInt_sc(val);
  ret->typ = ICGO_RO_ADDR;
  return ret;
}

ICGElmOp *newOpLabel_c(char *val){
  ICGElmOp *ret = newOpInt_sc(val);
  ret->typ = ICGO_LABEL;
  return ret;
}

ICGElmOp *newOpCopy(ICGElmOp *cpy){
  ICGElmOp *ret = newOp(cpy->typ, NULL);
  if(ret->typ == ICGO_NUMERICLIT || ret->typ == ICGO_RO_ADDR){
    char *newData = malloc(strlen(cpy->data)+1);
    strcpy(newData, cpy->data);
    ret->data = (Variable*)newData;
  }else if(ret->typ == ICGO_REG){
    ret->data = cpy->data;
  }else{
    fprintf(stderr, "Unknown ICGOP Copy!!! Number: %d\n", ret->typ);
  }
}

void printOp(FILE *f, ICGElmOp *op){
  if(op->typ == ICGO_NUMERICLIT){
    fprintf(f, "%s", (char*)op->data);
  }else if(op->typ == ICGO_RO_ADDR){
    fprintf(f, "%%%s", (char*)op->data);
  }else if(op->typ == ICGO_REG){
    char *symname = getVariableUniqueName(op->data);
    fprintf(f, "$%s", symname);
    free(symname);
  }else if(op->typ == ICGO_LABEL){
    fprintf(f, "%s", (char*)op->data);
  }else{
    fprintf(f, "??");
  }
}

ICGDataType typeToICGDataType(Type d)
{
	ICGDataType dt = ICGDT_NONE;
	if(d.base == TB_NATIVE_INT || d.base == TB_NATIVE_BOOL){
		dt = ICGDT_INT;
	}else if(d.base == TB_NATIVE_FLOAT){
		dt = ICGDT_FLOAT;
	}else{
		dt = ICGDT_PTR;
	}
	return dt;
}

void printICGTypeSuffix(ICGElm *elm, FILE* f){
	if(elm->dataType == ICGDT_INT){
		fprintf(f, "i");
	}else if(elm->dataType == ICGDT_FLOAT){
		fprintf(f, "f");
	}else if(elm->dataType == ICGDT_PTR){
		fprintf(f, "p");
	}else{
		//fprintf(f, "");
	}
}

char *newROStringLit(char *str){
	lastROSL ++;
	char *nm = calloc(26, 1);
	sprintf(nm, ".rosl%ld", lastROSL);

	char *newData = malloc(strlen(str)+1);
	strcpy(newData, str);

	struct ROStringLit *s = (struct ROStringLit*)malloc(sizeof(struct ROStringLit));
	s->varname = nm;
	s->rodata = newData;
	HASH_ADD_KEYPTR(hh, ROSL, s->varname, strlen(s->varname), s);
	return nm;
}

char *newLabel(char *base){
	lastLabel ++;
	char *nm = calloc(12+strlen(base), 1);
	sprintf(nm, ".l%s.%ld", base, lastLabel);
	return nm;
}


ICGElm * derefScope(ICGElm *prev){
  /*Scope *s = currentScope();
  Variable *ptr = (Variable*)s->variables;
  while(ptr != NULL){
    if(!ptr->disposedTemp){
      if(!isTypeNumeric(ptr->sig)){
	prev = newICGElm(prev, ICG_DR, typeToICGDataType(ptr->sig), NULL);
	prev->result = newOp(ICGO_REG, ptr);
      }
    }
    ptr = (Variable*)ptr->next;
  }*/
  return prev;
}

