#include "ast.h"
#include "visitor.h"
#include "common.h"
#include "symtab.h"
#include "type.h"
#include "semerr.h"
#include <stdbool.h>
#include <stdio.h>

#define RET_TYPE type_t *
#define ARG typ
VISITOR_DEF(AST_NODES, sem, RET_TYPE);

extern bool sem_err;

// no nested functions, a pointer is sufficient
static syment_t *cur_fun;

// do not insert fields into symtab
static u32 nested_struct = 0;

type_t ast_check(ast_t *node) {
    if (node == NULL) {
        return (type_t){.kind = TYPE_ERR};
    }
    type_t ARG = {0};
    visitor_dispatch(visitor_sem, node, &ARG);
    return ARG;
}

VISIT(CONS_PROG) {
    ast_iter(node->decls, it) {
        ast_check(it);
    }
}

VISIT(STMT_EXPR) {
    ast_check(node->expr);
}

VISIT(STMT_SCOP) {
    sym_scope_push();
    ast_iter(node->decls, it) {
        ast_check(it);
    }
    ast_iter(node->stmts, it) {
        ast_check(it);
    }
    sym_scope_pop();
}

VISIT(STMT_IFTE) {
    type_t cond_typ = ast_check(node->cond);
    if (!IS_LOGIC(cond_typ)) {
        TODO("IFTE cond_typ");
    }

    ast_check(node->tru_stmt);
    ast_check(node->fls_stmt);
}

VISIT(STMT_WHLE) {
    type_t cond_typ = ast_check(node->cond);
    if (!IS_LOGIC(cond_typ)) {
        TODO("WHLE cond_typ");
    }
    ast_check(node->body);
}

VISIT(STMT_RET) {
    type_t expr_typ = ast_check(node->expr);
    if (!type_eq(expr_typ, cur_fun->typ)) {
        SEM_ERR(ERR_RET_MISMATCH, node->expr->fst_l);
    }
}

