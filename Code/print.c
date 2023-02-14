#include "ast.h"
#include "common.h"
#include <stdarg.h>

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
    void visitor_dispatch(const struct ast_visitor visitor, ast_t *node, void *p);

    tabs++;
    display("%s\n", AST_NODE_NAMES[node->ast_kind]);
    visitor_dispatch(visitor_print, node, NULL);
    tabs--;
}

void print(FILE *file, ast_t *node, ...) {
    fout = file;
    print_(node);
}

static void visit_EXPR_INT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_INT);
    display("value: %d\n", cnode->value);
}

static void visit_EXPR_FLT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_FLT);
    display("value: %lf\n", cnode->value);
}

static void visit_EXPR_BIN(ast_t *node, va_list ap) {
    extern const char *OP_NAMES[];
    INSTANCE_OF(node, EXPR_BIN);
    print_(cnode->lhs);
    display("%s\n", OP_NAMES[cnode->op]);
    print_(cnode->rhs);
}

static void visit_EXPR_UNR(ast_t *node, va_list ap) {
    extern const char *OP_NAMES[];
    INSTANCE_OF(node, EXPR_UNR);
    display("%d\n", OP_NAMES[cnode->op]);
    print_(cnode->sub);
}

static void visit_EXPR_IDEN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_IDEN);
    display("%s\n", cnode->str);
}

static void visit_STMT_EXPR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_EXPR);
    print_(cnode->expr);
}

static void visit_STMT_RET(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_RET);
    print_(cnode->expr);
}

static void visit_STMT_WHLE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_WHLE);
    print_(cnode->cond);
    print_(cnode->body);
}

static void visit_STMT_IFTE(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_IFTE);
    print_(cnode->cond);
    print_(cnode->tru_stmt);
    print_(cnode->fls_stmt);
}

static void visit_EXPR_DOT(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_DOT);
    print_(cnode->base);
    print_(cnode->field);
}

static void visit_EXPR_ASS(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ASS);
    print_(cnode->lhs);
    print_(cnode->rhs);
}

static void visit_CONS_PROG(ast_t *node, va_list ap) {
    INSTANCE_OF(node, CONS_PROG);
    ast_foreach(cnode->decls, it) {
        print_(it);
    }
}

static void visit_DECL_FUN(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_FUN);
    ast_foreach(cnode->body, it) {
        print_(it);
    }
}

static void visit_DECL_VAR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_VAR);
    switch (cnode->type.spec_type) {
        case TYPE_PRIM_INT: display("INT\n"); break;
        case TYPE_PRIM_FLT: display("FLT\n"); break;
        case TYPE_STRUCT: display("STRUCT\n"); break;
    }
    display("%s\n", cnode->str);
    if (cnode->type.spec_type == TYPE_STRUCT) {
        ast_foreach(cnode->type.decls, it) {
            print_(it);
        }
    }
    if (cnode->expr) {
        print_(cnode->expr);
    }
}

static void visit_EXPR_ARR(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ARR);
    print_(cnode->arr);
    print_(cnode->ind);
}

static void visit_STMT_SCOP(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_SCOP);
    ast_foreach(cnode->decls, it) {
        print_(it);
    }
    ast_foreach(cnode->stmts, it) {
        print_(it);
    }
}

static void visit_EXPR_CALL(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_CALL);
    display("CALL %s\n", cnode->str);
    print_(cnode->expr);
}