#include "ast.h"
#include "ir.h"
#include "visitor.h"
#include "common.h"
#include "symtab.h"
#include "type.h"
#include "semerr.h"
#include <stdbool.h>
#include <stdio.h>

#define RET_TYPE type_t *
#define ARG typ
VISITOR_DEF(AST, sem, RET_TYPE);

extern bool   sem_err;
extern type_t type_int, type_flt, type_err;

// no nested functions, a pointer is sufficient
static syment_t *cur_fun;

// do not insert fields into symtab
static u32 nested_struct = 0;

type_t ast_check(AST_t *node) {
    if (node == NULL) {
        return type_err;
    }
    type_t ARG = {0};
    VISITOR_DISPATCH(AST, sem, node, &ARG);
    return ARG;
}

VISIT(CONS_PROG) {
    LIST_FOREACH(node->decls, ast_check);
}

VISIT(STMT_EXPR) {
    ast_check(node->expr);
}

VISIT(STMT_SCOP) {
    sym_scope_push();
    LIST_FOREACH(node->decls, ast_check);
    LIST_FOREACH(node->stmts, ast_check);
    sym_scope_pop();
}

VISIT(STMT_IFTE) {
    type_t cond_typ = ast_check(node->cond);
    if (!IS_LOGIC(cond_typ)) {
        SEM_ERR_RETURN(ERR_COND_TYPE, node->cond->fst_l);
    }
    ast_check(node->tru_stmt);
    ast_check(node->fls_stmt);
}

VISIT(STMT_WHLE) {
    type_t cond_typ = ast_check(node->cond);
    if (!IS_LOGIC(cond_typ)) {
        SEM_ERR_RETURN(ERR_COND_TYPE, node->cond->fst_l);
    }
    ast_check(node->body);
}

VISIT(STMT_RET) {
    type_t expr_typ = ast_check(node->expr);
    if (!type_eq(expr_typ, cur_fun->typ)) {
        SEM_ERR_RETURN(ERR_RET_MISMATCH, node->expr->fst_l);
    }
}

VISIT(EXPR_CALL) {
    syment_t *sym = sym_lookup(node->str);
    if (sym == NULL) {
        SEM_ERR_RETURN(ERR_FUN_UNDEF, node->super.fst_l, node->str);
    }
    if (sym->kind != SYM_FUN) {
        SEM_ERR_RETURN(ERR_CALL_NON_FUN, node->super.fst_l, node->str);
    }

    if (node->nexpr != sym->nparam) {
        SEM_ERR_RETURN(ERR_FUN_ARG_MISMATCH, node->super.fst_l, node->str);
    }

    syment_t *sit = sym->params;
    LIST_ITER(node->expr, nit) {
        type_t arg_typ = ast_check(nit);
        if (!type_eq(arg_typ, sit->typ)) {
            SEM_ERR_RETURN(ERR_FUN_ARG_MISMATCH, nit->fst_l, node->str);
        }
        sit = sit->next;
    }
    RETURN(sym->typ);
}

VISIT(EXPR_IDEN) {
    syment_t *sym = sym_lookup(node->str);
    if (sym == NULL) {
        SEM_ERR_RETURN(ERR_VAR_UNDEF, node->super.fst_l, node->str);
    }
    node->sym = sym;
    RETURN(sym->typ);
}

VISIT(EXPR_ARR) {
    type_t arr_typ = ast_check(node->arr);
    u32    len     = 0;
    bool   err     = false;
    if (arr_typ.kind != TYPE_ARRAY) {
        SEM_ERR(ERR_ACC_NON_ARRAY, node->arr->fst_l);
        err = true;
    }
    LIST_ITER(node->ind, it) {
        len++;
        type_t ind_typ = ast_check(it);
        if (!IS_LOGIC(ind_typ)) {
            SEM_ERR(ERR_ACC_INDEX, it->fst_l);
            err = true;
        }
    }
    if (err) {
        RETURN(type_err);
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
        SEM_ERR_RETURN(ERR_ASS_MISMATCH, node->super.fst_l);
    }
    if (!ast_lval(node->lhs)) {
        SEM_ERR_RETURN(ERR_ASS_TO_RVALUE, node->lhs->fst_l);
    }
    RETURN(ltyp);
}

VISIT(EXPR_DOT) {
    type_t base_typ = ast_check(node->base);
    if (base_typ.kind != TYPE_STRUCT) {
        SEM_ERR_RETURN(ERR_ACC_NON_STRUCT, node->base->fst_l);
    }
    LIST_ITER(base_typ.fields, it) {
        if (!symcmp(it->str, node->str)) {
            node->field = it;
            node->typ   = it->typ;
            RETURN(it->typ);
        }
    }
    SEM_ERR_RETURN(ERR_ACC_UNDEF_FIELD, node->super.fst_l, node->str);
}

VISIT(EXPR_INT) {
    RETURN(type_int);
}

