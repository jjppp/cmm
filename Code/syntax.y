%{
#include "symtab.h"
#include "ast.h"

extern ast_t *root;

int yylex(void);

void yyerror(char* s) {
	extern char* yytext;
	extern int yylineno;
	extern bool syntax_error_detected;
	syntax_error_detected = true;
	printf("Error type B At Line %d: syntax error, Content: %s\n", yylineno, yytext);
}

%}

%union {
	ast_t      *type_node;
    char       *type_str;
    double      type_float;
    int         type_int;
    type_t      type_type;
}

%token <type_node>      STRUCT RETURN IF WHILE SEMI COMMA LC RC
%token <type_float>     FLOAT 
%token <type_int>       INT 
%token <type_str>       ID 
%token <type_type>      TYPE

%type  <type_type>      Specifier StructSpecifier
%type  <type_node>      FunDec Stmt CompSt Program Def Dec DecList Exp ParamDec ExtDef ExtDefList ExtDecList DefList VarList StmtList Args VarDec
%type  <type_str>       OptTag Tag

%nonassoc LOWER_THAN_ELSE
%nonassoc <type_node> ELSE

// 8Priority
%right <type_node> ASSIGNOP
// 7Priority
%left <type_node> OR
// 6Priority
%left <type_node> AND
// 5Priority
%left <type_node> LE LT GE GT EQ NE
// 4Priority
%left <type_node> MINUS PLUS
// 3Priority
%left <type_node> STAR DIV
// 2Priority
%right <type_node> NOT _MINUS
// 1Priority
%left <type_node> LP RP LB RB DOT
%%

/* A Program consists of a string of ExtDefs */
Program : ExtDefList { root = new_ast_node(CONS_PROG, @1.first_line, $1); } ;

ExtDefList 
    : ExtDef ExtDefList { $$ = $1; $$->next = $2; }
	| %empty { $$ = NULL; }
;

/* global variables, struct types, functions */
ExtDef 
    : Specifier ExtDecList SEMI { 
        ast_foreach($2, it) {
            INSTANCE_OF(it, DECL_VAR);
            cnode->type = $1;
        }
        $$ = $2;
    }
	| Specifier SEMI { TODO; }
	| Specifier FunDec CompSt { 
        $$ = $2;
        INSTANCE_OF($$, DECL_FUN);
        cnode->type = $1;
        cnode->body = $3; 
    }
	| error SEMI { TODO; $$ = NULL; }
;

/* variable declarations */
ExtDecList 
    : VarDec { $$ = $1; }
	| VarDec COMMA ExtDecList { $$ = $1; $$->next = $3; }
;

/* different types */
Specifier 
    : TYPE { $$ = $1; }
	| StructSpecifier { $$ = $1; }
;

StructSpecifier 
    : STRUCT OptTag LC DefList RC { 
        $$ = (type_t) {
            .spec_type = TYPE_STRUCT,
            .decls = $4
        };
        if ($2 != NULL) {
            symcpy($$.str, $2);
            free($2);
        }
    }
	| STRUCT error ID LC DefList RC { TODO; yyerrok; }
	| STRUCT Tag { TODO; }
;

OptTag
	: Tag { $$ = $1; }
	| %empty { $$ = NULL; }
;

Tag : ID { $$ = $1; } ;

VarDec 
    : ID {
        $$ = new_ast_node(DECL_VAR, @1.first_line, $1, 0);
    }
	| VarDec LB INT RB { 
        $$ = $1; 
        INSTANCE_OF($$, DECL_VAR);
        cnode->dim = $3;
    }
	| VarDec LB error RB { TODO; $$ = NULL; }
;

FunDec 
    : ID LP VarList RP { $$ = new_ast_node(DECL_FUN, @1.first_line, $1, $3); }
	| ID LP RP { $$ = new_ast_node(DECL_FUN, @1.first_line, $1, NULL); }
	| ID LP error RP { TODO; $$ = NULL; }
;

VarList 
    : ParamDec COMMA VarList { $$ = $1; $$->next = $3; }
	| ParamDec { $$ = $1; }
	| ParamDec COMMA error { TODO; }
;

