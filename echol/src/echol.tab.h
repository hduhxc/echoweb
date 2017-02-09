/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_ECHOL_TAB_H_INCLUDED
# define YY_YY_ECHOL_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    FUNCTION = 258,
    IF = 259,
    ELSE = 260,
    ELIF = 261,
    WHILE = 262,
    FOR = 263,
    CONTINUE = 264,
    BREAK = 265,
    RETURN = 266,
    V_NULL = 267,
    V_TRUE = 268,
    V_FALSE = 269,
    L_BRACKET = 270,
    R_BRACKET = 271,
    L_BRACE = 272,
    R_BRACE = 273,
    COMMA = 274,
    SEMICOLON = 275,
    ASSIGN = 276,
    EQ = 277,
    NEQ = 278,
    LE = 279,
    GR = 280,
    AND = 281,
    OR = 282,
    NOT = 283,
    NLE = 284,
    NGR = 285,
    ADD = 286,
    SUB = 287,
    MUL = 288,
    DIV = 289,
    MOD = 290,
    ADD_ASSIGN = 291,
    SUB_ASSIGN = 292,
    MUL_ASSIGN = 293,
    DIV_ASSIGN = 294,
    MOD_ASSIGN = 295,
    CONCAT = 296,
    STRING = 297,
    IDENTIFIER = 298,
    INT_VALUE = 299,
    DOUBLE_VALUE = 300
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 9 "echol.y" /* yacc.c:1909  */

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

#line 113 "echol.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ECHOL_TAB_H_INCLUDED  */
