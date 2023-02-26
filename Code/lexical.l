%option yylineno
%option never-interactive
%option nounput
%{
	#include "syntax.tab.h"
	#include "ast.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>

	int yycolumn = 1;
	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line = yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn + yyleng - 1; \
		yycolumn += yyleng;

	int yywrap(void) {
		return 1;
	}

	int atoi_hex(char *num) {
		int len = strlen(num), res = 0;
		for (int i = 2; i < len; ++ i) {
			char ch = *(num + sizeof(char) * i);
			int dig = 0;
			if (ch >= '0' && ch <= '9') {
				dig = ch - '0';
			} else if (ch >= 'a' && ch <= 'f') {
				dig = ch - 'a' + 10;
			} else if (ch >= 'A' && ch <= 'F') {
				dig = ch - 'A' + 10;
			}
			res = res * 16 + dig;
		}
		return res;
	}

	int atoi_oct(char *num) {
		int len = strlen(num), res = 0;
		for (int i = 1; i < len; ++ i) {
			int dig = *(num + sizeof(char) * i) - '0';
			res = res * 8 + dig;
		}
		return res;
	}
%}
DELIM [ \t\r]
WS {DELIM}+
LETTER [_a-zA-Z]
DIGIT [0-9]
DIGITS {DIGIT}+
DEC_INT ([1-9]{DIGIT}*)|0
OCT_INT 0[1-7][0-7]*
HEX_INT 0(x|X)[1-9A-Fa-f][0-9A-Fa-f]*
PRE_FLOAT ({DIGIT}*\.{DIGIT}+)|({DIGIT}+\.{DIGIT}*)
FLOAT ({DIGIT}*\.{DIGIT}*[Ee][+\-]?{DIGITS})|({PRE_FLOAT})
LT <
LE <=
EQ ==
NE !=
GT >
GE >=
NOT !
IF if
ELSE else
WHILE while
STRUCT struct
RETURN return
T_INT int
T_FLOAT float
ASSIGNOP =
SEMI ;
COMMA ,
DOT \.
PLUS \+
MINUS \-
STAR \*
DIV \/
AND &&
OR \|\|
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
ID {LETTER}({LETTER}|{DIGIT})*
%%
"\n" {
    yycolumn = 1;
}
{WS} {
	// do nothing ..
}
"\/\/" {
    for (char ch = 0; (ch = input()) != '\n' && ch != 0;);
}
"\/\*" {
	for (char ch; true; ) {
		for (ch = input(); (ch != '*') && (ch != 0); ) {
            if (ch == '\n') {
                yycolumn = 1;
            }
			ch = input();
		}
		if (ch == '*') {
			for (ch = input(); ch == '*'; ) {
				ch = input();
			}
			if (ch =='/') {
				break;
			} else if (ch == '\n') {
                yycolumn = 1;
            }
		}
		if (ch == 0) {
            extern bool lex_err;
            lex_err = true;
            printf("Error type A at Line %d: open comments\n", yylineno);
			break;
		}
	}
}
{T_INT} {
    yylval.type_node.ast = ast_alloc(CONS_SPEC, yylineno, TYPE_PRIM_INT);
    yylval.type_node.cst = cst_alloc("TYPE", yytext, yylineno, 0);
	return(TYPE);
}
{T_FLOAT} {
    yylval.type_node.ast = ast_alloc(CONS_SPEC, yylineno, TYPE_PRIM_FLT);
    yylval.type_node.cst = cst_alloc("TYPE", yytext, yylineno, 0);
	return(TYPE);
}
{IF} {
    yylval.type_cst.cst = cst_alloc("IF", yytext, yylineno, 0);
	return(IF);
}
{ELSE} {
    yylval.type_cst.cst = cst_alloc("ELSE", yytext, yylineno, 0);
	return(ELSE);
}
{WHILE} {
    yylval.type_cst.cst = cst_alloc("WHILE", yytext, yylineno, 0);
	return(WHILE);
}
{RETURN} {
    yylval.type_cst.cst = cst_alloc("RETURN", yytext, yylineno, 0);
	return(RETURN);
}
{STRUCT} {
    yylval.type_cst.cst = cst_alloc("STRUCT", yytext, yylineno, 0);
	return(STRUCT);
}
{ID} {
    yylval.type_str.str = zalloc(SYM_STR_SIZE);
    yylval.type_str.cst = cst_alloc("ID", yytext, yylineno, 0);
    symcpy(yylval.type_str.str, yytext);
	return(ID);
}
{DEC_INT} {
    static char __DEC_INT_buf[64];
    yylval.type_int.val = atoi(yytext);
    snprintf(__DEC_INT_buf, 63, "%d", yylval.type_int.val);
    yylval.type_int.cst = cst_alloc("INT", __DEC_INT_buf, yylineno, 0);
	return(INT);
}
{OCT_INT} {
    static char __OCT_INT_buf[MAX_SYM_LEN];
    yylval.type_int.val = atoi_oct(yytext);
    snprintf(__OCT_INT_buf, MAX_SYM_LEN - 1, "%d", yylval.type_int.val);
    yylval.type_int.cst = cst_alloc("INT", __OCT_INT_buf, yylineno, 0);
	return(INT);
}
{HEX_INT} {
    static char __HEX_INT_buf[MAX_SYM_LEN];
    yylval.type_int.val = atoi_hex(yytext);
    snprintf(__HEX_INT_buf, MAX_SYM_LEN - 1, "%d", yylval.type_int.val);
    yylval.type_int.cst = cst_alloc("INT", __HEX_INT_buf, yylineno, 0);
	return(INT);
}
{FLOAT} {
    static char __FLT_buf[MAX_SYM_LEN];
    yylval.type_float.val = atof(yytext);
    snprintf(__FLT_buf, MAX_SYM_LEN - 1, "%lf", yylval.type_float.val);
    yylval.type_float.cst = cst_alloc("FLOAT", __FLT_buf, yylineno, 0);
	return(FLOAT);
}
{SEMI} {
    yylval.type_cst.cst = cst_alloc("SEMI", yytext, yylineno, 0);
	return(SEMI);
}
{COMMA} {
    yylval.type_cst.cst = cst_alloc("COMMA", yytext, yylineno, 0);
	return(COMMA);
}
{DOT} {
    yylval.type_cst.cst = cst_alloc("DOT", yytext, yylineno, 0);
	return(DOT);
}
{LE} {
    yylval.type_cst.cst = cst_alloc("RELOP", "LE", yylineno, 0);
	return(LE);
}
{GE} {
    yylval.type_cst.cst = cst_alloc("RELOP", "GE", yylineno, 0);
	return(GE);
}
{GT} {
    yylval.type_cst.cst = cst_alloc("RELOP", "GT", yylineno, 0);
	return(GT);
}
{LT} {
    yylval.type_cst.cst = cst_alloc("RELOP", "LT", yylineno, 0);
	return(LT);
}
{EQ} {
    yylval.type_cst.cst = cst_alloc("RELOP", "EQ", yylineno, 0);
	return(EQ);
}
{NE} {
    yylval.type_cst.cst = cst_alloc("RELOP", "NE", yylineno, 0);
	return(NE);
}
{NOT} {
    yylval.type_cst.cst = cst_alloc("NOT", yytext, yylineno, 0);
	return(NOT);
}
{ASSIGNOP} {
    yylval.type_cst.cst = cst_alloc("ASSIGNOP", yytext, yylineno, 0);
	return(ASSIGNOP);
}
{PLUS} {
    yylval.type_cst.cst = cst_alloc("PLUS", yytext, yylineno, 0);
	return(PLUS);
}
{MINUS} {
    yylval.type_cst.cst = cst_alloc("MINUS", yytext, yylineno, 0);
	return(MINUS);
}
{STAR} {
    yylval.type_cst.cst = cst_alloc("STAR", yytext, yylineno, 0);
	return(STAR);
}
{DIV} {
    yylval.type_cst.cst = cst_alloc("DIV", yytext, yylineno, 0);
	return(DIV);
}
{AND} {
    yylval.type_cst.cst = cst_alloc("AND", yytext, yylineno, 0);
	return(AND);
}
{OR} {
    yylval.type_cst.cst = cst_alloc("OR", yytext, yylineno, 0);
	return(OR);
}
{LP} {
    yylval.type_cst.cst = cst_alloc("LP", yytext, yylineno, 0);
	return(LP);
}
{LB} {
    yylval.type_cst.cst = cst_alloc("LB", yytext, yylineno, 0);
	return(LB);
}
{RP} {
    yylval.type_cst.cst = cst_alloc("RP", yytext, yylineno, 0);
	return(RP);
}
{RB} {
    yylval.type_cst.cst = cst_alloc("RB", yytext, yylineno, 0);
	return(RB);
}
{LC} {
    yylval.type_cst.cst = cst_alloc("LC", yytext, yylineno, 0);
	return(LC);
}
{RC} {
    yylval.type_cst.cst = cst_alloc("RC", yytext, yylineno, 0);
	return(RC);
}
. {
    extern bool lex_err;
    lex_err = true;
    printf("Error type A at Line %d: character \"%s\"\n", yylineno, yytext);
}
%%
