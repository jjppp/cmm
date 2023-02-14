#include "ast.h"
#include "common.h"
#include "symtab.h"
#include <stdarg.h>
#include <string.h>

#define STRING_LIST(NODE) \
    STRINGIFY(NODE),

const char *AST_NODE_NAMES[] = {
    AST_NODES(STRING_LIST) "\0"};

const char *OP_NAMES[] = {
    OPS(STRING_LIST) "\0"};

const struct ast_visitor visitor_new;

void visitor_dispatch(const struct ast_visitor visitor, ast_t *node, void *p) {
    ASSERT(node != NULL, "%s is NULL", AST_NODE_NAMES[node->ast_kind]);
    LOG("%s at %s\n", visitor.name, AST_NODE_NAMES[node->ast_kind]);
#define AST_NODE_DISPATCH(NODE)                                      \
    case NODE:                                                       \
        ASSERT(visitor.visit_##NODE != NULL,                         \
               "%s has no method %s", visitor.name, STRINGIFY(NODE)) \
        visitor.visit_##NODE(node, p);                               \
        break;

    switch (node->ast_kind) {
        AST_NODES(AST_NODE_DISPATCH)
        default: UNREACHABLE;
    }
}

ast_t *new_ast_node(ast_kind_t kind, u32 fst_l, ...) {
#define AST_NODE_ALLOC(NODE)                 \
    case NODE:                               \
        ptr = zalloc(sizeof(NODE##_node_t)); \
        break;

    va_list ap;
    ast_t  *ptr;
    va_start(ap, fst_l);

    switch (kind) {
        AST_NODES(AST_NODE_ALLOC)
        default: assert(0);
    }
    ptr->ast_kind = kind;
    ptr->fst_l    = fst_l;
    ptr->next     = NULL;
    visitor_dispatch(visitor_new, ptr, ap);

    va_end(ap);
    return ptr;
}

static void visit_int(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_INT);
    cnode->value = va_arg(ap, i32);
}

static void visit_flt(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_FLT);
    cnode->value = va_arg(ap, f32);
}

static void visit_bin(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_BIN);
    cnode->lhs = va_arg(ap, ast_t *);
    cnode->op  = va_arg(ap, op_kind_t);
    cnode->rhs = va_arg(ap, ast_t *);
}

static void visit_unr(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_UNR);
    cnode->op  = va_arg(ap, op_kind_t);
    cnode->sub = va_arg(ap, ast_t *);
}

static void visit_iden(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_IDEN);
    symmov(cnode->str, va_arg(ap, char *));
}

static void visit_ret(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_RET);
    cnode->expr = va_arg(ap, ast_t *);
}

static void visit_whle(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_WHLE);
    cnode->cond = va_arg(ap, ast_t *);
    cnode->body = va_arg(ap, ast_t *);
}

static void visit_ifte(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_IFTE);
    cnode->cond     = va_arg(ap, ast_t *);
    cnode->tru_stmt = va_arg(ap, ast_t *);
    cnode->fls_stmt = va_arg(ap, ast_t *);
}

static void visit_scop(ast_t *node, va_list ap) {
    INSTANCE_OF(node, STMT_SCOP);
    cnode->decls = va_arg(ap, ast_t *);
    cnode->stmts = va_arg(ap, ast_t *);
}

static void visit_dot(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_DOT);
    cnode->base  = va_arg(ap, ast_t *);
    cnode->field = va_arg(ap, ast_t *);
}

static void visit_ass(ast_t *node, va_list ap) {
    INSTANCE_OF(node, EXPR_ASS);
    cnode->lhs = va_arg(ap, ast_t *);
    cnode->rhs = va_arg(ap, ast_t *);
}

static void visit_prog(ast_t *node, va_list ap) {
    INSTANCE_OF(node, CONS_PROG);
    cnode->decls = va_arg(ap, ast_t *);
}

static void visit_fun(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_FUN);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->params = va_arg(ap, ast_t *);
}

static void visit_var(ast_t *node, va_list ap) {
    INSTANCE_OF(node, DECL_VAR);
    symmov(cnode->str, va_arg(ap, char *));
    cnode->dim = va_arg(ap, i32);
}

const struct ast_visitor visitor_new = (struct ast_visitor){
    .name            = "new",
    .visit_EXPR_INT  = (void *) visit_int,
    .visit_EXPR_BIN  = (void *) visit_bin,
    .visit_EXPR_FLT  = (void *) visit_flt,
    .visit_EXPR_IDEN = (void *) visit_iden,
    .visit_EXPR_UNR  = (void *) visit_unr,
    .visit_DECL_FUN  = (void *) visit_fun,
    .visit_CONS_PROG = (void *) visit_prog,
    .visit_EXPR_ASS  = (void *) visit_ass,
    .visit_EXPR_DOT  = (void *) visit_dot,
    .visit_STMT_IFTE = (void *) visit_ifte,
    .visit_STMT_WHLE = (void *) visit_whle,
    .visit_STMT_RET  = (void *) visit_ret,
    .visit_STMT_SCOP = (void *) visit_scop,
    .visit_DECL_VAR  = (void *) visit_var};
