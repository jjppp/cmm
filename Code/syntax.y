%{
#include "symtab.h"
#include "ast.h"

extern ast_t *root;
extern cst_t *croot;

int yylex(void);

void yyerror(char* s) {
	extern char* yytext;
	extern int yylineno;
	extern bool syn_err;
	syn_err = true;
	printf("Error type B At Line %d: syntax error, Content: %s\n", yylineno, yytext);
}

%}

%union {
	struct {
        ast_t *ast;
        cst_t *cst;
    } type_node;
    struct {
        cst_t *cst;
    } type_cst;
    struct {
        char  *str;
        cst_t *cst;
    } type_str;
    struct {
        double val;
        cst_t *cst;
    } type_float;
    struct {
        int    val;
        cst_t *cst;
    } type_int;
    struct {
        type_t typ;
        cst_t *cst;
    } type_type;
}

%token <type_cst>       STRUCT RETURN IF ELSE WHILE SEMI COMMA LC RC
%token <type_float>     FLOAT 
%token <type_int>       INT 
%token <type_str>       ID 
%token <type_type>      TYPE

%type  <type_type>      Specifier StructSpecifier
%type  <type_node>      FunDec Stmt CompSt Program Def Dec DecList Exp ParamDec ExtDef ExtDefList ExtDecList DefList VarList StmtList Args VarDec
%type  <type_str>       OptTag Tag

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

// 8Priority
%right <type_cst> ASSIGNOP
// 7Priority
%left  <type_cst> OR
// 6Priority
%left  <type_cst> AND
// 5Priority
%left  <type_cst> LE LT GE GT EQ NE
// 4Priority
%left  <type_cst> MINUS PLUS
// 3Priority
%left  <type_cst> STAR DIV
// 2Priority
%right <type_cst> NOT _MINUS
// 1Priority
%left  <type_cst> LP RP LB RB DOT

%destructor {
    if ($$.str != NULL) {
        zfree($$.str);
    } 
    if (root == NULL) {
        cst_free($$.cst);
    }
} <type_str>

%destructor {
    if (root == NULL) { // to avoid cleanup popups
        ast_free($$.ast);
        cst_free($$.cst);
    }
} <type_node>

%destructor {
    if (root == NULL) {
        cst_free($$.cst);
    }
} <type_cst>

%destructor {
    if (root == NULL) {
        cst_free($$.cst);
    }
} <type_int>

%destructor {
    if (root == NULL) {
        cst_free($$.cst);
    }
} <type_float>

%destructor {
    if (root == NULL) {
        cst_free($$.cst);
    }
} <type_type>
%%

/* A Program consists of a string of ExtDefs */
Program : ExtDefList {
    croot = cst_alloc("Program", "", @1.first_line, 1, $1.cst);
    root = ast_alloc(CONS_PROG, @1.first_line, $1.ast); 
};

ExtDefList 
    : ExtDef ExtDefList {
        $$.cst = cst_alloc("ExtDefList", "", @1.first_line, 2, $1.cst, $2.cst);
        // concat 2 lists
        if ($1.ast == NULL) {
            $$.ast = $2.ast;
        } else {
            $$.ast = $1.ast; 
            ast_foreach($$.ast, it) {
                if (it->next == NULL) {
                    it->next = $2.ast;
                    break;
                }
            }
        }
    }
	| %empty {
        $$.cst = NULL;
        $$.ast = NULL; 
    }
;

