/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  icgDecl.c
 *  Intermediate Code Generator : Assignments
 *
 */

#include "icg.h"

ICGElm * icGenAssnToX(PTree *root, ICGElm *prev, char *to, Type assignType){
	char *dest = malloc(strlen(to)+1);
	strcpy(dest, to);

	ICGElm *data = icGen(root, prev);
	if(data->typ == ICG_LITERAL){
		freeICGElm(data);
		char *src = malloc(strlen((char *)root->tok->extra)+1);
		strcpy(src, (char *)root->tok->extra);

		if(assignType.base == TB_NATIVE_INT8 || assignType.base == TB_NATIVE_BOOL){
			prev = newICGElm(prev, ICG_MOVL_INT8, (PTree*)root->parent);
			prev->result = dest;
			prev->op1 = src;
		}else if(assignType.base == TB_NATIVE_INT16){
			prev = newICGElm(prev, ICG_MOVL_INT16, (PTree*)root->parent);
			prev->result = dest;
			prev->op1 = src;
		}else if(assignType.base == TB_NATIVE_INT32){
			prev = newICGElm(prev, ICG_MOVL_INT32, (PTree*)root->parent);
			prev->result = dest;
			prev->op1 = src;
		}else if(assignType.base == TB_NATIVE_INT64){
			prev = newICGElm(prev, ICG_MOVL_INT64, (PTree*)root->parent);
			prev->result = dest;
			prev->op1 = src;
		}else if(assignType.base == TB_NATIVE_FLOAT32){
			prev = newICGElm(prev, ICG_MOVL_FLOAT32, (PTree*)root->parent);
			prev->result = dest;
			prev->op1 = src;
		}else if(assignType.base == TB_NATIVE_FLOAT64){
			prev = newICGElm(prev, ICG_MOVL_FLOAT64, (PTree*)root->parent);
			prev->result = dest;
			prev->op1 = src;
		}else{
			fatalError("Unknown Literal Assignment!");
		}
	}else{
		// TODO
	}
	return prev;
}

void icGenAssn_print(ICGElm *elm, FILE* f)
{
	if(elm->typ == ICG_MOVL_INT8){
		fprintf(f, "movli8 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_INT16){
		fprintf(f, "movli16 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_INT32){
		fprintf(f, "movli32 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_INT64){
		fprintf(f, "movli64 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_FLOAT32){
		fprintf(f, "movlf32 %s, %s", (char*)elm->result, (char*)elm->op1);
	}else if(elm->typ == ICG_MOVL_FLOAT64){
		fprintf(f, "movlf64 %s, %s", (char*)elm->result, (char*)elm->op1);
	}
}
