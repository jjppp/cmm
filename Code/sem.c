#include "ast.h"
#include "common.h"
#include "symtab.h"
#include <stdbool.h>

VISITOR_DEF(sem, type_t *);

extern bool sem_err;

void ast_check(ast_t *node, type_t *typ) {
    visitor_dispatch(visitor_sem, node, typ);
}

static void visit_CONS_PROG(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, CONS_PROG);
    ast_foreach(cnode->decls, it) {
        ast_check(it, typ);
    }
}

static void visit_STMT_EXPR(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_EXPR);
    ast_check(cnode->expr, typ);
}

static void visit_STMT_SCOP(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_SCOP);
    ast_foreach(cnode->decls, it) {
        ast_check(it, typ);
    }
    ast_foreach(cnode->stmts, it) {
        ast_check(it, typ);
    }
}

static void visit_STMT_IFTE(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_IFTE);
    type_t cond_typ;
    ast_check(cnode->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO;
    }

    ast_check(cnode->tru_stmt, typ);
    ast_check(cnode->fls_stmt, typ);
}

static void visit_STMT_WHLE(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_WHLE);
    type_t cond_typ;
    ast_check(cnode->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO;
    }
    ast_check(cnode->body, typ);
}

static void visit_STMT_RET(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_EXPR_CALL(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_EXPR_IDEN(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_EXPR_ARR(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_EXPR_ASS(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_EXPR_DOT(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_EXPR_INT(ast_t *node, type_t *typ) {
    *typ = (type_t){
        .spec_type = TYPE_PRIM_INT,
        .decls     = NULL,
        .dim       = 0};
}

static void visit_EXPR_FLT(ast_t *node, type_t *typ) {
    *typ = (type_t){
        .spec_type = TYPE_PRIM_FLT,
        .decls     = NULL,
        .dim       = 0};
}

#define CASE(OP) case OP:

static void visit_EXPR_BIN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_BIN);
    type_t ltyp, rtyp;

    ast_check(cnode->lhs, &ltyp);
    ast_check(cnode->rhs, &rtyp);

    if (ltyp.spec_type != rtyp.spec_type) {
        TODO;
    }
    if (!IS_SCALAR(ltyp)) {
        TODO;
    }
    switch (cnode->op) {
        LOGIC_OPS(CASE) {
            if (!IS_LOGIC(ltyp)) {
                TODO;
            }
            break;
        }
        default: break;
    }
    *typ = ltyp;
}

static void visit_EXPR_UNR(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_UNR);
    type_t styp;
    ast_check(cnode->sub, &styp);

    if (!IS_SCALAR(styp)) {
        TODO;
    }

    switch (cnode->op) {
        LOGIC_OPS(CASE) {
            if (!IS_LOGIC(styp)) {
                TODO;
            }
        }
        default: break;
    }
    *typ = styp;
}

static void visit_DECL_VAR(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_DECL_TYP(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_DECL_FUN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, DECL_FUN);
    // TODO: sym position
    SYM_FUN_entry *sym = sym_insert(
        sizeof(SYM_FUN_entry),
        cnode->str,
        SYM_FUN,
        0, 0);
    sym->typ = cnode->type;

    sym_scope_push();
    ast_foreach(cnode->params, it) {
        INSTANCE_OF_VAR(it, DECL_VAR, vnode);
        syment_t *vsym = sym_insert(
            sizeof(SYM_VAR_entry),
            vnode->str,
            SYM_VAR,
            0, 0);
        vsym->next  = sym->params;
        sym->params = vsym;
    }

    ast_check(cnode->body, typ);
}
