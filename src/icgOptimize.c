
#include "icgOptimize.h"


ICGElm *localOptimizeBlock(ICGElm *ptr){
	printf("LocalEntry\n");
	while(ptr != NULL){
		if(ptr->typ == ICG_LBL || ptr->typ == ICG_JNZ || ptr->typ == ICG_JZ || ptr->typ == ICG_JMP || ptr->typ == ICG_JAL || ptr->typ == ICG_RET){
			return ptr;
		}

		// TODO: Optimize

		ptr = (ICGElm*)ptr->next;
	}
}

void performICGOptimization(UT_array *funcs, FILE *outfl){
	// Local optimizations take place in each section of code without any
	// jumps or labels


// TODO: Build dependency graph for this method's variables


/*
	while(icgroot != NULL){
		icgroot = localOptimizeBlock(icgroot);
		if(icgroot != NULL){
			icgroot = (ICGElm*)icgroot->next;
		}
	}*/
}


