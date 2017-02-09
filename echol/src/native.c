#include <stdio.h>
#include <string.h>
#include "native.h"
#include "client.h"

#define MAX_BUF_SIZE 1024

typedef GSList NativeFuncMapList;

NativeFuncMapList* native_func_map_list = NULL;

FuncValue __string_to_int(gint argc,
						  FuncValue* argv)
{
	FuncValue ret;
	if (argc != 1) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING) {
		ret.type = TYPE_VOID;
		return ret;
	}

	gchar* str;
	gint val;
	str = argv[0].value.string->str;
	sscanf(str, "%d", &val);

	ret.type = TYPE_INT;
	ret.value.int_value = val;
	return ret;
}

FuncValue __fread(gint argc,
				  FuncValue* argv)
{
	FuncValue ret;
	if (argc != 1) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_INT) {
		ret.type = TYPE_VOID;
		return ret;
	}

	gint len;
	gchar* str;
	len = argv[0].value.int_value;
	str = g_alloca(len);

	guint count = 0;
	gchar* p = str;
	gint nr;

	while (count < len) {
		nr = fread(p, 1, len - count, stdin);
		if (nr <= 0) {
			break;
		}

		count += nr;
		p += nr;
	}

	ret.type = TYPE_STRING;
	ret.value.string = g_string_new_len(str, len);
	return ret;
}

FuncValue __getenv(gint argc,
				   FuncValue* argv)
{
	FuncValue ret;
	if (argc != 1) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING) {
		ret.type = TYPE_VOID;
		return ret;
	}

	const gchar* name;
	const gchar* val;
	name = argv[0].value.string->str;
	val = g_getenv(name);

	if (val == NULL) {
		ret.type = TYPE_BOOL;
		ret.value.bool_value = FALSE;
	} else {
		ret.type = TYPE_STRING;
		ret.value.string = g_string_new(val);
	}
	
	return ret;
}

FuncValue __strcat(gint argc,
				   FuncValue* argv)
{
	FuncValue ret;
	if (argc != 2) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING
	 || argv[1].type != TYPE_STRING) {
		ret.type = TYPE_VOID;
		return ret;
	}

	GString* str1 = argv[0].value.string;
	GString* str2 = argv[1].value.string;

	ret.type = TYPE_STRING;
	ret.value.string = g_string_append(str1, str2->str);

	return ret;
}

FuncValue __printf(gint argc,
				   FuncValue* argv)
{
	FuncValue ret;
	if (argc != 1) {
		ret.type = TYPE_VOID;
		return ret;
	}

	switch (argv[0].type) {
		case TYPE_BOOL: {
				if (argv[0].value.bool_value == TRUE) {
					printf("TRUE");
				} else {
					printf("FALSE");
				}
				break;
			}
		case TYPE_INT: {
				printf("%d", argv[0].value.int_value);
				break;
			}
		case TYPE_STRING: {
				printf("%s", argv[0].value.string->str);
				break;
			}
		default:
			break;
	}

	ret.type = TYPE_VOID;
	return ret;
}

FuncValue __echodb_connect(gint argc,
						   FuncValue* argv)
{
	FuncValue ret;
	if (argc != 2) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING
	 || argv[1].type != TYPE_INT) {

		ret.type = TYPE_VOID;
		return ret;
	}

	gchar* address;
	guint port;
	address = argv[0].value.string->str;
	port = argv[1].value.int_value;

	echodb_connect(address, port);

	ret.type = TYPE_VOID;
	return ret;
}

FuncValue __echodb_get(gint argc,
					   FuncValue* argv)
{
	FuncValue ret;
	if (argc != 1) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING) {
		ret.type = TYPE_VOID;
		return ret;
	}

	gchar* key = argv[0].value.string->str;
	guint key_len = strlen(key);
	gchar* val;
	guint val_len;
	RespStat stat;

	stat = echodb_get(key_len, key, &val_len, &val);
	if (stat != RESP_STAT_OK) {
		ret.type = TYPE_BOOL;
		ret.value.bool_value = FALSE;
		return ret;
	}

	ret.type = TYPE_STRING;
	ret.value.string = g_string_new_len(val, val_len);
	return ret;
}

FuncValue __echodb_set(gint argc,
					   FuncValue* argv)
{
	FuncValue ret;
	if (argc != 2) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING
	 || argv[1].type != TYPE_STRING) {

		ret.type = TYPE_VOID;
		return ret;
	}

	gchar* key = argv[0].value.string->str;
	guint key_len = strlen(key);
	gchar* val = argv[1].value.string->str;
	guint val_len = strlen(val);
	RespStat stat;

	stat = echodb_set(key_len, key, val_len, val);
	if (stat != RESP_STAT_OK) {
		ret.type = TYPE_BOOL;
		ret.value.bool_value = FALSE;
		return ret;
	}

	ret.type = TYPE_BOOL;
	ret.value.bool_value = TRUE;
	return ret;
}

