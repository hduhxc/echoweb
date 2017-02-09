%{
#include "semantic.h"
#include "echol.tab.h"

void yyerror (char const *s)
{
  fprintf (stderr, "%s\n", s);
}

%}
%option noyywrap
%start EL COMMENT STRING_LITERAL
%%

<INITIAL>"<?el"		BEGIN EL;

<EL>"?>"			BEGIN INITIAL;
<EL>"function"      return FUNCTION;
<EL>"if"            return IF;
<EL>"else"          return ELSE;
<EL>"elif"          return ELIF;
<EL>"while"         return WHILE;
<EL>"for"           return FOR;
<EL>"continue"      return CONTINUE;
<EL>"break"         return BREAK;
<EL>"return"        return RETURN;
<EL>"null"          return V_NULL;
<EL>"TRUE"			return V_TRUE;
<EL>"FALSE"			return V_FALSE;
<EL>"("             return L_BRACKET;
<EL>")"             return R_BRACKET;
<EL>"{"             return L_BRACE;
<EL>"}"             return R_BRACE;
<EL>","             return COMMA;
<EL>";"             return SEMICOLON;
<EL>":="            return ASSIGN;
<EL>"="             return EQ;
<EL>"!="            return NEQ;
<EL>"<"             return LE;
<EL>">"             return GR;
<EL>"&&"            return AND;
<EL>"||"            return OR;
<EL>"!"             return NOT;
<EL>">="            return NLE;
<EL>"<="            return NGR;
<EL>"+"             return ADD;
<EL>"-"             return SUB;
<EL>"*"             return MUL;
<EL>"/"             return DIV;
<EL>"%"             return MOD;
<EL>"+="            return ADD_ASSIGN;
<EL>"-="            return SUB_ASSIGN;
<EL>"*="            return MUL_ASSIGN;
<EL>"/="            return DIV_ASSIGN;
<EL>"%="            return MOD_ASSIGN;
<EL>"."				return CONCAT;

<EL>\"				BEGIN STRING_LITERAL;
<EL>"//"			BEGIN COMMENT;

<EL>[A-Za-z_][0-9A-Za-z_]* {
	yylval.string = create_string(yytext);
	return IDENTIFIER;
}

<EL>([1-9][0-9]*)|"0" {
	sscanf(yytext, "%d", &yylval.int_value);
	return INT_VALUE;
}

<EL>(([1-9][0-9]*)|"0")\.[0-9]+ {
	sscanf(yytext, "%lf", &yylval.double_value);
	return DOUBLE_VALUE;
}

<EL>[ \t\n]			;

<EL>.               ;

<STRING_LITERAL>[^"]* {
	yylval.string = create_string(yytext);
	return STRING;
}

<STRING_LITERAL>\"	BEGIN EL;

<COMMENT>\n			return COMMENT;

<COMMENT>.			;

%%
