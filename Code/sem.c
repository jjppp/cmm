#include "ast.h"
#include "common.h"
#include "symtab.h"
#include <stdbool.h>

VISITOR_DEF(sem, type_t *);

extern bool sem_err;

// no nested functions, a pointer is sufficient
static SYM_FUN_entry *cur_fun;

void ast_check(ast_t *node, type_t *typ) {
    visitor_dispatch(visitor_sem, node, typ);
}

static bool type_eq(type_t typ1, type_t typ2) {
    if (typ1.spec_typ != typ2.spec_typ) {
        return false;
    }
    // TODO: struct equality
    return true;
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
    INSTANCE_OF(node, STMT_RET);
    ast_check(cnode->expr, typ);
    if (!type_eq(*typ, cur_fun->typ)) {
        TODO;
    }
}

static void visit_EXPR_CALL(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_CALL);
    POINTS_TO(cnode->fun, sym_lookup(cnode->str));

    if (cnode->fun == NULL) {
        TODO;
    }

    SYM_VAR_entry *sit = cnode->fun->params;
    ast_foreach(cnode->expr, nit) {
        if (sit == NULL) {
            TODO;
        }
        ast_check(nit, typ);
        if (!type_eq(*typ, sit->typ)) {
            TODO;
        }
        sit = (void *) sit->super.next;
    }
    if (sit != NULL) {
        TODO;
    }
    *typ = cnode->fun->typ;
}

static void visit_EXPR_IDEN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_IDEN);
    POINTS_TO(cnode->sym, sym_lookup(cnode->str));

    if (cnode->sym == NULL) {
        TODO;
    }
    *typ = cnode->sym->typ;
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
        .spec_typ = TYPE_PRIM_INT,
        .decls    = NULL,
        .dim      = 0};
}

static void visit_EXPR_FLT(ast_t *node, type_t *typ) {
    *typ = (type_t){
        .spec_typ = TYPE_PRIM_FLT,
        .decls    = NULL,
        .dim      = 0};
}

#define CASE(OP) case OP:

static void visit_EXPR_BIN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_BIN);
    type_t ltyp, rtyp;

    ast_check(cnode->lhs, &ltyp);
    ast_check(cnode->rhs, &rtyp);

    if (ltyp.spec_typ != rtyp.spec_typ) {
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
    INSTANCE_OF(node, DECL_VAR);

    if (cnode->expr != NULL) {
        ast_check(cnode->expr, typ);
        if (!type_eq(*typ, cnode->typ)) {
            TODO;
        }
    }

    POINTS_TO(cnode->sym, sym_insert(
                              sizeof(SYM_VAR_entry),
                              cnode->str,
                              SYM_VAR,
                              0, 0));
    ASSERT(cnode->sym != NULL, "NULL");
    cnode->sym->typ = cnode->typ;
}

static void visit_DECL_TYP(ast_t *node, type_t *typ) {
    TODO;
}

static void visit_DECL_FUN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, DECL_FUN);
    // TODO: sym position
    SYM_FUN_entry *sym;
    POINTS_TO(sym, sym_insert(
                       sizeof(SYM_FUN_entry),
                       cnode->str,
                       SYM_FUN,
                       0, 0));
    sym->typ = cnode->typ;

    sym_scope_push();
    ast_foreach(cnode->params, it) {
        INSTANCE_OF_VAR(it, DECL_VAR, vnode);

        visit_DECL_VAR(it, typ);
        vnode->sym->super.next = (void *) sym->params;
        POINTS_TO(sym->params, vnode->sym);
    }

    POINTS_TO(cur_fun, sym);
    ast_check(cnode->body, typ);
    POINTS_FREE(cur_fun);
    sym_scope_pop();
}
