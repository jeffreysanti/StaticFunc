/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  io.h
 *  Error Reporting & In/Output Handler
 *
 */

#ifndef STATICFUNC_SRC_IO_H_
#define STATICFUNC_SRC_IO_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


typedef char bool;
#define true 1
#define false 0


extern void fatalError(const char* format, ... );
extern void reportError(const char *code, const char* format, ... );
extern bool reportParseError(void *ps, const char *code, const char* format, ... );
void errShowType(const char* prefix, void *typ);
void errShowTypeStr(const char* prefix, const char *str);

#endif /* STATICFUNC_SRC_IO_H_ */
