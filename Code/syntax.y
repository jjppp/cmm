%{
#include "symtab.h"

extern int lineno;

i32 yylex(void);

void yyerror(const char *s) {
	extern bool syn_err;
	syn_err = true;
    printf("syn_err %d\n", lineno);
}

#define emit(...) ir_push(ir_alloc(__VA_ARGS__))

static ir_list instrs = {0};
static const ir_list empty_list = {0};

static void ir_push(IR_t *ir) {
    ir_append(&instrs, ir);
}

static void fun_push(char *str) {
    extern ir_fun_t *prog;
    ir_fun_t *fun = zalloc(sizeof(ir_fun_t));
    fun->instrs = instrs;
    fun->next = prog;
    symcpy(fun->str, str);
    prog = fun;
    instrs = empty_list;
}

static hashtab_t vartab;

static oprd_t var_lookup(char *str) {
    hashent_t *ent = hash_lookup(&vartab, str);
    if (ent->ptr == NULL) {
        symcpy(ent->str, str);
        oprd_t oprd = var_alloc(str, 0);
        ent->ptr = (void *) oprd.id;
        return oprd;
    }
    return (oprd_t) {
        .name = str,
        .kind = OPRD_VAR,
        .id = (uptr) ent->ptr,
        .lineno = 0,
        .offset = 0};
}
%}

%union {
    char  *type_str;
    i64    type_int;
    oprd_t type_oprd;
}

%token LABEL FUNCTION GOTO IF RETURN DEC ARG CALL PARAM READ WRITE
%token LT LE GT GE NE EQ NEWLINE
%token PLUS MINUS STAR DIV
%token AMP SHARP COLON ASSIGNOP
%token <type_int> INT
%token <type_str> ID

%nterm <type_oprd> Operand
%nterm <type_oprd> Var
%nterm Fun FunList
%nterm Instr

%%

FunList
    : FunList Fun
    | Fun;

Fun : FUNCTION ID COLON NEWLINE InstrList { fun_push($2); };

InstrList
    : InstrList NEWLINE Instr
    | InstrList NEWLINE
    | Instr;

Operand
    : Var { $$ = $1; }
    | SHARP INT {
        $$ = lit_alloc($2);
    }
    | AMP Var {
        $$ = var_alloc(NULL, 0);
        emit(IR_DREF, $$, $2);
    };

Var : ID { $$ = var_lookup($1); };

Instr
    : LABEL ID COLON { emit(IR_LABEL, $2); }
    | Var ASSIGNOP Operand { emit(IR_ASSIGN, $1, $3); }
    | Var ASSIGNOP Operand PLUS  Operand { emit(IR_BINARY, OP_ADD, $1, $3, $5); }
    | Var ASSIGNOP Operand MINUS Operand { emit(IR_BINARY, OP_SUB, $1, $3, $5); }
    | Var ASSIGNOP Operand STAR  Operand { emit(IR_BINARY, OP_MUL, $1, $3, $5); }
    | Var ASSIGNOP Operand DIV   Operand { emit(IR_BINARY, OP_DIV, $1, $3, $5); }
    | Var ASSIGNOP STAR Var { emit(IR_LOAD, $1, $4); } // TODO: check `x := *&y` out
    | STAR Var ASSIGNOP Operand { emit(IR_STORE, $2, $4); }
    | GOTO ID { emit(IR_GOTO, $2); }
    | IF Operand LT Operand GOTO ID { emit(IR_BRANCH, OP_LT, $2, $4, $6); }
    | IF Operand LE Operand GOTO ID { emit(IR_BRANCH, OP_LE, $2, $4, $6); }
    | IF Operand GT Operand GOTO ID { emit(IR_BRANCH, OP_GT, $2, $4, $6); }
    | IF Operand GE Operand GOTO ID { emit(IR_BRANCH, OP_GE, $2, $4, $6); }
    | IF Operand EQ Operand GOTO ID { emit(IR_BRANCH, OP_EQ, $2, $4, $6); }
    | IF Operand NE Operand GOTO ID { emit(IR_BRANCH, OP_NE, $2, $4, $6); }
    | RETURN Operand { emit(IR_RETURN, $2); }
    | DEC Var INT { emit(IR_DEC, $2, lit_alloc($3)); }
    | ARG Operand { emit(IR_ARG, $2); }
    | Var ASSIGNOP CALL ID { emit(IR_CALL, $1, $4); }
    | PARAM Operand { emit(IR_PARAM, $2); }
    | READ Operand { emit(IR_READ, $2); }
    | WRITE Operand { emit(IR_WRITE, $2); }
    ;

%%
#include "lex.yy.c"