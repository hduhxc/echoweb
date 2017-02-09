#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <glib.h>
#include "binary_tree.h"

typedef GSList FunctionList, VariableList, ExpressionList,
StatementList, StringList, ElifBlockList;
typedef BinaryTree ExpressionTree;

enum BasicType {
	TYPE_INT,
	TYPE_DOUBLE,
	TYPE_STRING,
	TYPE_BOOL,
	TYPE_VOID
};
union BasicValue {
	gint		int_value;
	gdouble		double_value;
	GString*	string;
	gboolean	bool_value;
};

typedef struct Function {
	GString*			name;
	StringList*			param;
	StatementList*		block;
} Function;

enum StatementType {
	STAT_ASSIGN, STAT_FUNC_CALL,
	STAT_RETURN, STAT_CONTINUE,
	STAT_BREAK, STAT_IF,
	STAT_FOR, STAT_WHILE
};
typedef struct Statement {
	enum StatementType type;
	gpointer value;
} Statement;

typedef struct Expression Expression;
enum ExpressionType {
	EXPR_OR, EXPR_AND, EXPR_EQ, EXPR_NEQ, EXPR_LE,
	EXPR_GR, EXPR_NLE, EXPR_NGR, EXPR_ADD, EXPR_SUB,
	EXPR_MUL, EXPR_DIV, EXPR_MOD, EXPR_FUNC_CALL, EXPR_NOT,
	EXPR_INT, EXPR_DOUBLE, EXPR_STRING, EXPR_VARIABLE,
	EXPR_BOOL, EXPR_NULL, EXPR_CONCAT
};
typedef struct ExpressionDef {
	enum ExpressionType type;
	union {
		Statement* func_call;
		Expression* expression;
		gint int_value;
		gdouble double_value;
		GString* string;
		GString* identifier;
		gboolean bool_value;
	} v;
} ExpressionDef;

struct Expression {
	ExpressionDef exp_def;
	enum BasicType result_type;
	union BasicValue result_value;
};

typedef struct Env {
	FunctionList*		function;
	VariableList*		variable;
	StatementList*		statement;
} Env;

typedef struct Variable {
	GString* name;
	enum BasicType type;
	union BasicValue v;
} Variable;

typedef struct ElifBlock {
	ExpressionTree* condition;
	StatementList* then_block;
} ElifBlock;

typedef struct IfStatement {
	ExpressionTree* condition;
	ElifBlockList* elif;
	StatementList* then_block;
	StatementList* else_block;
} IfStatement;

enum AssignOpType {
	OP_ASSIGN,
	OP_ADD_ASSIGN,
	OP_SUB_ASSIGN,
	OP_MUL_ASSIGN,
	OP_DIV_ASSIGN,
	OP_MOD_ASSIGN
};
typedef struct AssignStatement {
	GString* variable;
	enum AssignOpType type;
	ExpressionTree* exp_tree;
} AssignStatement;

typedef struct FuncCallStatement {
	GString* name;
	ExpressionList* argument;
} FuncCallStatement;

typedef struct ForStatement {
	Statement* init_assign;
	ExpressionTree* condition;
	Statement* re_assign;
	StatementList* block;
} ForStatement;

typedef struct WhileStatement {
	ExpressionTree* condition;
	StatementList* block;
} WhileStatement;

void add_global_statement(Statement* statement);

void create_function(GString* name,
					 StringList* param,
					 StatementList* block);

StringList* create_param_list(StringList* param,
							  GString* variable);

ExpressionList* create_argument_list(ExpressionList* argument,
									 ExpressionTree* expression);

Statement* create_statement(enum StatementType type,
							gpointer value);

Statement* create_if_statement(ExpressionTree* condition,
							   StatementList* then_block,
							   ElifBlockList* elif,
							   StatementList* else_block);

ElifBlockList* create_elif_block(ElifBlockList* elif_block,
								 ExpressionTree* condition,
								 StatementList* then_block);

Statement* create_assign_statement(GString* variable,
								   enum AssignOpType type,
								   ExpressionTree* expr);

Statement* create_func_call_statement(GString* name,
									  ExpressionList* argument);

Statement* create_return_statement(void);

Statement* create_continue_statement(void);

Statement* create_break_statement(void);

Statement* create_for_statement(Statement* init_assign,
								ExpressionTree* condition,
								Statement* re_assign,
								StatementList* block);

Statement* create_while_statement(ExpressionTree* condition,
								  StatementList* block);

StatementList* create_statement_list(StatementList* statement_list,
									 Statement* statement);

ExpressionTree* create_expression(ExpressionDef expression,
								  ExpressionTree* left,
								  ExpressionTree* right);

GString* create_string(const char* str);

#endif
