#include <string.h>
#include "semantic.h"
#include "binary_tree.h"
#include "native.h"

VariableList* var_list;
extern Env global;

FuncValue parse_func_call(FuncCallStatement* statement);
void parse_assign(AssignStatement* statement);
void parse_if(IfStatement* statement);
void parse_while(WhileStatement* statement);
void parse_for(ForStatement* statement);

gint compare_variable(gconstpointer a,
					  gconstpointer b)
{
	const GString* name_a = ((Variable*)a)->name;
	const GString* name_b = b;
	gboolean ans = g_string_equal(name_a, name_b);

	return ans == TRUE ? 0 : 1;
}

Variable* search_variable(GString* var)
{
	GSList* list = g_slist_find_custom(var_list, var, compare_variable);
	return list == NULL ? NULL : list->data;
}

gboolean calc_bool(union BasicValue left,
				   union BasicValue right,
				   enum ExpressionType type)
{
	switch (type) {
		case EXPR_OR: return left.bool_value || right.bool_value;
		case EXPR_AND: return left.bool_value && right.bool_value;
		case EXPR_LE: return left.int_value < right.int_value;
		case EXPR_GR: return left.int_value > right.int_value;
		case EXPR_NLE: return left.int_value >= right.int_value;
		case EXPR_NGR: return left.int_value <= right.int_value;
	}
}

gint calc_int(union BasicValue left,
			  union BasicValue right,
			  enum ExpressionType type)
{ 
	switch (type) {
		case EXPR_ADD: return left.int_value + right.int_value;
		case EXPR_SUB: return left.int_value - right.int_value;
		case EXPR_MUL: return left.int_value * right.int_value;
		case EXPR_DIV: return left.int_value / right.int_value;
		case EXPR_MOD: return left.int_value % right.int_value;
	}
}

void traverse_exp_func(BinaryTree* tree)
{
	Expression* exp = tree->data;
	ExpressionDef def = exp->exp_def;

	if (tree->left == NULL
	 && tree->right == NULL) {

		switch (def.type) {
			case EXPR_INT: {
					exp->result_type = TYPE_INT;
					exp->result_value.int_value = def.v.int_value;
					break;
				}
			case EXPR_STRING: {
					exp->result_type = TYPE_STRING;
					exp->result_value.string = def.v.string;
					break;
				}
			case EXPR_VARIABLE: {
					GString* var_name;
					Variable* var;
					var_name = def.v.identifier;
					var = search_variable(var_name);

					switch (var->type) {
						case TYPE_INT: {
								exp->result_type = TYPE_INT;
								exp->result_value.int_value = var->v.int_value;
								break;
							}
						case TYPE_STRING: {
								exp->result_type = TYPE_STRING;
								exp->result_value.string = var->v.string;
								break;
							}
						case TYPE_BOOL: {
								exp->result_type = TYPE_BOOL;
								exp->result_value.bool_value = var->v.bool_value;
								break;
							}
					}
					break;
				}
			case EXPR_BOOL: {
					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = def.v.bool_value;
					break;
				}
			case EXPR_FUNC_CALL: {
					FuncValue ret;
					ret = parse_func_call(def.v.func_call->value);
					exp->result_type = ret.type;
					exp->result_value = ret.value;
				}

			default:
				break;
		}
		
		return;
	}
	Expression* left = tree->left->data;
	ExpressionDef left_def = left->exp_def;
	Expression* right = tree->right->data;
	ExpressionDef right_def = right->exp_def;

	switch (def.type) {
		case EXPR_OR:
		case EXPR_AND: {
				if (left->result_type == TYPE_BOOL
				 && right->result_type == TYPE_BOOL) {

					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = calc_bool(left->result_value, right->result_value, def.type);
				} else {
					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = FALSE;
				}
				break;
			}
		case EXPR_EQ: {
				if ((left->result_type == TYPE_BOOL && right->result_type == TYPE_BOOL)
				 || (left->result_type == TYPE_INT && right->result_type == TYPE_INT)) {

					exp->result_type = TYPE_BOOL;
					if (left->result_type == TYPE_INT) {
						exp->result_value.bool_value = left->result_value.int_value == right->result_value.int_value;
					} else {
						exp->result_value.bool_value = left->result_value.bool_value == right->result_value.bool_value;
					}
				} else {
					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = FALSE;
				}
				break;
			}
		case EXPR_NEQ: {
				if ((left->result_type == TYPE_BOOL && right->result_type == TYPE_BOOL)
				 || (left->result_type == TYPE_INT && right->result_type == TYPE_INT)) {

					exp->result_type = TYPE_BOOL;
					if (left->result_type == TYPE_INT) {
						exp->result_value.bool_value = left->result_value.int_value != right->result_value.int_value;
					} else {
						exp->result_value.bool_value = left->result_value.bool_value != right->result_value.bool_value;
					}
				} else {
					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = FALSE;
				}
				break;
			}
		case EXPR_LE:
		case EXPR_GR:
		case EXPR_NLE:
		case EXPR_NGR: {
				if (left->result_type == TYPE_INT
				 && right->result_type == TYPE_INT) {
					
					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = calc_bool(left->result_value, right->result_value, def.type);
				} else {
					exp->result_type = TYPE_BOOL;
					exp->result_value.bool_value = FALSE;
				}
				break;
			}
		case EXPR_ADD:
		case EXPR_SUB:
		case EXPR_MUL:
		case EXPR_DIV:
		case EXPR_MOD: {
				if (left->result_type == TYPE_INT
				 && right->result_type == TYPE_INT) {

					exp->result_type = TYPE_INT;
					exp->result_value.int_value = calc_int(left->result_value, right->result_value, def.type);
				}
				break;
			}
		case EXPR_CONCAT: {
				if (left->result_type == TYPE_STRING
				 && right->result_type == TYPE_STRING) {

					NativeFunc* func;
					gint argc = 2;
					FuncValue* argv = g_new(FuncValue, 2);

					argv[0].type = left->result_type;
					argv[0].value.string = left->result_value.string;
					argv[1].type = right->result_type;
					argv[1].value.string = right->result_value.string;

					FuncValue ret;
					func = search_native_func(g_string_new("strcat"));
					ret = func(argc, argv);

					exp->result_type = ret.type;
					exp->result_value.string = ret.value.string;
				}
				break;
			}
		default:
			break;
	}
}

