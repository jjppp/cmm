%option yylineno
%option never-interactive
%option noinput
%option nounput
%{
	#include "syntax.tab.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>

	int yywrap(void) { return 1; }

    int lineno = 0;
%}
DELIM [ \t\r]
WS {DELIM}+
LETTER [_a-zA-Z]
DIGIT [0-9]
INT ([1-9]{DIGIT}*)|0
LT <
LE <=
EQ ==
NE !=
GT >
GE >=
LABEL LABEL
FUNCTION FUNCTION
GOTO GOTO
IF IF
RETURN RETURN
DEC DEC
ARG ARG
PARAM PARAM
CALL CALL
READ READ
WRITE WRITE
ASSIGNOP :=
COLON :
PLUS \+
MINUS \-
STAR \*
DIV \/
AMP &
SHARP #
ID {LETTER}({LETTER}|{DIGIT})*
%%
"\n"        { lineno ++; return(NEWLINE); }
{WS}        { }
{LABEL}     { return(LABEL); }
{FUNCTION}  { return(FUNCTION); }
{GOTO}      { return(GOTO); }
{IF}        { return(IF); }
{RETURN}    { return(RETURN); }
{DEC}       { return(DEC); }
{ARG}       { return(ARG); }
{PARAM}     { return(PARAM); }
{CALL}      { return(CALL); }
{READ}      { return(READ); }
{WRITE}     { return(WRITE); }
{COLON}     { return(COLON); }
{ID}        {
    yylval.type_str = zalloc(SYM_STR_SIZE);
    symcpy(yylval.type_str, yytext);
	return(ID);
}
{INT}       {
    yylval.type_int = atoll(yytext);
	return(INT);
}
{LE}        { return(LE); }
{GE}        { return(GE); }
{GT}        { return(GT); }
{LT}        { return(LT); }
{EQ}        { return(EQ); }
{NE}        { return(NE); }
{ASSIGNOP}  { return(ASSIGNOP); }
{PLUS}      { return(PLUS); }
{MINUS}     { return(MINUS); }
{STAR}      { return(STAR); }
{DIV}       { return(DIV); }
{AMP}       { return(AMP); }
{SHARP}     { return(SHARP); }
.           {
    extern bool lex_err;
    lex_err = true;
    printf("Error type A at Line %d: character \"%s\"\n", yylineno, yytext);
}
%%