FuncValue __echodb_update(gint argc,
						  FuncValue* argv)
{
	FuncValue ret;
	if (argc != 2) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING
	 || argv[1].type != TYPE_STRING) {

		ret.type = TYPE_VOID;
		return ret;
	}

	gchar* key = argv[0].value.string->str;
	guint key_len = strlen(key);
	gchar* val = argv[1].value.string->str;
	guint val_len = strlen(val);
	RespStat stat;

	stat = echodb_update(key_len, key, val_len, val);
	if (stat != RESP_STAT_OK) {
		ret.type = TYPE_BOOL;
		ret.value.bool_value = FALSE;
		return ret;
	}

	ret.type = TYPE_BOOL;
	ret.value.bool_value = TRUE;
	return ret;
}

FuncValue __echodb_delete(gint argc,
					      FuncValue* argv)
{
	FuncValue ret;
	if (argc != 1) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING) {
		ret.type = TYPE_VOID;
		return ret;
	}

	gchar* key = argv[0].value.string->str;
	guint key_len = strlen(key);
	RespStat stat;

	stat = echodb_delete(key_len, key);
	if (stat != RESP_STAT_OK) {
		ret.type = TYPE_BOOL;
		ret.value.bool_value = FALSE;
		return ret;
	}

	ret.type = TYPE_BOOL;
	ret.value.bool_value = TRUE;
	return ret;
}

FuncValue __echodb_close(gint argc,
						 FuncValue* argv)
{
	FuncValue ret;
	if (argc != 0) {
		ret.type = TYPE_VOID;
		return ret;
	}

	echodb_close();

	ret.type = TYPE_BOOL;
	ret.value.bool_value = TRUE;
	return ret;
}

gchar* unescape_string(gchar* str)
{
	gchar* p;
	p = str;

	while (*p != '\0') {

		if (*p == '+') {
			*p = ' ';
		}
		p++;
	}
	return g_uri_unescape_string(str, NULL);
}

FuncValue __parse_query(gint argc,
						FuncValue* argv)
{
	FuncValue ret;
	if (argc != 2) {
		ret.type = TYPE_VOID;
		return ret;
	}

	if (argv[0].type != TYPE_STRING
	 || argv[1].type != TYPE_STRING) {

		ret.type = TYPE_VOID;
		return ret;
	}

	gchar** split;
	split = g_strsplit_set(argv[0].value.string->str, "=&", 0);

	while (*split != NULL) {
		gchar* query_key;
		gchar* key;
		query_key = argv[1].value.string->str;
		key = unescape_string(*split);

		if (strcmp(query_key, key) == 0) {
			split++;
			break;
		}
		split += 2;
	}

	if (*split == NULL) {
		ret.type = TYPE_BOOL;
		ret.value.bool_value = FALSE;
	} else {
		gchar* val;
		val = unescape_string(*split);
		ret.type = TYPE_STRING;
		ret.value.string = g_string_new(val);
	}

	return ret;
}

gint identifier_comp_func(gconstpointer a,
						  gconstpointer b)
{
	NativeFuncMap* map_a = (NativeFuncMap*)a;
	GString* str_a = map_a->identifier;
	GString* str_b = (GString*)b;

	if (g_string_equal(str_a, str_b) == TRUE) {
		return 0;
	}
	return 1;
}

NativeFunc* search_native_func(GString* identifier)
{
	GSList* list;
	NativeFuncMap* map;
	list = g_slist_find_custom(native_func_map_list, identifier, identifier_comp_func);
	map = (NativeFuncMap*)list->data;

	return map->func;
}

void register_native_func(gchar* identifier,
						  NativeFunc* func)
{
	GString* str;
	NativeFuncMap* map;
	str = g_string_new(identifier);
	map = g_new(NativeFuncMap, 1);
	map->identifier = str;
	map->func = func;

	native_func_map_list = g_slist_append(native_func_map_list, map);
}

void init_native_func()
{
	register_native_func("strcat", __strcat);
	register_native_func("print", __printf);
	register_native_func("echodb_connect", __echodb_connect);
	register_native_func("echodb_get", __echodb_get);
	register_native_func("echodb_set", __echodb_set);
	register_native_func("echodb_update", __echodb_update);
	register_native_func("echodb_delete", __echodb_delete);
	register_native_func("echodb_close", __echodb_close);
	register_native_func("getenv", __getenv);
	register_native_func("read", __fread);
	register_native_func("string_to_int", __string_to_int);
	register_native_func("parse_query", __parse_query);
}
