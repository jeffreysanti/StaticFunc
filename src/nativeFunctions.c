/*
 *  Jeffrey Santi
 *  Static_Func Compiler
 *
 *  nativeFunctions.c
 *  Writes Built-In Functions
 *
 */

#include "nativeFunctions.h"

Type sig_void_string(){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 1);
	((Type*)typ.children)[0] = newBasicType(TB_NATIVE_VOID);
	((Type*)typ.children)[1] = newBasicType(TB_NATIVE_STRING);
	return typ;
}

Type sig_string_int(){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 1);
	((Type*)typ.children)[0] = newBasicType(TB_NATIVE_STRING);
	((Type*)typ.children)[1] = newBasicType(TB_NATIVE_INT);
	return typ;
}

Type sig_vecx_x_x(TypeBase x){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 2);
	((Type*)typ.children)[0] = newVectorType(newBasicType(x));
	((Type*)typ.children)[1] = newBasicType(x);
	((Type*)typ.children)[2] = newBasicType(x);
	return typ;
}

void initalizeBuiltInFunctions()
{
	registerNativeFunction("println", sig_void_string());

	registerNativeFunction("string", sig_string_int());

	registerNativeFunction("range", sig_vecx_x_x(TB_NATIVE_INT));

}