ParamDec 
    : Specifier VarDec { 
        ast_foreach($2, it) {
            INSTANCE_OF(it, DECL_VAR);
            cnode->type = $1;
        }
        $$ = $2;
    }
;

CompSt 
    : LC DefList StmtList RC { $$ = new_ast_node(STMT_SCOP, @1.first_line, $2, $3); } ;

StmtList 
    : Stmt StmtList { $$ = $1; $$->next = $2; }
	| %empty { $$ = NULL; }
	| error StmtList { TODO; }
;

Stmt: Exp SEMI { $$ = new_ast_node(STMT_EXPR, @1.first_line, $1); }
	| error SEMI { TODO; }
	| CompSt { $$ = $1; }
	| RETURN Exp SEMI { $$ = new_ast_node(STMT_RET, @1.first_line, $2);  }
	| RETURN error SEMI { TODO; }
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = new_ast_node(STMT_IFTE, @1.first_line, $3, $5, NULL);  }
	| IF LP error RP Stmt %prec LOWER_THAN_ELSE { TODO; }
	| IF LP Exp RP Stmt ELSE Stmt { $$ = new_ast_node(STMT_IFTE, @1.first_line, $3, $5, $7); }
	| IF LP error RP Stmt ELSE Stmt { TODO; }
	| WHILE LP Exp RP Stmt { $$ = new_ast_node(STMT_WHLE, @1.first_line, $3, $5); }
	| WHILE LP error RP Stmt { TODO; }
;

DefList 
    : Def DefList { $$ = $1; $$->next = $2; }
	| %empty { $$ = NULL; }
;

Def : Specifier DecList SEMI {
        $$ = $2;
        ast_foreach($$, it) {
            INSTANCE_OF(it, DECL_VAR);
            cnode->type = $1;
        }
    }
	| Specifier error SEMI { TODO; yyerrok; }
;

DecList
	: Dec { $$ = $1; }
	| Dec COMMA DecList { $$ = $1; $$->next = $3; }
	| error COMMA Dec { TODO; }
;

Dec : VarDec { $$ = $1; }
	| VarDec ASSIGNOP Exp { 
        $$ = $1;
        INSTANCE_OF($$, DECL_VAR); 
        cnode->expr = $3;
    }
;

Exp : Exp ASSIGNOP Exp { $$ = new_ast_node(EXPR_ASS, @1.first_line, $1, $3); }
	| Exp AND Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_AND, $3); }
	| Exp OR Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_OR, $3); }
	| Exp LE Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_LE, $3); }
	| Exp LT Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_LT, $3); }
	| Exp GE Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_GE, $3); }
	| Exp GT Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_GT, $3); }
	| Exp EQ Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_EQ, $3); }
	| Exp NE Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_NE, $3); }
	| Exp PLUS Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_ADD, $3); }
	| Exp MINUS Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_SUB, $3); }
	| Exp STAR Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_MUL, $3); }
	| Exp DIV Exp { $$ = new_ast_node(EXPR_BIN, @1.first_line, $1, OP_DIV, $3); }
	| LP Exp RP { $$ = $2; }
	| MINUS Exp %prec _MINUS { $$ = new_ast_node(EXPR_UNR, @1.first_line, OP_NEG, $2); }
	| NOT Exp { $$ = new_ast_node(EXPR_UNR, @1.first_line, OP_NOT, $2); }
	| ID LP Args RP { $$ = new_ast_node(EXPR_CALL,@1.first_line, $1, $3); }
	| ID LP RP { $$ = new_ast_node(EXPR_CALL,@1.first_line, $1); }
	| Exp LB Exp RB { $$ = new_ast_node(EXPR_CALL,@1.first_line, $1); }
	| Exp LB error RB { TODO; }
	| Exp DOT ID { $$ = new_ast_node(EXPR_DOT, @1.first_line, $1, $3); }
	| ID { $$ = new_ast_node(EXPR_IDEN,@1.first_line, $1); }
	| INT { $$ = new_ast_node(EXPR_INT, @1.first_line, $1); }
	| FLOAT { $$ = new_ast_node(EXPR_FLT, @1.first_line, $1); }
;

Args: Exp COMMA Args { $$ = $1; $$->next = $3; }
	| Exp { $$ = $1; }
;

%%
#include "lex.yy.c"