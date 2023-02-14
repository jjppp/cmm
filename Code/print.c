#include "ast.h"
#include "common.h"
#include <stdarg.h>

const struct ast_visitor visitor_print;

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

static void visit_int(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_INT);
    display("value: %d\n", cnode->value);
}

static void visit_flt(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_FLT);
    display("value: %lf\n", cnode->value);
}

static void visit_bin(ast_t *node, va_list ap) {
    extern const char *OP_NAMES[];
    INSTANCE_OF(node, EXPR_BIN);
    print_(cnode->lhs);
    display("%s\n", OP_NAMES[cnode->op]);
    print_(cnode->rhs);
}

static void visit_unr(ast_t *node, va_list ap) {
    extern const char *OP_NAMES[];
    INSTANCE_OF(node, EXPR_UNR);
    display("%d\n", OP_NAMES[cnode->op]);
    print_(cnode->sub);
}

static void visit_iden(ast_t *node, va_list ap) {
    // TODO
}

static void visit_ret(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_RET);
    print_(cnode->expr);
}

static void visit_whle(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_WHLE);
    print_(cnode->cond);
    print_(cnode->body);
}

static void visit_ifte(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_IFTE);
    print_(cnode->cond);
    print_(cnode->tru_stmt);
    print_(cnode->fls_stmt);
}

static void visit_dot(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_DOT);
    print_(cnode->base);
    print_(cnode->field);
}

static void visit_ass(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ASS);
    print_(cnode->lhs);
    print_(cnode->rhs);
}

static void visit_prog(ast_t *node, va_list ap) {
    INSTANCE_OF(node, CONS_PROG);
    ast_foreach(cnode->decls, it) {
        print_(it);
    }
}

static void visit_fun(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_FUN);
    ast_foreach(cnode->body, it) {
        print_(it);
    }
}

static void visit_var(ast_t *node, va_list ap) {
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

static void visit_scop(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_SCOP);
    ast_foreach(cnode->decls, it) {
        print_(it);
    }
    ast_foreach(cnode->stmts, it) {
        print_(it);
    }
}

const struct ast_visitor visitor_print = (struct ast_visitor){
    .name            = "print",
    .visit_EXPR_INT  = (void *) visit_int,
    .visit_EXPR_BIN  = (void *) visit_bin,
    .visit_EXPR_FLT  = (void *) visit_flt,
    .visit_EXPR_IDEN = (void *) visit_iden,
    .visit_EXPR_UNR  = (void *) visit_unr,
    .visit_DECL_FUN  = (void *) visit_fun,
    .visit_DECL_VAR  = (void *) visit_var,
    .visit_CONS_PROG = (void *) visit_prog,
    .visit_EXPR_ASS  = (void *) visit_ass,
    .visit_EXPR_DOT  = (void *) visit_dot,
    .visit_STMT_IFTE = (void *) visit_ifte,
    .visit_STMT_WHLE = (void *) visit_whle,
    .visit_STMT_RET  = (void *) visit_ret,
    .visit_STMT_SCOP = (void *) visit_scop};
