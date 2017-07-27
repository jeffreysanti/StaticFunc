
#include "icgOptimize.h"


#define OPT_ZEROREAD_BLACKLIST(x) \
	x->typ == ICG_LOADRET || \
	x->typ == ICG_POP



MethodAnalysis *newMethodAnalysis(){
	MethodAnalysis *ma = malloc(sizeof(MethodAnalysis));

	ma->variableflow = NULL;

	return ma;
}

void freeMethodAnalysis(MethodAnalysis *ma){
	VariableFlowTracker *tracker, *tmp;

	HASH_ITER(hh, ma->variableflow, tracker, tmp) {
      HASH_DEL(ma->variableflow, tracker);
      freeVariableFlowTracker(tracker);
    }

	free(ma);
}

VariableFlowTracker *newVariableFlowTracker(Variable *v, ICGElm *firstAccess){
	VariableFlowTracker *tracker = malloc(sizeof(VariableFlowTracker));
	tracker->var = v;
	tracker->reads = 0;
	tracker->writes = 0;
	tracker->lastRead = NULL;
	tracker->lastWrite = NULL;
	tracker->firstAccess = firstAccess;
	tracker->flagLocal = true;
	return tracker;
}

void freeVariableFlowTracker(VariableFlowTracker *tracker){
	free(tracker);
}


OptimizationCounter newOptimizationCounter(){
	OptimizationCounter ret;
	ret.removeZeroRead = 0;

	return ret;
}

OptimizationCounter sumOptimizationCounter(OptimizationCounter o1, OptimizationCounter o2){
	OptimizationCounter ret;
	ret.removeZeroRead = o1.removeZeroRead + o2.removeZeroRead;

	return ret;
}


bool inSameLocalBlock(ICGElm *parent, ICGElm *child){
	while(parent != child){
		if(parent->typ == ICG_LBL || parent->typ == ICG_JNZ || parent->typ == ICG_JZ || parent->typ == ICG_JMP || parent->typ == ICG_JAL || parent->typ == ICG_RET){
			return false;
		}

		parent = (ICGElm*)parent->next;
	}
	return true;
}

ICGElm *localOptimizeBlock(ICGElm *ptr, MethodAnalysis *ma, OptimizationCounter *stats){
	//printf("LocalEntry\n");
	while(ptr != NULL){
		if(ptr->typ == ICG_LBL || ptr->typ == ICG_JNZ || ptr->typ == ICG_JZ || ptr->typ == ICG_JMP || ptr->typ == ICG_JAL || ptr->typ == ICG_RET){
			return ptr;
		}

		// Within this local block we can assume no variables will externally change

		// TODO: Optimize

		ptr = (ICGElm*)ptr->next;
	}
}

ICGElm *removeInstruction(ICGElm *inst){
	ICGElm *ret = (ICGElm*)inst->next;

	if(inst->prev != NULL){
		((ICGElm*)inst->prev)->next = (void*)ret;
	}
	if(ret != NULL){
		ret->prev = inst->prev;
	}
	
	inst->next = NULL;
	inst->prev = NULL;
	freeICGElm(inst);
	return ret;
}

