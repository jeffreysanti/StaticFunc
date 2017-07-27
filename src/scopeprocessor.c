/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  scopeprocessor.c
 *  Scope Processor
 *
 */

#include "scopeprocessor.h"


UT_array *PackedLambdaList;
PackedGlobalVariables GlobalVars;

void initScopeProcessor(){

  utarray_new(PackedLambdaList, &ut_ptr_icd);

  GlobalVars.size = 0;
  utarray_new(GlobalVars.vars, &ut_ptr_icd);
  utarray_new(GlobalVars.vars_offsets, &ut_ptr_icd);

}


void freeScopeProcessor(){
	PackedLambdaData **p;

	for(p=(PackedLambdaData**)utarray_front(PackedLambdaList); p!=NULL; p=(PackedLambdaData**)utarray_next(PackedLambdaList,p)) {
		freePackedLambda(*p);
	}

	utarray_free(PackedLambdaList);
	utarray_free(GlobalVars.vars);
	utarray_free(GlobalVars.vars_offsets);
}

PackedLambdaData *newPackedLambda(){
	PackedLambdaData *pl = malloc(sizeof(PackedLambdaData));

	utarray_new(pl->vars, &ut_ptr_icd);
	utarray_new(pl->vars_offsets, &ut_int_icd);
	utarray_new(pl->spaces, &ut_ptr_icd);
	utarray_new(pl->spaces_offsets, &ut_int_icd);
	//utarray_new(pl->spaces_hops, &ut_int_icd);
	
	pl->size = 0;

	return pl;
}

void freePackedLambda(PackedLambdaData *pl){

	utarray_free(pl->vars);
	utarray_free(pl->vars_offsets);
	utarray_free(pl->spaces);
	utarray_free(pl->spaces_offsets);
	//utarray_free(pl->spaces_hops);

	free(pl);
}


PackedLambdaData *processMethodScope(Scope *scope){

	// Most of the bookeeping is done inside
	// findVariable(), but we want to cache
	// the size of the tables, and correct orderin
	PackedLambdaData *pl = newPackedLambda();

	int offset = 8; // 8 bytes dedicated to execute pointer
	offset += 8; // 8 bytes for pointer to previous packed lambda data (regardless of whether it's in spaces)

	hashset_itr_t iter = hashset_iterator(scope->lambdaspace.variables);
	while(hashset_iterator_next(iter)){
		Variable *v = (Variable*)hashset_iterator_value(iter);

		utarray_push_back(pl->vars, &v);
		utarray_push_back(pl->vars_offsets, &offset);
		offset += 8;
	}
	free(iter);

	iter = hashset_iterator(scope->lambdaspace.spaces);
	while(hashset_iterator_next(iter)){
		Scope *s = (Scope*)hashset_iterator_value(iter);

		utarray_push_back(pl->spaces, &s);
		utarray_push_back(pl->spaces_offsets, &offset);

		/*int hops = 0;
		Scope *tmpScope = scope;
		while(tmpScope != s){
			if(tmpScope->typ == SCOPETYPE_FUNCTION){
				hops ++;
			}

			tmpScope = tmpScope->prev;
		}

		utarray_push_back(pl->spaces_hops, &hops);*/

		offset += 8;
	}
	free(iter);

	pl->size = offset;

	/*if(hashset_num_items(scope->lambdaspace.variables) > 0){
		do{
			Variable *v = (Variable*)hashset_iterator_value(iter);
			printf(" * %s \n", v->refname);

		}while((hashset_iterator_has_next(iter) > 0) && hashset_iterator_next(iter));
	}*/


	utarray_push_back(PackedLambdaList, &pl);
	return pl;
}



PackedLambdaOffset findVariableLambdaOffset(Variable *find, Scope *from){
	PackedLambdaOffset offset;

	Scope *methodScope = getMethodScope(from);
	PackedLambdaData *packed = methodScope->packedLambdaSpace;
	PackedLambdaData *origPacked = packed;

	
	int spaceOffset = 0;
	int spaceIDX = 0;
	while(1){

		Variable **v;
		int idx = -1;
		for(v=(Variable**)utarray_front(packed->vars); v!=NULL; v=(Variable**)utarray_next(packed->vars,v)) {
			if(*v == find){
				idx = utarray_eltidx(packed->vars, v);
				break;
			}
		}
		
		if(idx >= 0){
			offset.offsetDirect = *((int*)utarray_eltptr(packed->vars_offsets, idx));
			if(spaceOffset == 0){
				offset.offsetIndirect = 0;
			}else{
				offset.offsetIndirect = offset.offsetDirect;
				offset.offsetDirect = spaceOffset;
			}
			return offset;
		}else{
			if(spaceIDX >= utarray_len(origPacked->spaces)){
				printf("BAD CODE!\n");
				return offset;
			}
			packed = (*((Scope**)utarray_eltptr(origPacked->spaces, spaceIDX)))->packedLambdaSpace;
			spaceOffset = *((int*)utarray_eltptr(origPacked->spaces_offsets, spaceIDX));
			spaceIDX ++;
		}
	}
}



// Global variables must be accessed from memory
// and not stored in registers
// We pack them into a structure here
void processSystemGlobal(Variable *v){
	utarray_push_back(GlobalVars.vars, &v);
	utarray_push_back(GlobalVars.vars_offsets, &GlobalVars.size);
	GlobalVars.size += 8;
}

int findGlobalVariableOffset(Variable *find){
	Variable **v;
	int idx = -1;
	for(v=(Variable**)utarray_front(GlobalVars.vars); v!=NULL; v=(Variable**)utarray_next(GlobalVars.vars,v)) {
		if(*v == find){
			idx = utarray_eltidx(GlobalVars.vars, v);
			break;
		}
	}

	return *((int*)utarray_eltptr(GlobalVars.vars_offsets, idx));
}


int getStaticVarSize(){
	return GlobalVars.size;
}









