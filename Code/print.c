#include "visitor.h"
#include "common.h"
#include "type.h"
#include <stdarg.h>

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(AST, print, va_list);

static FILE *fout;
static u32   tabs = 0;

static void display(const char *fmt, ...) {
    for (u32 i = 1; i < tabs; i++) {
        fprintf(fout, "  ");
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(fout, fmt, ap);
    va_end(ap);
}

static void print_(AST_t *node) {
    ASSERT(node != NULL, "print NULL");
    tabs++;
    display("%s\n", AST_NAMES[node->kind]);
    VISITOR_DISPATCH(AST, print, node, NULL);
    tabs--;
}

void print(FILE *file, AST_t *node, ...) {
    fout = file;
    print_(node);
}

VISIT(EXPR_INT) {
    display("value: %d\n", node->value);
}

VISIT(EXPR_FLT) {
    display("value: %lf\n", node->value);
}

VISIT(EXPR_BIN) {
    extern const char *OP_NAMES[];
    print_(node->lhs);
    display("%s\n", OP_NAMES[node->op]);
    print_(node->rhs);
}

VISIT(EXPR_UNR) {
    extern const char *OP_NAMES[];
    display("%d\n", OP_NAMES[node->op]);
    print_(node->sub);
}

VISIT(EXPR_IDEN) {
    display("%s\n", node->str);
}

VISIT(STMT_EXPR) {
    print_(node->expr);
}

VISIT(STMT_RET) {
    print_(node->expr);
}

VISIT(STMT_WHLE) {
    print_(node->cond);
    print_(node->body);
}

VISIT(STMT_IFTE) {
    print_(node->cond);
    print_(node->tru_stmt);
    print_(node->fls_stmt);
}

VISIT(EXPR_DOT) {
    print_(node->base);
    display("%s\n", node->str);
}

VISIT(EXPR_ASS) {
    print_(node->lhs);
    print_(node->rhs);
}

VISIT(CONS_PROG) {
    ast_foreach(node->decls, print_);
}

VISIT(CONS_FUN) {
    ast_foreach(node->body, print_);
}

VISIT(DECL_VAR) {
    print_(node->spec);
    display("%s\n", node->str);
    if (node->expr) {
        print_(node->expr);
    }
}

VISIT(EXPR_ARR) {
    print_(node->arr);
    print_(node->ind);
}

VISIT(STMT_SCOP) {
    ast_foreach(node->decls, print_);
    ast_foreach(node->stmts, print_);
}

VISIT(EXPR_CALL) {
    display("CALL %s\n", node->str);
    print_(node->expr);
}

VISIT(DECL_TYP) {
    print_(node->spec);
}

VISIT(CONS_SPEC) {
    switch (node->kind) {
        case TYPE_PRIM_INT: display("INT\n"); break;
        case TYPE_PRIM_FLT: display("FLT\n"); break;
        case TYPE_STRUCT: display("STRUCT\n"); break;
        default: UNREACHABLE;
    }
    display("%s\n", node->str);
    if (node->kind == TYPE_STRUCT) {
        ast_foreach(node->fields, print_);
    }
}

VISIT(DECL_FUN) {
}