VISIT(EXPR_CALL) {
    node->fun = sym_lookup(node->str);
    if (node->fun == NULL) {
        SEM_ERR(ERR_FUN_UNDEF, node->super.fst_l, node->str);
    }
    if (node->fun->kind != SYM_FUN) {
        // TODO: print fun
        SEM_ERR(ERR_CALL_NON_FUN, node->super.fst_l, node->str);
    }

    syment_t *sit = node->fun->params;
    ast_iter(node->expr, nit) {
        if (sit == NULL) {
            SEM_ERR(ERR_FUN_ARG_MISMATCH, nit->fst_l, node->str);
        }
        type_t arg_typ = ast_check(nit);
        if (!type_eq(arg_typ, sit->typ)) {
            SEM_ERR(ERR_FUN_ARG_MISMATCH, nit->fst_l, node->str);
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
    }
    RETURN(node->sym->typ);
}

VISIT(EXPR_ARR) {
    u32 len = 0;
    ast_iter(node->ind, it) {
        len++;
        type_t ind_typ = ast_check(it);
        if (!IS_LOGIC(ind_typ)) {
            // TODO: print index
            SEM_ERR(ERR_ACC_INDEX, it->fst_l);
        }
    }
    type_t arr_typ = ast_check(node->arr);
    if (arr_typ.kind != TYPE_ARRAY) {
        // TODO: print array
        SEM_ERR(ERR_ACC_NON_ARRAY, node->arr->fst_l);
    }
    if (len != arr_typ.dim) {
        arr_typ.dim -= len;
        RETURN(arr_typ);
    }
    RETURN(*arr_typ.elem_typ);
}

VISIT(EXPR_ASS) {
    type_t ltyp = ast_check(node->lhs);
    type_t rtyp = ast_check(node->rhs);
    if (!type_eq(ltyp, rtyp)) {
        SEM_ERR(ERR_ASS_MISMATCH, node->super.fst_l);
    }
    if (!ast_lval(node->lhs)) {
        SEM_ERR(ERR_ASS_TO_RVALUE, node->lhs->fst_l);
    }
    RETURN(ltyp);
}

VISIT(EXPR_DOT) {
    type_t base_typ = ast_check(node->base);
    if (base_typ.kind != TYPE_STRUCT) {
        // TODO: print base
        SEM_ERR(ERR_ACC_NON_STRUCT, node->base->fst_l);
    }
    printf("%d %s\n", node->super.fst_l, node->str);
    field_iter(base_typ.fields, it) {
        if (!symcmp(it->str, node->str)) {
            RETURN(it->typ);
        }
    }
    SEM_ERR(ERR_ACC_UNDEF_FIELD, node->super.fst_l, node->str);
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
    type_t ltyp = ast_check(node->lhs);
    type_t rtyp = ast_check(node->rhs);

    if (!IS_SCALAR(ltyp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    if (!type_eq(ltyp, rtyp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    logic_check(node->op, ltyp);
    RETURN(ltyp);
}

VISIT(EXPR_UNR) {
    type_t styp = ast_check(node->sub);
    if (!IS_SCALAR(styp)) {
        SEM_ERR(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    logic_check(node->op, styp);
    RETURN(styp);
}

VISIT(DECL_VAR) {
    type_t spec_typ = ast_check(node->spec);
    type_t var_typ  = spec_typ;
    if (node->dim != 0) {
        var_typ = (type_t){
            .dim      = node->dim,
            .kind     = TYPE_ARRAY,
            .elem_typ = zalloc(sizeof(type_t)),
            .is_ref   = false};
        *var_typ.elem_typ = spec_typ;
        memcpy(var_typ.len, node->len, node->dim * sizeof(var_typ.len[0]));
    }
    if (node->expr != NULL) {
        type_t expr_typ = ast_check(node->expr);
        if (!type_eq(var_typ, expr_typ)) {
            SEM_ERR(ERR_ASS_MISMATCH, node->expr->fst_l);
        }
    }
    if (!nested_struct) {
        node->sym = sym_insert(
            node->str,
            SYM_VAR,
            var_typ, 0, 0);
        if (!node->sym) {
            typ_free(var_typ);
            SEM_ERR(ERR_VAR_REDEF, node->super.fst_l, node->str);
        }
    }
    RETURN(var_typ);
}

VISIT(DECL_TYP) {
    RETURN(ast_check(node->spec));
}

VISIT(CONS_FUN) {
    type_t ret_typ = ast_check(node->spec);

    // TODO: sym position
    syment_t *sym = sym_insert(
        node->str,
        SYM_FUN,
        ret_typ, 0, 0);
    node->sym = sym;
    if (!node->sym) {
        node->sym = sym_lookup(node->str);
        if (node->sym->kind != SYM_FUN) {
            SEM_ERR(ERR_FUN_REDEF, node->super.fst_l, node->str);
        }
        if (node->sym->body != NULL) {
            SEM_ERR(ERR_FUN_REDEF, node->super.fst_l, node->str);
        }
        if (!type_eq(node->sym->typ, ret_typ)) {
            SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
        }
        node->sym->body = node->body;
        syment_t *jt    = node->sym->params;
        sym_scope_push();
        ast_iter(node->params, it) {
            type_t param_typ = ast_check(it);
            if (jt == NULL) {
                sym_scope_pop();
                SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
            }
            if (!type_eq(param_typ, jt->typ)) {
                sym_scope_pop();
                SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
            }
            jt = jt->next;
        }
        sym_scope_pop();
        node->sym->body = node->body;
        RETURN(ret_typ);
    }

    sym_scope_push();
    ast_iter(node->params, it) {
        INSTANCE_OF(it, DECL_VAR) {
            ast_check(it);
            LIST_APPEND(sym->params, cnode->sym);
        }
    }

    cur_fun         = sym;
    node->sym->body = node->body;
    ast_check(node->body);
    cur_fun = NULL;
    sym_scope_pop();
}

VISIT(CONS_SPEC) {
    if (node->kind == TYPE_PRIM_INT
        || node->kind == TYPE_PRIM_FLT) {
        RETURN((type_t){.kind = node->kind});
    }
    if (node->done) {
        RETURN(sym_lookup(node->str)->typ);
    }
    node->done = true;
    if (node->is_ref) {
        syment_t *sym = sym_lookup(node->str);
        if (sym == NULL) {
            SEM_ERR(ERR_STRUCT_UNDEF, node->super.fst_l, node->str);
        }
        if (sym->kind != SYM_TYP) {
            SEM_ERR(ERR_STRUCT_UNDEF, node->super.fst_l, node->str);
        }
        *typ        = sym->typ;
        typ->is_ref = true;
        RETURN(*typ);
    }

    symcpy(typ->str, node->str);
    typ->kind   = node->kind;
    typ->fields = NULL;
    ast_iter(node->fields, it) {
        INSTANCE_OF(it, DECL_VAR) {
            // if (cnode->expr != NULL) {
            //     SEM_ERR(ERR_FIELD_REDEF, it->fst_l, cnode->str);
            // }

            nested_struct++;
            // ast_check(spec var) -> typeof(spec)
            type_t field_typ = ast_check(it);
            nested_struct--;

            field_iter(typ->fields, jt) {
                if (!symcmp(jt->str, cnode->str)) {
                    typ_free(*typ);
                    SEM_ERR(ERR_FIELD_REDEF, it->fst_l, cnode->str);
                }
            }
            LIST_APPEND(typ->fields, field_alloc(field_typ, cnode->str));
        }
    }
    if (NULL == sym_insert(typ->str, SYM_TYP, *typ, 0, 0)) {
        typ_free(*typ);
        SEM_ERR(ERR_STRUCT_REDEF, node->super.fst_l, node->str);
    }
}

VISIT(DECL_FUN) {
    type_t ret_typ = ast_check(node->spec);

    // TODO: sym position
    syment_t *sym = sym_insert(
        node->str,
        SYM_FUN,
        ret_typ, 0, 0);
    node->sym = sym;
    if (!node->sym) {
        node->sym = sym_lookup(node->str);
        if (node->sym->kind != SYM_FUN) {
            SEM_ERR(ERR_FUN_REDEF, node->super.fst_l, node->str);
        }
        if (!type_eq(node->sym->typ, ret_typ)) {
            SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
        }
        sym_scope_push();
        syment_t *jt = node->sym->params;
        ast_iter(node->params, it) {
            type_t param_typ = ast_check(it);
            if (jt == NULL) {
                sym_scope_pop();
                SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
            }
            if (!type_eq(param_typ, jt->typ)) {
                sym_scope_pop();
                SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
            }
            jt = jt->next;
        }
        sym_scope_pop();
        RETURN(ret_typ);
    }

    sym_scope_push();
    ast_iter(node->params, it) {
        INSTANCE_OF(it, DECL_VAR) {
            ast_check(it);
            LIST_APPEND(sym->params, cnode->sym);
        }
    }
    sym_scope_pop();
}