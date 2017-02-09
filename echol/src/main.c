#include <stdio.h>
#include "semantic.h"
#include "native.h"

extern Env global;
extern void run(void);
extern void g_type_init(void);

int main(int argc,
		 char* argv[])
{
	extern FILE* yyin;
	extern int yyparse(void);
	//extern int yydebug;
	//yydebug = 1;
	g_type_init();

	if (argc != 2) {
		g_error("Wrong Paramter number");
	}
	init_native_func();

	yyin = fopen(argv[1], "r");
	yyparse();

	//run();

	fclose(yyin);
	return 0;
}
