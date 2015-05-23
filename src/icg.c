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
			elm->typ == ICG_DIV){
		icGenArith_print(elm, f);
	}else if(elm->typ == ICG_NEWVEC || elm->typ == ICG_VECSTORE || elm->typ == ICG_NEWDICT ||
			elm->typ == ICG_DICTSTORE || elm->typ == ICG_NEWTUPLE || elm->typ == ICG_TPLSTORE){
		icGenArray_print(elm, f);
	}else if(elm->typ == ICG_TPLLOAD){
		icGenDot_print(elm, f);
	}else if(elm->typ == ICG_DICTLOAD || elm->typ == ICG_VECLOAD){
		icGenArrAcc_print(elm, f);
	}else if(elm->typ == ICG_JMP || elm->typ == ICG_JNZ || elm->typ == ICG_JZ){
		icGenJump_print(elm, f);
	}
}

void printICG(ICGElm *root, FILE *f)
{
	while(root != NULL){
		if(root->typ == ICG_LBL){
			fprintf(f, "  %s : ", root->result->data);
			fprintf(f, "\n");
		}else if(root->typ == ICG_LITERAL || root->typ == ICG_IDENT){

		}else{
			fprintf(f, "%16lx: ", root->id);
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
			prev->result = newOp(ICGO_NUMERICREG, getSymbolUniqueName(root->tok->extra));
		}else{
			prev->result = newOp(ICGO_OBJREF, getSymbolUniqueName(root->tok->extra));
		}
	}else if(root->typ == PTT_DECL){
		prev = icGenDecl(root, prev);
	}else if(root->typ == PTT_ASSIGN){
		prev = icGenAssn(root, prev);
	}else if(root->typ == PTT_STRING){
		prev = icGenStringLit(root, prev);
	}else if(root->typ == PTT_ADD || root->typ == PTT_SUB || root->typ == PTT_MULT ||
			root->typ == PTT_DIV){
		prev = icGenArith(root, prev);
	}else if(root->typ == PTT_ARRAY_ELM){
		prev = icGenArray(root, prev);
	}else if(root->typ == PTT_DOT){
		prev = icGenDot(root, prev);
	}else if(root->typ == PTT_ARR_ACCESS){
		prev = icGenArrAcc(root, prev);
	}else if(root->typ == PTT_IF){
		prev = icGenIf(root, prev);
	}else if(root->typ == PTT_EQUAL){
		prev = icGenEquality(root, prev);
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

void icRunGen(PTree *root)
{
	ICGElm *icgroot = newICGElm(NULL, ICG_NONE, ICGDT_NONE, NULL);
	ICGElm *ptr = icgroot;

	utarray_new(temp_vars_tofree, &ut_ptr_icd);

	icGenBlock(root, ptr);

	printICG(icgroot, stdout);
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

char *newTempVariable(Type t)
{
	lastTempVar ++;
	char *nm = calloc(26, 1);
	sprintf(nm, ".temp%ld", lastTempVar);
	addSymbol(nm, duplicateType(t));
	utarray_push_back(temp_vars_tofree, &nm);
	return nm;
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
	if(d.base == TB_NATIVE_INT8 || d.base == TB_NATIVE_BOOL){
		dt = ICGDT_INT8;
	}else if(d.base == TB_NATIVE_INT16){
		dt = ICGDT_INT16;
	}else if(d.base == TB_NATIVE_INT32){
		dt = ICGDT_INT32;
	}else if(d.base == TB_NATIVE_INT64){
		dt = ICGDT_INT64;
	}else if(d.base == TB_NATIVE_FLOAT32){
		dt = ICGDT_FLOAT32;
	}else if(d.base == TB_NATIVE_FLOAT64){
		dt = ICGDT_FLOAT64;
	}else{
		dt = ICGDT_PTR;
	}
	return dt;
}

void printICGTypeSuffix(ICGElm *elm, FILE* f){
	if(elm->dataType == ICGDT_INT8){
		fprintf(f, "i8");
	}else if(elm->dataType == ICGDT_INT16){
		fprintf(f, "i16");
	}else if(elm->dataType == ICGDT_INT32){
		fprintf(f, "i32");
	}else if(elm->dataType == ICGDT_INT64){
		fprintf(f, "i64");
	}else if(elm->dataType == ICGDT_FLOAT32){
		fprintf(f, "f32");
	}else if(elm->dataType == ICGDT_FLOAT64){
		fprintf(f, "f64");
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


