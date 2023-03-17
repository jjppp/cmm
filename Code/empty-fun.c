#include "ast.h"
#include "visitor.h"
#include "common.h"
#include "symtab.h"
#include "semerr.h"

#define RET_TYPE va_list
#define ARG __none
VISITOR_DEF(AST, empty_fun, RET_TYPE);

extern bool sem_err;

void empty_fun_check(AST_t *node) {
    VISITOR_DISPATCH(AST, empty_fun, node, NULL);
}

VISIT(CONS_PROG) {
    LIST_FOREACH(node->decls, empty_fun_check);
}

VISIT(CONS_FUN) {
    syment_t *sym = sym_lookup(node->str);
    if (!sym || sym->body == NULL) {
        SEM_ERR(ERR_FUN_DEC_UNDEF, node->super.fst_l, node->str);
    }
}

VISIT_EMPTY(STMT_EXPR);
VISIT_EMPTY(STMT_SCOP);
VISIT_EMPTY(STMT_IFTE);
VISIT_EMPTY(STMT_WHLE);
VISIT_EMPTY(STMT_RET);
VISIT_EMPTY(EXPR_CALL);
VISIT_EMPTY(EXPR_IDEN);
VISIT_EMPTY(EXPR_ARR);
VISIT_EMPTY(EXPR_ASS);
VISIT_EMPTY(EXPR_DOT);
VISIT_EMPTY(EXPR_INT);
VISIT_EMPTY(EXPR_FLT);
VISIT_EMPTY(EXPR_BIN);
VISIT_EMPTY(EXPR_UNR);
VISIT_EMPTY(DECL_VAR);
VISIT_EMPTY(DECL_TYP);
VISIT_EMPTY(CONS_SPEC);
