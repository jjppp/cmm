%option yylineno
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
DELIM [ \t]|((\r)?\n)
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
{WS} {
	// do nothing ..
}
"\/\/" {
    for (char ch = 0; (ch = input()) != '\n' && ch != 0;);
}
"\/\*" {
	for (char ch; true; ) {
		for (ch = input(); (ch != '*') && (ch != 0); ) {
			ch = input();
		}
		if (ch == '*') {
			for (ch = input(); ch == '*'; ) {
				ch = input();
			}
			if (ch =='/') {
				break;
			}
		}
		if (ch == 0) {
			yyerror("0");
			break;
		}
	}
}
{T_INT} {
    yylval.type_type = (type_t) {
        .spec_type = TYPE_PRIM_INT,
        .decls = NULL
    };
	return(TYPE);
}
{T_FLOAT} {
    yylval.type_type = (type_t) {
        .spec_type = TYPE_PRIM_FLT,
        .decls = NULL
    };
	return(TYPE);
}
{IF} {
	return(IF);
}
{ELSE} {
	return(ELSE);
}
{WHILE} {
	return(WHILE);
}
{RETURN} {
	return(RETURN);
}
{STRUCT} {
	return(STRUCT);
}
{ID} {
    yylval.type_str = zalloc(SYM_STR_SIZE);
    symcpy(yylval.type_str, yytext);
	return(ID);
}
{DEC_INT} {
    yylval.type_int = atoi(yytext);
	return(INT);
}
{OCT_INT} {
	return(INT);
}
{HEX_INT} {
	return(INT);
}
{FLOAT} {
	return(FLOAT);
}
{SEMI} {
	return(SEMI);
}
{COMMA} {
	return(COMMA);
}
{DOT} {
	return(DOT);
}
{LE} {
	return(LE);
}
{GE} {
	return(GE);
}
{GT} {
	return(GT);
}
{LT} {
	return(LT);
}
{EQ} {
	return(EQ);
}
{NE} {
	return(NE);
}
{NOT} {
	return(NOT);
}
{ASSIGNOP} {
	return(ASSIGNOP);
}
{PLUS} {
	return(PLUS);
}
{MINUS} {
	return(MINUS);
}
{STAR} {
	return(STAR);
}
{DIV} {
	return(DIV);
}
{AND} {
	return(AND);
}
{OR} {
	return(OR);
}
{LP} {
	return(LP);
}
{LB} {
	return(LB);
}
{RP} {
	return(RP);
}
{RB} {
	return(RB);
}
{LC} {
	return(LC);
}
{RC} {
	return(RC);
}
. { printf("Error type A at Line %d: character \"%s\"\n", yylineno, yytext); }
%%
