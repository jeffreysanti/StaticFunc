/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  io.h
 *  Error Reporting & In/Output Handler
 *
 */

#include "io.h"
#include "parse.h"
#include "types.h"

void fatalError( const char* format, ... ) {
	fprintf(stderr, "FATAL: ");
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args );
    exit(EXIT_FAILURE);
}

void reportError(const char *code, const char* format, ... ) {
	fprintf(stderr, "ERROR [%s]: ", code);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args );

    fprintf(stderr, "\n");
}

bool reportParseError(void *ps, const char *code, const char* format, ... ) {

	((PState*) ps)->err ++;

	fprintf(stderr, "ERROR [%s]: ", code);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args );

    fprintf(stderr, "\n");
    return false;
}

void errShowType(const char* prefix, void * typ) {
	fprintf(stderr, "\t  %s", prefix);
	char *t = getTypeAsString(*((Type*)typ));
	fprintf(stderr, "%s\n", t);
	free(t);
}

void errRaw(const char* str) {
	fprintf(stderr, "%s", str);
}

void errShowTypeStr(const char* prefix, const char *str) {
	fprintf(stderr, "\t  %s%s\n", prefix, str);
}
