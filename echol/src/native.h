#ifndef __NATIVE_H
#define __NATIVE_H

#include "glib.h"
#include "semantic.h"

typedef struct FuncValue {
	enum BasicType type;
	union BasicValue value;
} FuncValue;

typedef FuncValue(NativeFunc)(gint, FuncValue*);

typedef struct NativeFuncMap {
	GString* identifier;
	NativeFunc* func;
} NativeFuncMap;

NativeFunc* search_native_func(GString* identifier);
void register_native_func(gchar* identifier, NativeFunc* func);
void init_native_func();

#endif
