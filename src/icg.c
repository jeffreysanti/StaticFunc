/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icg.c
 *  Intermediate Code Generator
 *
 */

#include "icg.h"

static long long lastICGID = 0;
static long lastTempVar = 0;
static long lastROSL = 0;
static long lastLabel = 0;

static struct ROStringLit *ROSL = NULL;

UT_array *temp_vars_tofree = NULL;

// all external code generators
extern ICGElm * icGenDecl(PTree *, ICGElm *);
extern void icGenDecl_print(ICGElm *, FILE*);

extern ICGElm * icGenAssn(PTree *, ICGElm *);
extern void icGenAssn_print(ICGElm *, FILE*);

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
		free(elm->result->data);
		free(elm->result);
	}
	if(elm->op1 != NULL){
		free(elm->op1->data);
		free(elm->op1);
	}
	if(elm->op2 != NULL){
		free(elm->op2->data);
		free(elm->op2);
	}
	if(elm->op3 != NULL){
		free(elm->op3->data);
		free(elm->op3);
	}

	free(elm);
}

void printSingleICGElm(ICGElm *elm, FILE *f){
	if(elm->typ == ICG_NONE || elm->typ == ICG_IDENT){
	  //fprintf(f, "nop");
	}else if(elm->typ == ICG_DEFINE){
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
		 elm->typ == ICG_VSIZE){
	  icGenVecMethod_print(elm, f);
	}else if(elm->typ == ICG_VREMOVE || elm->typ == ICG_VCONTAINS || elm->typ == ICG_DREMOVE || elm->typ == ICG_DCONTAINS){
	  icGenRemoveContainsMethod_print(elm, f);
	}else if(elm->typ == ICG_TPLLOAD){
		icGenDot_print(elm, f);
	}else if(elm->typ == ICG_DICTLOAD || elm->typ == ICG_VECLOAD){
		icGenArrAcc_print(elm, f);
	}else if(elm->typ == ICG_JMP || elm->typ == ICG_JNZ || elm->typ == ICG_JZ){
		icGenJump_print(elm, f);
	}else if(elm->typ == ICG_COMPOBJ){
		icGenCompObj_print(elm, f);
	}else if(elm->typ == ICG_ITER_INIT || elm->typ == ICG_ITER_NEXT || elm->typ == ICG_ITER_CLOSE){
		icGenFor_print(elm, f);
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
		}else if(root->typ == ICG_LITERAL || root->typ == ICG_IDENT){

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
	return newOp(typ, newData);
}

ICGElm *icGen(PTree *root, ICGElm *prev)
{
	// for every type of statement:
	if(root->typ == PTT_INT || root->typ == PTT_FLOAT){
		prev = newICGElm(prev, ICG_LITERAL, typeToICGDataType(root->finalType), root);
		prev->result = newOpCopyData(ICGO_NUMERICLIT, root->tok->extra);
	}else if(root->typ == PTT_IDENTIFIER){
		prev = newICGElm(prev, ICG_IDENT, typeToICGDataType(root->finalType), root);
		if(isTypeNumeric(root->finalType)){
		  prev->result = newOp(ICGO_NUMERICREG, getVariableUniqueName(getNearbyVariable(root->tok->extra)));
		}else{
		  prev->result = newOp(ICGO_OBJREF, getVariableUniqueName(getNearbyVariable(root->tok->extra)));
		}
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
		 root->typ == PTT_SIZE){
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
        }else{
		//fatalError("ICG Code GEN: Unknown Tree Expression: %s", getParseNodeName(root));
		fprintf(stderr, "ICG Code GEN: Unknown Tree Expression: %s\n", getParseNodeName(root));
	}
	return prev;
}

ICGElm * icGenBlock(PTree *root, ICGElm *prev){
	if(root->typ != PTT_STMTBLOCK){
		prev = icGen(root, prev);
		return prev;
	}
	while(root != NULL && root->typ == PTT_STMTBLOCK){
		PTree *gen = (PTree*)root->child1;
		prev = icGen(gen, prev);
		root = (PTree*)root->child2;
	}
	return prev;
}

void icRunGen(PTree *root, char *outfl)
{
	ICGElm *icgroot = newICGElm(NULL, ICG_NONE, ICGDT_NONE, NULL);
	ICGElm *ptr = icgroot;

	utarray_new(temp_vars_tofree, &ut_ptr_icd);

	icGenBlock(root, ptr);

	printICG(icgroot, stdout, true);

	FILE *tmpout = openOutputFile(outfl, "icg");
	printICG(icgroot, tmpout, false);
	fclose(tmpout);

	freeICGElm(icgroot);

	char **p = NULL;
	while ( (p=(char**)utarray_next(temp_vars_tofree,p))) {
		free(*p);
	}
	utarray_free(temp_vars_tofree);

	struct ROStringLit *tmp1, *tmp2;
	HASH_ITER(hh, ROSL, tmp1, tmp2) {
		HASH_DEL(ROSL,tmp1);
		free(tmp1->rodata);
		free(tmp1->varname);
		free(tmp1);
	}

}

ICGElmOp *newOp(ICGElmOpType typ, char *data)
{
	ICGElmOp *ret = malloc(sizeof(ICGElmOp));
	if(ret == NULL){
		fatalError("Out of Memory [newOp]\n");
	}
	ret->data = data;
	ret->typ = typ;
	return ret;
}

ICGElmOp *newOpInt(ICGElmOpType typ, int val)
{
	char *str = calloc(20, 1);
	sprintf(str, "%d", val);
	return newOp(ICGO_NUMERICLIT, str);
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