MethodAnalysis *performMethodAnalysis(ICGElm** root){
	ICGElm *ptr = *root;
	MethodAnalysis *ma = newMethodAnalysis();

	Variable *written;
	int idxR = 0;
	Variable *read[3];
	

	while(ptr != NULL){
		if(ptr->typ == ICG_NONE || ptr->typ == ICG_LITERAL || ptr->typ == ICG_IDENT){
			if(ptr == *root){
				ptr = removeInstruction(ptr);
				*root = ptr;
			}else{
				ptr = removeInstruction(ptr);
			}
			continue;
		}
		if(ptr->typ == ICG_LBL){
			ptr = (ICGElm*)ptr->next;
			continue;
		}

		idxR = 0;
		read[0] = read[1] = read[2] = NULL;
		written = NULL;

		// First: Assume standard OP <dest> <src> <src> notation
		if(ptr->result != NULL && ptr->result->typ == ICGO_REG){
			written = (Variable*)ptr->result->data;
		}
		if(ptr->op1 != NULL && ptr->op1->typ == ICGO_REG){
			read[idxR] = (Variable*)ptr->op1->data;
			idxR ++;
		}
		if(ptr->op2 != NULL && ptr->op2->typ == ICGO_REG){
			read[idxR] = (Variable*)ptr->op2->data;
			idxR ++;
		}
		if(ptr->op3 != NULL && ptr->op3->typ == ICGO_REG){
			read[idxR] = (Variable*)ptr->op3->data;
			idxR ++;
		}

		// Now handle execptions
		if(ptr->typ == ICG_STOREH){
			// first param is read from (an address)
			read[idxR] = written;
			idxR ++;
			written = NULL;
		}
		if(ptr->typ == ICG_WRITE_METHOD_PTR){
			read[idxR] = written;
			idxR ++;
			written = NULL;
		}
		if(ptr->typ == ICG_RET){
			read[idxR] = written;
			idxR ++;
			written = NULL;
		}
		if(ptr->typ == ICG_PUSH){
			read[idxR] = written;
			idxR ++;
			written = NULL;
		}

		/*printf("i: ");
		printSingleICGElm(ptr, stdout);
		printf("\n");

		if(written != NULL){
			printf(" * W: %s (%d)\n", written->refname, written->uuid);
		}
		if(read[0] != NULL){
			printf(" * R: %s (%d)\n", read[0]->refname, read[0]->uuid);
		}
		if(read[1] != NULL){
			printf(" * R: %s (%d)\n", read[1]->refname, read[1]->uuid);
		}
		if(read[2] != NULL){
			printf(" * R: %s (%d)\n", read[2]->refname, read[2]->uuid);
		}*/


		// Now we know which registers are read/written, so perform analysis
		VariableFlowTracker *tracker;
		if(written != NULL){
			HASH_FIND_PTR(ma->variableflow, &written, tracker);
			if(tracker == NULL){
				tracker = newVariableFlowTracker(written, ptr);
				HASH_ADD_PTR(ma->variableflow, var, tracker);
			}else if(tracker->flagLocal && !inSameLocalBlock(tracker->lastAccess, ptr)){
				tracker->flagLocal = false;
			}
			tracker->writes ++;
			tracker->lastWrite = ptr;
			tracker->lastAccess = ptr;
		}
		for(int i=0; i<3; ++i){
			if(read[i] != NULL){
				HASH_FIND_PTR(ma->variableflow, &read[i], tracker);
				if(tracker == NULL){
					tracker = newVariableFlowTracker(read[i], ptr);
					HASH_ADD_PTR(ma->variableflow, var, tracker);
				}else if(tracker->flagLocal && !inSameLocalBlock(tracker->lastAccess, ptr)){
					tracker->flagLocal = false;
				}
				tracker->reads ++;
				tracker->lastRead = ptr;
				tracker->lastAccess = ptr;
			}
		}



		ptr = (ICGElm*)ptr->next;
	}


	ma->root = *root;
	return ma;
}

OptimizationCounter performMethodICGOptimization(ICGElm** root){
	OptimizationCounter stats = newOptimizationCounter();

	ICGElm *ptr = *root;
	MethodAnalysis *ma = performMethodAnalysis(root);
	
	

	// Now go back and perform optimizations

	VariableFlowTracker *tracker, *tmp;
	HASH_ITER(hh, ma->variableflow, tracker, tmp) {
		if(tracker->flagLocal)
			printf(" * VVV [LOCAL] %s (%d) R%d W%d\n", tracker->var->refname, tracker->var->uuid, tracker->reads, tracker->writes);
		else
			printf(" * VVV [     ] %s (%d) R%d W%d\n", tracker->var->refname, tracker->var->uuid, tracker->reads, tracker->writes);

		// written, but never read : why write?
		if(tracker->reads == 0 && !(OPT_ZEROREAD_BLACKLIST(tracker->lastWrite))){
			stats.removeZeroRead ++;
			removeInstruction(tracker->lastWrite);
		}
    }


	// now do local optimization
	ptr = *root;
	while(ptr != NULL){
		ptr = localOptimizeBlock(ptr, ma, &stats);
		if(ptr != NULL){
			ptr = (ICGElm*)ptr->next;
		}
	}

	freeMethodAnalysis(ma);
	return stats;
}

UT_array *performICGOptimization(UT_array *funcs, FILE *outfl){
	// Local optimizations take place in each section of code without any
	// jumps or labels

	OptimizationCounter netStats = newOptimizationCounter();

	ICGElm **p;
	for(p=(ICGElm**)utarray_front(funcs); p!=NULL; p=(ICGElm**)utarray_next(funcs,p)) {
		OptimizationCounter stat = performMethodICGOptimization(p);
		netStats = sumOptimizationCounter(netStats, stat);
	}


	printf("POST OPTIMIZATIONS\n");
	printf(" -- removeZeroRead: %d\n", netStats.removeZeroRead);

	UT_array *ret;
	utarray_new(ret, &ut_ptr_icd);
	for(p=(ICGElm**)utarray_front(funcs); p!=NULL; p=(ICGElm**)utarray_next(funcs,p)) {

		MethodAnalysis *ma = performMethodAnalysis(p);
		utarray_push_back(ret, &ma);

		printICG(*p, outfl, false);
		printICG(*p, stdout, true);
	}

	return ret;

/*
	while(icgroot != NULL){
		icgroot = localOptimizeBlock(icgroot);
		if(icgroot != NULL){
			icgroot = (ICGElm*)icgroot->next;
		}
	}*/
}


