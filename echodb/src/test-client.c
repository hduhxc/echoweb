#include <stdio.h>
#include <stdlib.h>
#include "client.h"

int main()
{
	echodb_connect("127.0.0.1", 6666);

	RespStat resp;
	unsigned int val_len;
	char* val;

	resp = echodb_set(5, "Hello", 5, "World");
	printf("%d\n", resp);
	resp = echodb_get(5, "Hello", &val_len, &val);
	printf("%d\n", resp);
	free(val);

	resp = echodb_update(5, "Hello", 5, "Japan");
	printf("%d\n", resp);
	resp = echodb_get(5, "Hello", &val_len, &val);
	printf("%d\n", resp);
	free(val);

	//resp = echodb_delete(5, "Hello");
	//printf("%d\n", resp);
	//resp = echodb_get(5, "Hello", &val_len, &val);
	//printf("%d\n", resp);

	echodb_close();

	return 0;
}
