#include "ast.h"
#include "common.h"
#include "symtab.h"
#include "type.h"
#include <stdbool.h>

VISITOR_DEF(sem, type_t *);

extern bool sem_err;

// no nested functions, a pointer is sufficient
static syment_t *cur_fun;

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
    sym_scope_push();
    ast_foreach(cnode->decls, it) {
        ast_check(it, typ);
    }
    ast_foreach(cnode->stmts, it) {
        ast_check(it, typ);
    }
    sym_scope_pop();
}

static void visit_STMT_IFTE(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_IFTE);
    type_t cond_typ;
    ast_check(cnode->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO("IFTE cond_typ");
    }

    ast_check(cnode->tru_stmt, typ);
    ast_check(cnode->fls_stmt, typ);
}

static void visit_STMT_WHLE(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_WHLE);
    type_t cond_typ;
    ast_check(cnode->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO("WHLE cond_typ");
    }
    ast_check(cnode->body, typ);
}

static void visit_STMT_RET(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, STMT_RET);
    ast_check(cnode->expr, typ);
    if (!type_eq(*typ, cur_fun->typ)) {
        TODO("RET typ");
    }
}

static void visit_EXPR_CALL(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_CALL);
    cnode->fun = sym_lookup(cnode->str);

    if (cnode->fun == NULL) {
        TODO("CALL undef fun");
    }

    syment_t *sit = cnode->fun->params;
    ast_foreach(cnode->expr, nit) {
        if (sit == NULL) {
            TODO("CALL arg cnt");
        }
        ast_check(nit, typ);
        if (!type_eq(*typ, sit->typ)) {
            TODO("CALL arg typ");
        }
        sit = sit->next;
    }
    if (sit != NULL) {
        TODO("CALL arg cnt");
    }
    *typ = cnode->fun->typ;
}

static void visit_EXPR_IDEN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_IDEN);
    cnode->sym = sym_lookup(cnode->str);

    if (cnode->sym == NULL) {
        TODO("EXPR undef IDEN");
    }
    *typ = cnode->sym->typ;
}

static void visit_EXPR_ARR(ast_t *node, type_t *typ) {
    TODO("EXPR ARR");
}

// TODO: lvalue check
static void visit_EXPR_ASS(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_ASS);
    type_t ltyp;
    ast_check(cnode->lhs, &ltyp);
    ast_check(cnode->rhs, typ);
    if (!type_eq(ltyp, *typ)) {
        TODO("err ASS typ match");
    }
}

static void visit_EXPR_DOT(ast_t *node, type_t *typ) {
    TODO("EXPR DOT");
}

static void visit_EXPR_INT(ast_t *node, type_t *typ) {
    *typ = (type_t){
        .kind   = TYPE_PRIM_INT,
        .fields = NULL,
        .dim    = 0};
}

static void visit_EXPR_FLT(ast_t *node, type_t *typ) {
    *typ = (type_t){
        .kind   = TYPE_PRIM_FLT,
        .fields = NULL,
        .dim    = 0};
}

#define CASE(OP) case OP:

static void visit_EXPR_BIN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, EXPR_BIN);
    type_t ltyp, rtyp;

    ast_check(cnode->lhs, &ltyp);
    ast_check(cnode->rhs, &rtyp);

    if (ltyp.kind != rtyp.kind) {
        TODO("EXPR BIN typ match");
    }
    if (!IS_SCALAR(ltyp)) {
        TODO("EXPR BIN typ must be scalar");
    }
    switch (cnode->op) {
        LOGIC_OPS(CASE) {
            if (!IS_LOGIC(ltyp)) {
                TODO("EXPR BIN LOGIC_OP typ must be logic");
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
        TODO("EXPR UNR typ must be scalar");
    }

    switch (cnode->op) {
        LOGIC_OPS(CASE) {
            if (!IS_LOGIC(styp)) {
                TODO("EXPR UNR LOGIC_OP typ must be logic");
            }
        }
        default: break;
    }
    *typ = styp;
}

static void visit_DECL_VAR(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, DECL_VAR);
    type_t expr_typ;

    if (cnode->expr != NULL) {
        ast_check(cnode->expr, &expr_typ);
    }
    ast_check(cnode->spec, typ);
    if (cnode->expr != NULL) {
        if (!type_eq(*typ, expr_typ)) {
            TODO("DECL_VAR init typ");
        }
    }

    cnode->sym = sym_insert(
        cnode->str,
        SYM_VAR,
        0, 0);
    cnode->sym->typ = *typ;
}

static void visit_DECL_TYP(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, DECL_TYP);
    ast_check(cnode->spec, typ);
}

static void visit_DECL_FUN(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, DECL_FUN);
    ast_check(cnode->spec, typ);

    // TODO: sym position
    syment_t *sym = sym_insert(
        cnode->str,
        SYM_FUN,
        0, 0);
    sym->typ   = *typ;
    cnode->sym = sym;

    sym_scope_push();
    ast_foreach(cnode->params, it) {
        INSTANCE_OF_VAR(it, DECL_VAR, vnode);

        visit_DECL_VAR(it, typ);
        if (sym->params == NULL) {
            sym->params = vnode->sym;
        } else {
            sym_foreach(sym->params, jt) {
                if (jt->next == NULL) {
                    jt->next = vnode->sym;
                    break;
                }
            }
        }
    }

    cur_fun = sym;
    ast_check(cnode->body, typ);

    POINTS_FREE(cur_fun, zfree);
    sym_scope_pop();
}

static void visit_CONS_SPEC(ast_t *node, type_t *typ) {
    INSTANCE_OF(node, CONS_SPEC);
    typ->kind = cnode->kind;
    switch (cnode->kind) {
        case TYPE_PRIM_INT:
        case TYPE_PRIM_FLT:
            break;
        case TYPE_STRUCT:
            symcpy(typ->str, cnode->str);
            typ->fields = NULL;

            if (cnode->is_ref) {
                syment_t *sym = sym_lookup(typ->str);
                if (sym == NULL) {
                    TODO("err undef STRUCT");
                }
                *typ = sym->typ;
            } else {
                field_t *tail = NULL;
                ast_foreach(cnode->fields, it) {
                    INSTANCE_OF_VAR(it, DECL_VAR, cit);
                    if (cit->expr != NULL) {
                        TODO("err STRUCT field init");
                    }

                    type_t field_typ;
                    // ast_check(spec var) -> typeof(spec)
                    ast_check(it, &field_typ);
                    if (typ->fields == NULL) {
                        typ->fields = field_alloc(field_typ, cit->str);
                        tail        = typ->fields;
                    } else {
                        tail->next = field_alloc(field_typ, cit->str);
                        tail       = tail->next;
                    }
                }
                syment_t *sym = sym_insert(
                    typ->str,
                    SYM_TYP,
                    0, 0);
                sym->typ = *typ;
            }
            break;
        case TYPE_ARRAY:
            TODO("TYPE_ARRAY");
            break;
        default: UNREACHABLE;
    }
}