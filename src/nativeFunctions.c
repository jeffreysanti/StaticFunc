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

Type sig_string_int8(){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 1);
	((Type*)typ.children)[0] = newBasicType(TB_NATIVE_STRING);
	((Type*)typ.children)[1] = newBasicType(TB_NATIVE_INT8);
	return typ;
}Type sig_string_int16(){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 1);
	((Type*)typ.children)[0] = newBasicType(TB_NATIVE_STRING);
	((Type*)typ.children)[1] = newBasicType(TB_NATIVE_INT16);
	return typ;
}Type sig_string_int32(){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 1);
	((Type*)typ.children)[0] = newBasicType(TB_NATIVE_STRING);
	((Type*)typ.children)[1] = newBasicType(TB_NATIVE_INT32);
	return typ;
}Type sig_string_int64(){
	Type typ = newBasicType(TB_FUNCTION);
	allocTypeChildren(&typ, 1 + 1);
	((Type*)typ.children)[0] = newBasicType(TB_NATIVE_STRING);
	((Type*)typ.children)[1] = newBasicType(TB_NATIVE_INT64);
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

	registerNativeFunction("string", sig_string_int8());
	registerNativeFunction("string", sig_string_int16());
	registerNativeFunction("string", sig_string_int32());
	registerNativeFunction("string", sig_string_int64());

	registerNativeFunction("range", sig_vecx_x_x(TB_NATIVE_INT8));
	registerNativeFunction("range", sig_vecx_x_x(TB_NATIVE_INT16));
	registerNativeFunction("range", sig_vecx_x_x(TB_NATIVE_INT32));
	registerNativeFunction("range", sig_vecx_x_x(TB_NATIVE_INT64));


}

