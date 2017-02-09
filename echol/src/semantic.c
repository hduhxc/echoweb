#include "semantic.h"

Env global;

extern void parse_statement(gpointer, gpointer);

void add_global_statement(Statement* statement)
{
	//if (global.statement == NULL) {
	//	global.statement = statement;
	//} else {
	//	g_slist_concat(global.statement, statement);
	//}
	parse_statement(statement, NULL);
}

void create_function(GString* name,
					 StringList* param,
					 StatementList* block)
{
	Function* new_func = g_new(Function, 1);
	new_func->name = name;
	new_func->param = param;
	new_func->block = block;

	global.function = g_slist_prepend(global.function, new_func);
}

StringList* create_param_list(StringList* param,
							  GString* variable)
{
	return g_slist_append(param, variable);
}

ExpressionList* create_argument_list(ExpressionList* argument,
									 ExpressionTree* expression)
{
	return g_slist_append(argument, expression);
}

Statement* create_statement(enum StatementType type,
							gpointer value)
{
	Statement* new_statement = g_new(Statement, 1);
	new_statement->type = type;
	new_statement->value = value;

	return new_statement;
}

Statement* create_if_statement(ExpressionTree* condition,
							   StatementList* then_block,
							   ElifBlockList* elif,
							   StatementList* else_block)
{
	IfStatement* new_if = g_new(IfStatement, 1);
	new_if->condition = condition;
	new_if->elif = elif;
	new_if->then_block = then_block;
	new_if->else_block = else_block;

	return create_statement(STAT_IF, new_if);
}

ElifBlockList* create_elif_block(ElifBlockList* elif_block,
								 ExpressionTree* condition,
								 StatementList* then_block)
{
	ElifBlock* new_elif = g_new(ElifBlock, 1);
	new_elif->condition = condition;
	new_elif->then_block = then_block;

	return g_slist_append(elif_block, new_elif);
}

Statement* create_assign_statement(GString* variable,
								   enum AssignOpType type,
								   ExpressionTree* expr)
{
	AssignStatement* new_assign = g_new(AssignStatement, 1);
	new_assign->variable = variable;
	new_assign->type = type;
	new_assign->exp_tree = expr;

	return create_statement(STAT_ASSIGN, new_assign);
}

Statement* create_func_call_statement(GString* name,
									  ExpressionList* argument)
{
	FuncCallStatement* new_func_call = g_new(FuncCallStatement, 1);
	new_func_call->name = name;
	new_func_call->argument = argument;

	return create_statement(STAT_FUNC_CALL, new_func_call);
}

Statement* create_return_statement(void)
{
	return create_statement(STAT_RETURN, NULL);
}

Statement* create_continue_statement(void)
{
	return create_statement(STAT_CONTINUE, NULL);
}

Statement* create_break_statement(void)
{
	return create_statement(STAT_BREAK, NULL);
}

Statement* create_for_statement(Statement* init_assign,
								ExpressionTree* condition,
								Statement* re_assign,
								StatementList* block)
{
	ForStatement* new_for = g_new(ForStatement, 1);
	new_for->init_assign = init_assign;
	new_for->condition = condition;
	new_for->re_assign = re_assign;
	new_for->block = block;

	return create_statement(STAT_FOR, new_for);
}

Statement* create_while_statement(ExpressionTree* condition,
								  StatementList* block)
{
	WhileStatement* new_while = g_new(WhileStatement, 1);
	new_while->condition = condition;
	new_while->block = block;

	return create_statement(STAT_WHILE, new_while);
}

StatementList* create_statement_list(StatementList* statement_list,
									 Statement* statement)
{
	return g_slist_append(statement_list, statement);
}

ExpressionTree* create_expression(ExpressionDef expression,
								  ExpressionTree* left,
								  ExpressionTree* right)
{
	Expression* new_expr = g_new(Expression, 1);
	new_expr->exp_def = expression;

	return tree_new(left, right, new_expr);
}

GString* create_string(const char* str)
{
	return g_string_new(str);
}
