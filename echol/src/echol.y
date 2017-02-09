%{
#include "semantic.h"
#define YYERROR_VERBOSE

int yylex();
void yyerror(const char* s);

%}
%union {
	gint				int_value;
	gdouble				double_value;
	GString*			string;
	gboolean			bool_value;
	StringList*			param_list;
	ExpressionList*		argument_list;
	Statement*			statement;
	ElifBlockList*		elif_list;
	StatementList*		statement_list;
	ExpressionTree*		expression;
}
%token FUNCTION IF ELSE ELIF WHILE FOR CONTINUE BREAK RETURN V_NULL
	   V_TRUE V_FALSE L_BRACKET R_BRACKET L_BRACE R_BRACE COMMA
	   SEMICOLON ASSIGN EQ NEQ LE GR AND OR
	   NOT NLE NGR ADD SUB MUL DIV MOD
	   ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
	   CONCAT
%token  <string>            STRING;
%token  <string>			IDENTIFIER;
%token  <int_value>         INT_VALUE;
%token  <double_value>      DOUBLE_VALUE;
%type   <param_list>        param_list;
%type   <argument_list>     argument_list;
%type   <statement>         statement if_statement assign_statement function_call_statement return_statement
							continue_statement break_statement for_statement while_statement;
%type   <elif_list>         elif_block;
%type   <statement_list>    block statement_list;
%type   <expression>        expression binary_expression;
%left ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%left OR
%left AND
%left EQ NEQ
%left LE GR NLE NGR
%left ADD SUB
%left MUL DIV MOD
%right NOT
%%
parse
	: parse_unit
	| parse parse_unit
	;
parse_unit 
	: function_definition
	| statement
	{
		add_global_statement($1);
	}
	;
block
	: L_BRACE R_BRACE
	{
		$$ = NULL;
	}
	| L_BRACE statement_list R_BRACE
	{
		$$ = $2;
	}
	;
statement_list
	: statement
	{
		$$ = create_statement_list(NULL, $1);
	}
	| statement_list statement
	{
		$$ = create_statement_list($1, $2);
	}
	;
statement
	: assign_statement SEMICOLON
	{
		$$ = $1;
	}
	| function_call_statement SEMICOLON
	{
		$$ = $1;
	}
	| return_statement SEMICOLON
	{
		$$ = $1;
	}
	| continue_statement SEMICOLON
	{
		$$ = $1;
	}
	| break_statement SEMICOLON
	{
		$$ = $1;
	}
	| if_statement
	{
		$$ = $1;
	}
	| for_statement
	{
		$$ = $1;
	}
	| while_statement
	{
		$$ = $1;
	}
	;
function_definition
	: FUNCTION IDENTIFIER L_BRACKET R_BRACKET block
	{
		create_function($2, NULL, $5);
	}
	| FUNCTION IDENTIFIER L_BRACKET param_list R_BRACKET block
	{
		create_function($2, $4, $6);
	}
	;
param_list
	: IDENTIFIER
	{
		$$ = create_param_list(NULL, $1);
	}
	| param_list COMMA IDENTIFIER
	{
		$$ = create_param_list($1, $3);
	}
	;
argument_list
	: expression
	{
		$$ = create_argument_list(NULL, $1);
	}
	| argument_list COMMA expression
	{
		$$ = create_argument_list($1, $3);
	}
	;
elif_block
	: ELIF L_BRACKET expression R_BRACKET block
	{
		$$ = create_elif_block(NULL, $3, $5);
	}
	| elif_block ELIF L_BRACKET expression R_BRACKET block
	{
		$$ = create_elif_block($1, $4, $6);
	}
if_statement
	: IF L_BRACKET expression R_BRACKET block
	{
		$$ = create_if_statement($3, $5, NULL, NULL);
	}
	| IF L_BRACKET expression R_BRACKET block elif_block
	{
		$$ = create_if_statement($3, $5, $6, NULL);
	}
	| IF L_BRACKET expression R_BRACKET block elif_block ELSE block
	{
		$$ = create_if_statement($3, $5, $6, $8);
	}
	| IF L_BRACKET expression R_BRACKET block ELSE block
	{
		$$ = create_if_statement($3, $5, NULL, $7);
	}
	;
assign_statement
	: IDENTIFIER ASSIGN expression
	{
		$$ = create_assign_statement($1, OP_ASSIGN, $3);
	}
	| IDENTIFIER ADD_ASSIGN expression
	{
		$$ = create_assign_statement($1, OP_ADD_ASSIGN, $3);
	}
	| IDENTIFIER SUB_ASSIGN expression
	{
		$$ = create_assign_statement($1, OP_SUB_ASSIGN, $3);
	}
	| IDENTIFIER MUL_ASSIGN expression
	{
		$$ = create_assign_statement($1, OP_MUL_ASSIGN, $3);
	}
	| IDENTIFIER DIV_ASSIGN expression
	{
		$$ = create_assign_statement($1, OP_DIV_ASSIGN, $3);
	}
	| IDENTIFIER MOD_ASSIGN expression
	{
		$$ = create_assign_statement($1, OP_MOD_ASSIGN, $3);
	}
	;
function_call_statement
	: IDENTIFIER L_BRACKET argument_list R_BRACKET
	{
		$$ = create_func_call_statement($1, $3);
	}
	| IDENTIFIER L_BRACKET R_BRACKET
	{
		$$ = create_func_call_statement($1, NULL);
	}
	;
return_statement
	: RETURN expression
	{
		$$ = create_return_statement();
	}
	;
continue_statement
	: CONTINUE
	{
		$$ = create_continue_statement();
	}
	;
break_statement
	: BREAK
	{
		$$ = create_break_statement();
	}
	;
for_statement
	: FOR L_BRACKET assign_statement COMMA expression COMMA assign_statement R_BRACKET block
	{
		$$ = create_for_statement($3, $5, $7, $9);
	}
	;
while_statement
	: WHILE L_BRACKET expression R_BRACKET block
	{
		$$ = create_while_statement($3, $5);
	}
	;
expression
	: binary_expression
	| function_call_statement
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_FUNC_CALL;
		expr_def.v.func_call = $1;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| L_BRACKET expression R_BRACKET
	{
		$$ = $2;
	}
	| NOT expression
	{
	}
	| INT_VALUE
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_INT;
		expr_def.v.int_value = $1;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| DOUBLE_VALUE
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_DOUBLE;
		expr_def.v.double_value = $1;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| STRING
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_STRING;
		expr_def.v.string = $1;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| IDENTIFIER
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_VARIABLE;
		expr_def.v.identifier = $1;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| V_NULL
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_NULL;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| V_TRUE
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_BOOL;
		expr_def.v.bool_value = TRUE;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	| V_FALSE
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_BOOL;
		expr_def.v.bool_value = FALSE;

		$$ = create_expression(expr_def, NULL, NULL);
	}
	;
binary_expression
	: expression OR expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_OR;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression AND expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_AND;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression EQ expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_EQ;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression NEQ expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_NEQ;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression LE expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_LE;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression GR expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_GR;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression NLE expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_NLE;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression NGR expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_NGR;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression ADD expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_ADD;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression SUB expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_SUB;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression MUL expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_MUL;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression DIV expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_DIV;

		$$ = create_expression(expr_def, $1, $3);
	}
	| expression MOD expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_MOD;

		$$ = create_expression(expr_def, $1, $3);
	}
	;
	| expression CONCAT expression
	{
		ExpressionDef expr_def;
		expr_def.type = EXPR_CONCAT;

		$$ = create_expression(expr_def, $1, $3);
	}
%%
