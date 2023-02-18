#include "ast.h"
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
    INSTANCE_OF(node, CONS_PROG);
    ast_iter(cnode->decls, it) {
        ast_check(it, typ);
    }
}

VISIT(STMT_EXPR) {
    INSTANCE_OF(node, STMT_EXPR);
    ast_check(cnode->expr, typ);
}

VISIT(STMT_SCOP) {
    INSTANCE_OF(node, STMT_SCOP);
    sym_scope_push();
    ast_iter(cnode->decls, it) {
        ast_check(it, typ);
    }
    ast_iter(cnode->stmts, it) {
        ast_check(it, typ);
    }
    sym_scope_pop();
}

VISIT(STMT_IFTE) {
    INSTANCE_OF(node, STMT_IFTE);
    type_t cond_typ;
    ast_check(cnode->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO("IFTE cond_typ");
    }

    ast_check(cnode->tru_stmt, typ);
    ast_check(cnode->fls_stmt, typ);
}

VISIT(STMT_WHLE) {
    INSTANCE_OF(node, STMT_WHLE);
    type_t cond_typ;
    ast_check(cnode->cond, &cond_typ);
    if (!IS_LOGIC(cond_typ)) {
        TODO("WHLE cond_typ");
    }
    ast_check(cnode->body, typ);
}

VISIT(STMT_RET) {
    INSTANCE_OF(node, STMT_RET);
    ast_check(cnode->expr, typ);
    if (!type_eq(*typ, cur_fun->typ)) {
        SEM_ERR(ERR_RET_MISMATCH, node->fst_l);
    }
}

VISIT(EXPR_CALL) {
    INSTANCE_OF(node, EXPR_CALL);
    cnode->fun = sym_lookup(cnode->str);
    if (cnode->fun == NULL) {
        SEM_ERR(ERR_FUN_UNDEF, node->fst_l, cnode->str);
        return;
    }

    syment_t *sit = cnode->fun->params;
    ast_iter(cnode->expr, nit) {
        if (sit == NULL) {
            SEM_ERR(ERR_FUN_ARG_MISMATCH, node->fst_l);
        }
        ast_check(nit, typ);
        if (!type_eq(*typ, sit->typ)) {
            SEM_ERR(ERR_FUN_ARG_MISMATCH, node->fst_l);
        }
        sit = sit->next;
    }
    if (sit != NULL) {
        SEM_ERR(ERR_FUN_ARG_MISMATCH, node->fst_l);
    }
    *typ = cnode->fun->typ;
}

VISIT(EXPR_IDEN) {
    INSTANCE_OF(node, EXPR_IDEN);
    cnode->sym = sym_lookup(cnode->str);
    if (cnode->sym == NULL) {
        SEM_ERR(ERR_VAR_UNDEF, node->fst_l, cnode->str);
        return;
    }
    *typ = cnode->sym->typ;
}

VISIT(EXPR_ARR) {
    TODO("EXPR ARR");
}

// TODO: lvalue check
VISIT(EXPR_ASS) {
    INSTANCE_OF(node, EXPR_ASS);
    type_t ltyp;
    ast_check(cnode->lhs, &ltyp);
    ast_check(cnode->rhs, typ);
    if (!type_eq(ltyp, *typ)) {
        SEM_ERR(ERR_ASS_MISMATCH, node->fst_l);
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
    INSTANCE_OF(node, EXPR_BIN);
    type_t ltyp, rtyp;

    ast_check(cnode->lhs, &ltyp);
    ast_check(cnode->rhs, &rtyp);

    if (ltyp.kind != rtyp.kind) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->fst_l);
    }
    if (!IS_SCALAR(ltyp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->fst_l);
    }
    logic_check(cnode->op, ltyp);
    *typ = ltyp;
}

VISIT(EXPR_UNR) {
    INSTANCE_OF(node, EXPR_UNR);
    type_t styp;
    ast_check(cnode->sub, &styp);

    if (!IS_SCALAR(styp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->fst_l);
    }
    logic_check(cnode->op, styp);
    *typ = styp;
}

VISIT(DECL_VAR) {
    INSTANCE_OF(node, DECL_VAR);
    type_t expr_typ, spec_typ;

    ast_check(cnode->spec, &spec_typ);
    if (cnode->dim != 0) {
        *typ = (type_t){
            .dim      = cnode->dim,
            .kind     = TYPE_ARRAY,
            .elem_typ = zalloc(sizeof(type_t))};
        *typ->elem_typ = spec_typ;
        memcpy(typ->len, cnode->len, cnode->dim * sizeof(typ->len[0]));
    } else {
        *typ = spec_typ;
    }
    if (cnode->expr != NULL) {
        ast_check(cnode->expr, &expr_typ);
        if (!type_eq(*typ, expr_typ)) {
            SEM_ERR(ERR_ASS_MISMATCH, node->fst_l);
        }
    }
    if (!nested_struct) {
        cnode->sym = sym_insert(
            cnode->str,
            SYM_VAR,
            *typ, 0, 0);
        if (!cnode->sym) {
            SEM_ERR(ERR_VAR_REDEF, node->fst_l, cnode->str);
        }
    }
}

VISIT(DECL_TYP) {
    INSTANCE_OF(node, DECL_TYP);
    ast_check(cnode->spec, typ);
}

VISIT(DECL_FUN) {
    INSTANCE_OF(node, DECL_FUN);
    ast_check(cnode->spec, typ);

    // TODO: sym position
    syment_t *sym = sym_insert(
        cnode->str,
        SYM_FUN,
        *typ, 0, 0);
    cnode->sym = sym;
    if (!cnode->sym) {
        SEM_ERR(ERR_FUN_REDEF, node->fst_l, cnode->str);
        return;
    }

    sym_scope_push();
    ast_iter(cnode->params, it) {
        INSTANCE_OF_VAR(it, DECL_VAR, vnode);

        visit_DECL_VAR(it, typ);
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

    cur_fun = sym;
    ast_check(cnode->body, typ);
    cur_fun = NULL;
    sym_scope_pop();
}

VISIT(CONS_SPEC) {
    INSTANCE_OF(node, CONS_SPEC);
    typ->kind = cnode->kind;
    switch (cnode->kind) {
        case TYPE_PRIM_INT:
        case TYPE_PRIM_FLT:
            break;
        case TYPE_STRUCT:
            if (cnode->done) {
                *typ = sym_lookup(cnode->str)->typ;
                return;
            }
            cnode->done = true;
            symcpy(typ->str, cnode->str);
            typ->fields = NULL;

            if (cnode->is_ref) {
                syment_t *sym = sym_lookup(typ->str);
                if (sym == NULL) {
                    SEM_ERR(ERR_STRUCT_UNDEF, node->fst_l, cnode->str);
                    return;
                }
                *typ = sym->typ;
            } else {
                ast_iter(cnode->fields, it) {
                    INSTANCE_OF_VAR(it, DECL_VAR, cit);
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
                if (NULL == sym_insert(typ->str, SYM_TYP, *typ, 0, 0)) {
                    SEM_ERR(ERR_STRUCT_REDEF, node->fst_l, cnode->str);
                }
            }
            break;
        case TYPE_ARRAY:
            TODO("TYPE_ARRAY");
            break;
        default: UNREACHABLE;
    }
}