VISIT(EXPR_FLT) {
    RETURN(type_flt);
}

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
        SEM_ERR_RETURN(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    if (!type_eq(ltyp, rtyp)) {
        SEM_ERR_RETURN(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
    }
    logic_check(node->op, ltyp);
    RETURN(ltyp);
}

VISIT(EXPR_UNR) {
    type_t styp = ast_check(node->sub);
    if (!IS_SCALAR(styp)) {
        SEM_ERR_RETURN(ERR_EXP_OPERAND_MISMATCH, node->super.fst_l);
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
        var_typ.size = typ_set_size(&var_typ);
    }
    if (node->expr != NULL) {
        type_t expr_typ = ast_check(node->expr);
        if (nested_struct) {
            SEM_ERR(ERR_FIELD_REDEF, node->expr->fst_l, node->str);
        } else if (!type_eq(var_typ, expr_typ)) {
            SEM_ERR(ERR_ASS_MISMATCH, node->expr->fst_l);
        }
    }
    if (!nested_struct) {
        syment_t *sym = sym_insert(
            node->str,
            SYM_VAR);
        if (!sym) {
            typ_free(var_typ);
            SEM_ERR_RETURN(ERR_VAR_REDEF, node->super.fst_l, node->str);
        }
        sym->typ  = var_typ;
        sym->var  = var_alloc(node->str);
        node->sym = sym;
    }
    RETURN(var_typ);
}

VISIT(DECL_TYP) {
    RETURN(ast_check(node->spec));
}

static void check_params(AST_t *params, syment_t *sym_params, u32 fst_l, const char *str, bool err) {
    syment_t *jt = sym_params;
    LIST_ITER(params, it) {
        type_t param_typ = ast_check(it);
        if (!err && !type_eq(param_typ, jt->typ)) {
            SEM_ERR(ERR_FUN_DEC_COLLISION, fst_l, str);
            err = true;
        }
        if (jt != NULL) {
            jt = jt->next;
        }
    }
}

VISIT(CONS_FUN) {
    type_t    ret_typ = ast_check(node->spec);
    syment_t *sym     = sym_insert(node->str, SYM_FUN);

    if (sym != NULL) {
        sym->typ = ret_typ;
        sym_scope_push();
        LIST_ITER(node->params, it) {
            INSTANCE_OF(it, DECL_VAR) {
                ast_check(it);
                LIST_APPEND(sym->params, sym_lookup(cnode->str));
            }
        }
        sym->nparam = LIST_LENGTH(sym->params);
        if (node->body != NULL) {
            sym->body = node->body;
            cur_fun   = sym;
            ast_check(node->body);
            cur_fun = NULL;
        }
        sym_scope_pop();
        RETURN(ret_typ);
    }

    sym      = sym_lookup(node->str);
    bool err = false;
    if (sym->kind != SYM_FUN) {
        SEM_ERR(ERR_FUN_REDEF, node->super.fst_l, node->str);
        err = true;
    } else if (sym->body != NULL && node->body != NULL) {
        SEM_ERR(ERR_FUN_REDEF, node->super.fst_l, node->str);
        err = true;
    } else if (!type_eq(sym->typ, ret_typ)) {
        SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
        err = true;
    } else if (node->nparam != sym->nparam) {
        SEM_ERR(ERR_FUN_DEC_COLLISION, node->super.fst_l, node->str);
        err = true;
    }

    sym_scope_push();
    check_params(node->params, sym->params, node->super.fst_l, node->str, err);
    if (node->body != NULL) {
        if (!err) {
            sym->body = node->body;
        }
        cur_fun = sym;
        ast_check(node->body);
        cur_fun = NULL;
    }
    sym_scope_pop();
    RETURN(ret_typ);
}

VISIT(CONS_SPEC) {
    if (node->kind == TYPE_PRIM_INT) {
        RETURN(type_int);
    } else if (node->kind == TYPE_PRIM_FLT) {
        RETURN(type_flt);
    } else if (node->done) {
        RETURN(sym_lookup(node->str)->typ);
    }
    node->done = true;
    if (node->is_ref) {
        syment_t *sym = sym_lookup(node->str);
        if (sym == NULL) {
            SEM_ERR_RETURN(ERR_STRUCT_UNDEF, node->super.fst_l, node->str);
        }
        if (sym->kind != SYM_TYP) {
            SEM_ERR_RETURN(ERR_STRUCT_UNDEF, node->super.fst_l, node->str);
        }
        *typ        = sym->typ;
        typ->is_ref = true;
        RETURN(*typ);
    }

    symcpy(typ->str, node->str);
    typ->kind   = node->kind;
    typ->fields = NULL;
    bool err    = false;

    syment_t *sym = sym_lookup(node->str);
    if (sym != NULL) {
        SEM_ERR(ERR_STRUCT_REDEF, node->super.fst_l, node->str);
        err = true;
    }

    LIST_ITER(node->fields, it) {
        INSTANCE_OF(it, DECL_VAR) {
            nested_struct++;
            // ast_check(spec var) -> typeof(spec)
            type_t field_typ = ast_check(it);
            nested_struct--;

            if (field_exist(typ->fields, cnode->str)) {
                typ_free(field_typ);
                SEM_ERR(ERR_FIELD_REDEF, cnode->super.fst_l, cnode->str);
            } else if (!err && field_typ.kind != TYPE_ERR) {
                LIST_APPEND(typ->fields, field_alloc(field_typ, cnode->str));
            }
        }
    }
    sym = sym_insert(node->str, SYM_TYP);
    if (!err && sym == NULL) {
        SEM_ERR(ERR_STRUCT_REDEF, node->super.fst_l, node->str);
        err = true;
    }
    if (err) {
        RETURN(type_err);
    }
    typ->size = typ_set_size(typ);
    sym->typ  = *typ;
    RETURN(*typ);
}
