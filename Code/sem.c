#include "visitor.h"
#include "common.h"
#include "symtab.h"
#include "type.h"
#include "semerr.h"
#include <stdbool.h>
#include <stdio.h>

#define RET_TYPE type_t *
#define ARG typ
VISITOR_DEF(sem, type_t *);

extern bool sem_err;

// no nested functions, a pointer is sufficient
static syment_t *cur_fun;

// do not insert fields into symtab
static u32 nested_struct = 0;

void ast_check(ast_t *node, type_t *typ) {
    if (node == NULL) {
        return;
    }
    visitor_dispatch(visitor_sem, node, typ);
}

VISIT(CONS_PROG) {
    ast_iter(node->decls, it) {
        ast_check(it, typ);
    }
}

VISIT(STMT_EXPR) {
    ast_check(node->expr, typ);
}

VISIT(STMT_SCOP) {
    sym_scope_push();
    ast_iter(node->decls, it) {
        ast_check(it, typ);
    }
    ast_iter(node->stmts, it) {
        ast_check(it, typ);
    }
    sym_scope_pop();
}

VISIT(STMT_IFTE) {
    type_t cond_typ;
    ast_check(node->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO("IFTE cond_typ");
    }

    ast_check(node->tru_stmt, typ);
    ast_check(node->fls_stmt, typ);
}

VISIT(STMT_WHLE) {
    type_t cond_typ;
    ast_check(node->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO("WHLE cond_typ");
    }
    ast_check(node->body, typ);
}

VISIT(STMT_RET) {
    ast_check(node->expr, typ);
    if (!type_eq(*typ, cur_fun->typ)) {
        SEM_ERR(ERR_RET_MISMATCH, node->super.fst_l);
    }
}

VISIT(EXPR_CALL) {
    node->fun = sym_lookup(node->str);
    if (node->fun == NULL) {
        SEM_ERR(ERR_FUN_UNDEF, node->super.fst_l, node->str);
        return;
    }

    syment_t *sit = node->fun->params;
    ast_iter(node->expr, nit) {
        if (sit == NULL) {
            SEM_ERR(ERR_FUN_ARG_MISMATCH, node->super.fst_l, node->str);
            break;
        }
        ast_check(nit, typ);
        if (!type_eq(*typ, sit->typ)) {
            SEM_ERR(ERR_FUN_ARG_MISMATCH, node->super.fst_l, node->str);
        }
        sit = sit->next;
    }
    if (sit != NULL) {
        SEM_ERR(ERR_FUN_ARG_MISMATCH, node->super.fst_l, node->str);
    }
    RETURN(node->fun->typ);
}

VISIT(EXPR_IDEN) {
    node->sym = sym_lookup(node->str);
    if (node->sym == NULL) {
        SEM_ERR(ERR_VAR_UNDEF, node->super.fst_l, node->str);
        return;
    }
    RETURN(node->sym->typ);
}

VISIT(EXPR_ARR) {
    TODO("EXPR ARR");
}

// TODO: lvalue check
VISIT(EXPR_ASS) {
    type_t ltyp;
    ast_check(node->lhs, &ltyp);
    ast_check(node->rhs, typ);
    if (!type_eq(ltyp, *typ)) {
        SEM_ERR(ERR_ASS_MISMATCH, node->super.fst_l);
    }
    if (!ast_lval(node->lhs)) {
        SEM_ERR(ERR_ASS_TO_RVALUE, node->super.fst_l);
    }
}

VISIT(EXPR_DOT) {
    TODO("EXPR DOT");
}

VISIT(EXPR_INT) {
    *typ = (type_t){
        .kind   = TYPE_PRIM_INT,
        .fields = NULL,
        .dim    = 0};
}

VISIT(EXPR_FLT) {
    *typ = (type_t){
        .kind   = TYPE_PRIM_FLT,
        .fields = NULL,
        .dim    = 0};
}

#define CASE(OP) case OP:

static void logic_check(op_kind_t op, type_t typ) {
    switch (op) {
        LOGIC_OPS(CASE) {
            if (!IS_LOGIC(typ)) {
                TODO("EXPR BIN LOGIC_OP typ must be logic");
            }
            break;
        }
        default: break;
    }
}

