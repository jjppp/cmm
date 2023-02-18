#include "ast.h"
#include "common.h"
#include "type.h"
#include <stdarg.h>

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(print, va_list);

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

static void print_(ast_t *node) {
    ASSERT(node != NULL, "print NULL");
    tabs++;
    display("%s\n", AST_NODE_NAMES[node->ast_kind]);
    visitor_dispatch(visitor_print, node, NULL);
    tabs--;
}

void print(FILE *file, ast_t *node, ...) {
    fout = file;
    print_(node);
}

VISIT(EXPR_INT) {
    INSTANCE_OF(node, EXPR_INT);
    display("value: %d\n", cnode->value);
}

VISIT(EXPR_FLT) {
    INSTANCE_OF(node, EXPR_FLT);
    display("value: %lf\n", cnode->value);
}

VISIT(EXPR_BIN) {
    extern const char *OP_NAMES[];
    INSTANCE_OF(node, EXPR_BIN);
    print_(cnode->lhs);
    display("%s\n", OP_NAMES[cnode->op]);
    print_(cnode->rhs);
}

VISIT(EXPR_UNR) {
    extern const char *OP_NAMES[];
    INSTANCE_OF(node, EXPR_UNR);
    display("%d\n", OP_NAMES[cnode->op]);
    print_(cnode->sub);
}

VISIT(EXPR_IDEN) {
    INSTANCE_OF(node, EXPR_IDEN);
    display("%s\n", cnode->str);
}

VISIT(STMT_EXPR) {
    INSTANCE_OF(node, STMT_EXPR);
    print_(cnode->expr);
}

VISIT(STMT_RET) {
    INSTANCE_OF(node, STMT_RET);
    print_(cnode->expr);
}

VISIT(STMT_WHLE) {
    INSTANCE_OF(node, STMT_WHLE);
    print_(cnode->cond);
    print_(cnode->body);
}

VISIT(STMT_IFTE) {
    INSTANCE_OF(node, STMT_IFTE);
    print_(cnode->cond);
    print_(cnode->tru_stmt);
    print_(cnode->fls_stmt);
}

VISIT(EXPR_DOT) {
    INSTANCE_OF(node, EXPR_DOT);
    print_(cnode->base);
    display("%s\n", cnode->str);
}

VISIT(EXPR_ASS) {
    INSTANCE_OF(node, EXPR_ASS);
    print_(cnode->lhs);
    print_(cnode->rhs);
}

VISIT(CONS_PROG) {
    INSTANCE_OF(node, CONS_PROG);
    ast_foreach(cnode->decls, print_);
}

VISIT(DECL_FUN) {
    INSTANCE_OF(node, DECL_FUN);
    ast_foreach(cnode->body, print_);
}

VISIT(DECL_VAR) {
    INSTANCE_OF(node, DECL_VAR);
    print_(cnode->spec);
    display("%s\n", cnode->str);
    if (cnode->expr) {
        print_(cnode->expr);
    }
}

VISIT(EXPR_ARR) {
    INSTANCE_OF(node, EXPR_ARR);
    print_(cnode->arr);
    print_(cnode->ind);
}

VISIT(STMT_SCOP) {
    INSTANCE_OF(node, STMT_SCOP);
    ast_foreach(cnode->decls, print_);
    ast_foreach(cnode->stmts, print_);
}

VISIT(EXPR_CALL) {
    INSTANCE_OF(node, EXPR_CALL);
    display("CALL %s\n", cnode->str);
    print_(cnode->expr);
}

VISIT(DECL_TYP) {
    INSTANCE_OF(node, DECL_TYP);
    print_(cnode->spec);
}

VISIT(CONS_SPEC) {
    INSTANCE_OF(node, CONS_SPEC);
    switch (cnode->kind) {
        case TYPE_PRIM_INT: display("INT\n"); break;
        case TYPE_PRIM_FLT: display("FLT\n"); break;
        case TYPE_STRUCT: display("STRUCT\n"); break;
        case TYPE_ARRAY: TODO("TYPE_ARRAY");
        default: UNREACHABLE;
    }
    display("%s\n", cnode->str);
    if (cnode->kind == TYPE_STRUCT) {
        ast_foreach(cnode->fields, print_);
    }
}