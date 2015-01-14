/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  prep.h
 *  Preprocessor processor
 *
 */


#ifndef STATICFUNC_SRC_PREP_H_
#define STATICFUNC_SRC_PREP_H_

#include <string.h>
#include "lex.h"
#include "parse.h"
#include "io.h"

bool parsePreprocessor(char *str, int len);


#endif /* STATICFUNC_SRC_PREP_H_ */