VISIT(EXPR_BIN) {
    type_t ltyp, rtyp;
    ast_check(node->lhs, &ltyp);
    ast_check(node->rhs, &rtyp);

    if (ltyp.kind != rtyp.kind) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    if (!IS_SCALAR(ltyp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    logic_check(node->op, ltyp);
    RETURN(ltyp);
}

VISIT(EXPR_UNR) {
    type_t styp;
    ast_check(node->sub, &styp);

    if (!IS_SCALAR(styp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    logic_check(node->op, styp);
    RETURN(styp);
}

VISIT(DECL_VAR) {
    type_t expr_typ, spec_typ;

    ast_check(node->spec, &spec_typ);
    if (node->dim != 0) {
        *typ = (type_t){
            .dim      = node->dim,
            .kind     = TYPE_ARRAY,
            .elem_typ = zalloc(sizeof(type_t))};
        *typ->elem_typ = spec_typ;
        memcpy(typ->len, node->len, node->dim * sizeof(typ->len[0]));
    } else {
        *typ = spec_typ;
    }
    if (node->expr != NULL) {
        ast_check(node->expr, &expr_typ);
        if (!type_eq(*typ, expr_typ)) {
            SEM_ERR(ERR_ASS_MISMATCH, node->super.fst_l);
        }
    }
    if (!nested_struct) {
        node->sym = sym_insert(
            node->str,
            SYM_VAR,
            *typ, 0, 0);
        if (!node->sym) {
            SEM_ERR(ERR_VAR_REDEF, node->super.fst_l, node->str);
        }
    }
}

VISIT(DECL_TYP) {
    ast_check(node->spec, typ);
}

VISIT(DECL_FUN) {
    ast_check(node->spec, typ);

    // TODO: sym position
    syment_t *sym = sym_insert(
        node->str,
        SYM_FUN,
        *typ, 0, 0);
    node->sym = sym;
    if (!node->sym) {
        SEM_ERR(ERR_FUN_REDEF, node->super.fst_l, node->str);
        return;
    }

    sym_scope_push();
    ast_iter(node->params, it) {
        INSTANCE_OF_VAR(it, DECL_VAR, vnode) {
            visit_DECL_VAR(vnode, typ);
            if (sym->params == NULL) {
                sym->params = vnode->sym;
            } else {
                sym_iter(sym->params, jt) {
                    if (jt->next == NULL) {
                        jt->next = vnode->sym;
                        break;
                    }
                }
            }
        }
    }

    cur_fun = sym;
    ast_check(node->body, typ);
    cur_fun = NULL;
    sym_scope_pop();
}

VISIT(CONS_SPEC) {
    typ->kind = node->kind;
    if (node->kind == TYPE_PRIM_INT) {
        return;
    }
    if (node->kind == TYPE_PRIM_FLT) {
        return;
    }
    if (node->kind == TYPE_ARRAY) {
        TODO("TYPE_ARRAY");
    }
    if (node->done) {
        RETURN(sym_lookup(node->str)->typ);
    }
    node->done = true;
    if (node->is_ref) {
        syment_t *sym = sym_lookup(node->str);
        if (sym == NULL) {
            SEM_ERR(ERR_STRUCT_UNDEF, node->super.fst_l, node->str);
            return;
        }
        RETURN(sym->typ);
        return;
    }

    symcpy(typ->str, node->str);
    typ->fields = NULL;
    ast_iter(node->fields, it) {
        INSTANCE_OF_VAR(it, DECL_VAR, cit) {
            if (cit->expr != NULL) {
                SEM_ERR(ERR_FIELD_REDEF, it->fst_l, cit->str);
            }

            type_t field_typ;
            nested_struct++;
            // ast_check(spec var) -> typeof(spec)
            ast_check(it, &field_typ);
            nested_struct--;
            if (typ->fields == NULL) {
                typ->fields = field_alloc(field_typ, cit->str);
            } else {
                field_iter(typ->fields, jt) {
                    if (!symcmp(cit->str, jt->str)) {
                        SEM_ERR(ERR_FIELD_REDEF, it->fst_l, cit->str);
                    }
                    if (jt->next == NULL) {
                        jt->next = field_alloc(field_typ, cit->str);
                        break;
                    }
                }
            }
        }
    }
    if (NULL == sym_insert(typ->str, SYM_TYP, *typ, 0, 0)) {
        SEM_ERR(ERR_STRUCT_REDEF, node->super.fst_l, node->str);
    }
}