/* global variables, struct types, functions */
ExtDef 
    : Specifier ExtDecList SEMI { 
        $$.cst = cst_alloc("ExtDef", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        ast_foreach($2.ast, it) {
            INSTANCE_OF(it, DECL_VAR);
            cnode->typ = $1.typ;
        }
        $$.ast = $2.ast;
    }
	| Specifier SEMI { 
        $$.cst = cst_alloc("ExtDef", "", @1.first_line, 2, $1.cst, $2.cst);
        if ($1.typ.spec_typ == TYPE_STRUCT) {
            $$.ast = ast_alloc(DECL_TYP, @1.first_line, $1.typ);
        } else {
            $$.ast = NULL;
        }
    }
	| Specifier FunDec CompSt { 
        $$.cst = cst_alloc("ExtDef", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $2.ast;
        INSTANCE_OF($$.ast, DECL_FUN);
        cnode->typ = $1.typ;
        cnode->body = $3.ast; 
    }
	| error SEMI { TODO; $$.ast = NULL; }
;

/* variable declarations */
ExtDecList 
    : VarDec {
        $$.cst = cst_alloc("ExtDecList", "", @1.first_line, 1, $1.cst);
        $$.ast = $1.ast; 
    }
	| VarDec COMMA ExtDecList { 
        $$.cst = cst_alloc("ExtDecList", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $1.ast; $$.ast->next = $3.ast; 
    }
;

/* different types */
Specifier 
    : TYPE { 
        $$.cst = cst_alloc("Specifier", "", @1.first_line, 1, $1.cst);
        $$.typ = $1.typ; 
    }
	| StructSpecifier { 
        $$.cst = cst_alloc("Specifier", "", @1.first_line, 1, $1.cst);
        $$.typ = $1.typ; 
    }
;

StructSpecifier 
    : STRUCT OptTag LC DefList RC { 
        $$.cst = cst_alloc("StructSpecifier", "", @1.first_line, 5, $1.cst, $2.cst, $3.cst, $4.cst, $5.cst);
        $$.typ = (type_t) {
            .spec_typ = TYPE_STRUCT,
            .decls = $4.ast
        };
        if ($2.str != NULL) {
            symmov($$.typ.str, $2.str);
        }
    }
	| STRUCT error ID LC DefList RC { TODO; yyerrok; }
	| STRUCT Tag {
        $$.cst = cst_alloc("StructSpecifier", "", @1.first_line, 2, $1.cst, $2.cst);
        $$.typ = (type_t) {
            .spec_typ = TYPE_STRUCT,
            .decls = NULL,
        };
        symmov($$.typ.str, $2.str);
    }
;

OptTag
	: Tag { 
        $$.cst = cst_alloc("OptTag", "", @1.first_line, 1, $1.cst);
        $$.str = $1.str; 
    }
	| %empty {
        $$.cst = NULL;
        $$.str = NULL; 
    }
;

Tag : ID {
    $$.cst = cst_alloc("Tag", "", @1.first_line, 1, $1.cst);
    $$.str = $1.str; 
};

VarDec 
    : ID {
        $$.cst = cst_alloc("VarDec", "", @1.first_line, 1, $1.cst);
        $$.ast = ast_alloc(DECL_VAR, @1.first_line, $1.str, 0);
    }
	| VarDec LB INT RB { 
        $$.cst = cst_alloc("VarDec", "", @1.first_line, 4, $1.cst, $2.cst, $3.cst, $4.cst);
        $$.ast = $1.ast; 
        INSTANCE_OF($$.ast, DECL_VAR);
        cnode->dim = $3.val;
    }
	| VarDec LB error RB {
        $$.cst = cst_alloc("VarDec", "", @1.first_line, 3, $1.cst, $2.cst, $4.cst);
        $$.ast = $1.ast;
        yyerrok;
    }
;

FunDec 
    : ID LP VarList RP { 
        $$.cst = cst_alloc("FunDec", "", @1.first_line, 4, $1.cst, $2.cst, $3.cst, $4.cst);
        $$.ast = ast_alloc(DECL_FUN, @1.first_line, $1.str, $3.ast); 
    }
	| ID LP RP { 
        $$.cst = cst_alloc("FunDec", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(DECL_FUN, @1.first_line, $1.str, NULL); 
    }
	| ID LP error RP { TODO; $$.ast = NULL; }
;

VarList 
    : ParamDec COMMA VarList { 
        $$.cst = cst_alloc("VarList", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $1.ast; $$.ast->next = $3.ast; 
    }
	| ParamDec { 
        $$.cst = cst_alloc("VarList", "", @1.first_line, 1, $1.cst);
        $$.ast = $1.ast; 
    }
	| ParamDec COMMA error { TODO; }
;

ParamDec 
    : Specifier VarDec { 
        $$.cst = cst_alloc("ParamDec", "", @1.first_line, 2, $1.cst, $2.cst);
        ast_foreach($2.ast, it) {
            INSTANCE_OF(it, DECL_VAR);
            cnode->typ = $1.typ;
        }
        $$.ast = $2.ast;
    }
;

CompSt 
    : LC DefList StmtList RC { 
        $$.cst = cst_alloc("CompSt", "", @1.first_line, 4, $1.cst, $2.cst, $3.cst, $4.cst);
        $$.ast = ast_alloc(STMT_SCOP, @1.first_line, $2.ast, $3.ast); 
    } ;

StmtList 
    : Stmt StmtList {
        $$.cst = cst_alloc("StmtList", "", @1.first_line, 2, $1.cst, $2.cst);
        if ($1.ast != NULL) {
            $$.ast = $1.ast;
            $$.ast->next = $2.ast;
        } else {
            $$.ast = $2.ast;
        }
    }
	| %empty {
        $$.cst = NULL;
        $$.ast = NULL; 
    }
	| error StmtList {
        $$.cst = cst_alloc("StmtList", "", @1.first_line, 1, $2.cst);
        $$.ast = $2.ast;
    }
;

Stmt: Exp SEMI { 
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 2, $1.cst, $2.cst);
        $$.ast = ast_alloc(STMT_EXPR, @1.first_line, $1.ast); 
    }
	| error SEMI {
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 1, $2.cst);
        $$.ast = NULL; 
    }
	| CompSt {
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 1, $1.cst);
        $$.ast = $1.ast; 
    }
	| RETURN Exp SEMI {
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(STMT_RET, @1.first_line, $2.ast);  
    }
	| RETURN error SEMI {
        TODO; 
    }
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 5, $1.cst, $2.cst, $3.cst, $4.cst, $5.cst);
        $$.ast = ast_alloc(STMT_IFTE, @1.first_line, $3.ast, $5.ast, NULL);  
    }
	| IF LP error RP Stmt %prec LOWER_THAN_ELSE {
        TODO; 
    }
	| IF LP Exp RP Stmt ELSE Stmt {
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 7, $1.cst, $2.cst, $3.cst, $4.cst, $5.cst, $6.cst, $7.cst);
        $$.ast = ast_alloc(STMT_IFTE, @1.first_line, $3.ast, $5.ast, $7.ast); 
    }
	| IF LP error RP Stmt ELSE Stmt {
        TODO; 
    }
	| WHILE LP Exp RP Stmt {
        $$.cst = cst_alloc("Stmt", "", @1.first_line, 5, $1.cst, $2.cst, $3.cst, $4.cst, $5.cst);
        $$.ast = ast_alloc(STMT_WHLE, @1.first_line, $3.ast, $5.ast); 
    }
	| WHILE LP error RP Stmt {
        TODO; 
    }
