/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  io.h
 *  Error Reporting & In/Output Handler
 *
 */

#include "io.h"


void fatalError( const char* format, ... ) {
	fprintf(stderr, "FATAL: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args );
    exit(EXIT_FAILURE);
}

extern void reportError(const char *code, const char* format, ... ) {
	fprintf(stderr, "ERROR [%s]: ", code);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args );
}