void parse_expression(BinaryTree* exp)
{
	tree_traverse_root_last(exp, traverse_exp_func);
}

void parse_assign(AssignStatement* statement)
{
	Variable* var;
	GString* var_name = statement->variable;
	Expression* exp = statement->exp_tree->data;
	var = search_variable(var_name);

	if (var == NULL) {
		var = g_new(Variable, 1);
		var->name = var_name;
		var->type = TYPE_VOID;
		var_list = g_slist_prepend((GSList*)var_list, var);
	}
	parse_expression(statement->exp_tree);

	switch (statement->type) {
		case OP_ASSIGN: {
				var->type = exp->result_type;
				var->v = exp->result_value;
				break;
			}
		default: {
				g_warning("Not supported");
				break;
			}
	}
}

void parse_statement(gpointer data,
					 gpointer user_data)
{
	Statement* statement = (Statement*)data;

	switch (statement->type) {
		case STAT_ASSIGN: 
			parse_assign(statement->value); break;
		case STAT_FUNC_CALL: 
			parse_func_call(statement->value); break;
		//case STAT_RETURN: 
		//	parse_return(statement->value); break;
		//case STAT_CONTINUE: 
		//	parse_continue(statement->value); break;
		//case STAT_BREAK: 
		//	parse_break(statement->value); break;
		case STAT_IF: 
			parse_if(statement->value); break;
		case STAT_FOR: 
			parse_for(statement->value); break;
		case STAT_WHILE: 
			parse_while(statement->value); break;
		default:
			g_warning("Error statement");
	}
}

FuncValue parse_func_call(FuncCallStatement* statement)
{
	GString* name;
	NativeFunc* func;
	name = statement->name;
	func = search_native_func(name);

	ExpressionList* argument;
	int argc;
	int i;
	FuncValue* argv;
	argument = statement->argument;
	argc = g_slist_length(argument);
	argv = g_new(FuncValue, argc);

	for (i = 0; i < argc; i++) {
		ExpressionTree* tree;
		tree = argument->data;

		Expression* exp;
		parse_expression(tree);
		exp = tree->data;
		argv[i].type = exp->result_type;
		argv[i].value = exp->result_value;

		argument = argument->next;
	}

	return func(argc, argv);
}

gboolean parse_elif(ElifBlockList* elif_list)
{
	while (elif_list != NULL) {
		ElifBlock* elif;
		ExpressionTree* exp_tree;
		Expression* exp;

		elif = elif_list->data;
		exp_tree = elif->condition;
		parse_expression(exp_tree);
		exp = exp_tree->data;

		if (exp->result_type != TYPE_BOOL) {
			return FALSE;
		}

		if (exp->result_value.bool_value == TRUE) {
			g_slist_foreach(elif->then_block, parse_statement, NULL);
			return TRUE;
		}
		
		elif_list = elif_list->next;
	}
	return FALSE;
}

void parse_if(IfStatement* statement)
{
	ExpressionTree* exp_tree;
	Expression* cond;
	exp_tree = statement->condition;
	parse_expression(exp_tree);
	cond = exp_tree->data;

	if (cond->result_type != TYPE_BOOL) {
		return;
	}

	if (cond->result_value.bool_value == TRUE) {
		g_slist_foreach(statement->then_block, parse_statement, NULL);
		return;
	}
	if (statement->elif != NULL) {
		if (parse_elif(statement->elif) == TRUE) {
			return;
		}
	}
	if (statement->else_block != NULL) {
		g_slist_foreach(statement->else_block, parse_statement, NULL);
	}
}

void parse_while(WhileStatement* statement)
{
	ExpressionTree* exp_tree;
	Expression* cond;
	exp_tree = statement->condition;

	while (TRUE) {
		parse_expression(exp_tree);
		cond = exp_tree->data;

		if (cond->result_type != TYPE_BOOL) {
			return;
		}
		if (cond->result_value.bool_value != TRUE) {
			return;
		}

		g_slist_foreach(statement->block, parse_statement, NULL);
	}
}

void parse_for(ForStatement* statement)
{
	Statement* init_assign = statement->init_assign;
	ExpressionTree* exp_tree = statement->condition;
	Statement* re_assign = statement->re_assign;
	StatementList* block = statement->block;
	Expression* exp;

	parse_statement(init_assign, NULL);
	
	while (TRUE) {
		parse_expression(exp_tree);
		exp = exp_tree->data;

		if (exp->result_type != TYPE_BOOL) {
			return;
		}
		if (exp->result_value.bool_value != TRUE) {
			return;
		}

		g_slist_foreach(block, parse_statement, NULL);
		parse_statement(re_assign, NULL);
	}
}

void run()
{
	g_slist_foreach(global.statement, parse_statement, global.variable);
}