;

DefList 
    // concat 2 lists
    : Def DefList {
        $$.cst = cst_alloc("DefList", "", @1.first_line, 2, $1.cst, $2.cst);
        if ($1.ast == NULL) {
            $$.ast = $2.ast;
        } else {
            $$.ast = $1.ast;
            ast_foreach($1.ast, it) {
                if (it->next == NULL) {
                    it->next = $2.ast;
                    break;
                }
            }
        }
    }
	| %empty {
        $$.cst = NULL; 
        $$.ast = NULL; 
    }
;

Def : Specifier DecList SEMI {
        $$.cst = cst_alloc("Def", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $2.ast;
        ast_foreach($$.ast, it) {
            INSTANCE_OF(it, DECL_VAR);
            cnode->typ = $1.typ;
        }
    }
	| Specifier error SEMI {
        $$.cst = cst_alloc("Def", "", @1.first_line, 2, $1.cst, $3.cst);
        $$.ast = NULL;
        yyerrok;
    }
;

DecList
	: Dec { 
        $$.cst = cst_alloc("DecList", "", @1.first_line, 1, $1.cst);
        $$.ast = $1.ast; 
    }
	| Dec COMMA DecList { 
        $$.cst = cst_alloc("DecList", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $1.ast; $$.ast->next = $3.ast; 
    }
	| error COMMA Dec { TODO; }
;

Dec : VarDec { 
        $$.cst = cst_alloc("Dec", "", @1.first_line, 1, $1.cst);
        $$.ast = $1.ast; 
    }
	| VarDec ASSIGNOP Exp { 
        $$.cst = cst_alloc("Dec", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $1.ast;
        INSTANCE_OF($$.ast, DECL_VAR); 
        cnode->expr = $3.ast;
    }
;

Exp : Exp ASSIGNOP Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_ASS, @1.first_line, $1.ast, $3.ast); 
    }
	| Exp AND Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_AND, $3.ast); 
    }
	| Exp OR Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_OR, $3.ast); 
    }
	| Exp LE Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_LE, $3.ast); 
    }
	| Exp LT Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_LT, $3.ast); 
    }
	| Exp GE Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_GE, $3.ast); 
    }
	| Exp GT Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_GT, $3.ast); 
    }
	| Exp EQ Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_EQ, $3.ast); 
    }
	| Exp NE Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_NE, $3.ast); 
    }
	| Exp PLUS Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_ADD, $3.ast); 
    }
	| Exp MINUS Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_SUB, $3.ast); 
    }
	| Exp STAR Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_MUL, $3.ast); 
    }
	| Exp DIV Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_BIN, @1.first_line, $1.ast, OP_DIV, $3.ast); 
    }
	| LP Exp RP { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $2.ast; 
    }
	| MINUS Exp %prec _MINUS { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 2, $1.cst, $2.cst);
        $$.ast = ast_alloc(EXPR_UNR, @1.first_line, OP_NEG, $2.ast); 
    }
	| NOT Exp { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 2, $1.cst, $2.cst);
        $$.ast = ast_alloc(EXPR_UNR, @1.first_line, OP_NOT, $2.ast); 
    }
	| ID LP Args RP { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 4, $1.cst, $2.cst, $3.cst, $4.cst);
        $$.ast = ast_alloc(EXPR_CALL,@1.first_line, $1.str, $3.ast); 
    }
	| ID LP RP { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_CALL,@1.first_line, $1.str, NULL); 
    }
	| Exp LB Exp RB { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 4, $1.cst, $2.cst, $3.cst, $4.cst);
        $$.ast = ast_alloc(EXPR_ARR,@1.first_line, $1.ast, $3.ast); 
    }
	| Exp LB error RB { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $4.cst);
        $$.ast = ast_alloc(EXPR_ARR,@1.first_line, $1.ast, NULL);
        yyerrok; 
    }
	| Exp DOT ID { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = ast_alloc(EXPR_DOT, @1.first_line, $1.ast, $3.str); 
    }
	| ID { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 1, $1.cst);
        $$.ast = ast_alloc(EXPR_IDEN,@1.first_line, $1.str); 
    }
	| INT { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 1, $1.cst);
        $$.ast = ast_alloc(EXPR_INT, @1.first_line, $1.val); 
    }
	| FLOAT { 
        $$.cst = cst_alloc("Exp", "", @1.first_line, 1, $1.cst);
        $$.ast = ast_alloc(EXPR_FLT, @1.first_line, $1.val); 
    }
;

Args: Exp COMMA Args {
        $$.cst = cst_alloc("Args", "", @1.first_line, 3, $1.cst, $2.cst, $3.cst);
        $$.ast = $1.ast; $$.ast->next = $3.ast; 
    }
	| Exp { 
        $$.cst = cst_alloc("Args", "", @1.first_line, 1, $1.cst);
        $$.ast = $1.ast; 
    }
;

%%
#include "lex.yy.c"