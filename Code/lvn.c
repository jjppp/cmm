#include "ast.h"
#include "cfg.h"
#include "common.h"
#include "ir.h"
#include "visitor.h"
#include "symtab.h"
#include <stdio.h>
#include <string.h>

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(IR, lvn, RET_TYPE);

#define MAX_VAL 8192 // should be enough

typedef uptr          val_t;
typedef struct cvar_t cvar_t;

static hashtab_t hashtab;
static val_t     valcnt = 1;

struct cvar_t {
    oprd_t         var;
    struct cvar_t *next;
};

// canonical variable
static cvar_t *cvar[MAX_VAL];
// what oprd is holding
static val_t holding[MAX_VAL];

static void lvn_init() { // TODO: mem leak
    memset(&hashtab, 0, sizeof(hashtab));
    memset(cvar, 0, sizeof(cvar));
    memset(holding, 0, sizeof(cvar));
}

void do_lvn(cfg_t *cfg) {
    LIST_ITER(cfg->blocks, blk) {
        LIST_ITER(blk->instrs.head, instr) {
            lvn_init();
            LIST_ITER(instr, it) {
                VISITOR_DISPATCH(IR, lvn, it, NULL);
            }
        }
    }
}

static void cvar_insert(val_t val, oprd_t var) {
    cvar_t *cp = zalloc(sizeof(cvar_t));
    LIST_APPEND(cvar[val], cp);
    *cp = (cvar_t){
        .var  = var,
        .next = NULL};
}

static void cvar_remove(val_t val, oprd_t var) {
    cvar_t *cp = NULL;
    if (cvar[val]->var.id == var.id) {
        cp        = cvar[val];
        cvar[val] = cvar[val]->next;
        goto done;
    }
    LIST_ITER(cvar[val], it) {
        if (it->next->var.id == var.id) {
            cp       = it->next;
            it->next = cp->next;
            goto done;
        }
    }
    UNREACHABLE;
done:
    free(cp);
}

static val_t unrtab_insert(oprd_t oprd) {
    const char *str = oprd_to_str(oprd);
    hashent_t  *ent = hash_lookup(&hashtab, str);
    symcpy(ent->str, str);
    ent->ptr = (void *) valcnt;
    return valcnt++;
}

static val_t unrtab_lookup(oprd_t oprd) {
    hashent_t *ent = hash_lookup(&hashtab, oprd_to_str(oprd));
    return (val_t) ent->ptr;
}

static bool abel(op_kind_t op) {
    switch (op) {
        case OP_ADD:
        case OP_MUL:
            return true;
        default:
            return false;
    }
    UNREACHABLE;
}

static val_t bintab_insert(op_kind_t op, val_t lhs, val_t rhs) {
    if (abel(op) && lhs > rhs) {
        swap(lhs, rhs);
    }
    static char str[BUFSIZ];
    snprintf(str, sizeof(str), "%lu %u %lu", lhs, op, rhs);
    hashent_t *ent = hash_lookup(&hashtab, str);
    symcpy(ent->str, str);
    ent->ptr = (void *) valcnt;
    return valcnt++;
}

static val_t bintab_lookup(op_kind_t op, val_t lhs, val_t rhs) {
    if (abel(op) && lhs > rhs) {
        swap(lhs, rhs);
    }
    static char str[BUFSIZ];
    snprintf(str, sizeof(str), "%lu %u %lu", lhs, op, rhs);
    hashent_t *ent = hash_lookup(&hashtab, str);
    return (val_t) ent->ptr;
}

static val_t oprd_to_val(oprd_t oprd) {
    switch (oprd.kind) {
        case OPRD_VAR:
            if (holding[oprd.id]) {
                return holding[oprd.id];
            }
        case OPRD_LIT: {
            val_t val = unrtab_lookup(oprd);
            if (!val) {
                val = unrtab_insert(oprd);
            }
            cvar_insert(val, oprd);
            return val;
        }
    }
    UNREACHABLE;
}

static void oprd_holds(oprd_t oprd, val_t val) { // oprd = val
    if (holding[oprd.id]) {                      // kill oprd
        cvar_remove(holding[oprd.id], oprd);
        holding[oprd.id] = 0;
    }
    cvar_insert(val, oprd);
    holding[oprd.id] = val;
}

VISIT(IR_ASSIGN) { // tar = lhs
    val_t expr = oprd_to_val(node->lhs);
    if (cvar[expr] != NULL) {
        node->lhs = cvar[expr]->var;
    }
    oprd_holds(node->tar, expr);
}

VISIT(IR_BINARY) {
    val_t lhs  = oprd_to_val(node->lhs);
    val_t rhs  = oprd_to_val(node->rhs);
    val_t expr = bintab_lookup(node->op, lhs, rhs);
    if (!expr) {
        expr = bintab_insert(node->op, lhs, rhs);
    }
    if (cvar[expr] != NULL) {
        node->lhs  = cvar[expr]->var;
        node->kind = IR_ASSIGN;
    } else {
        if (cvar[lhs] != NULL) {
            node->lhs = cvar[lhs]->var;
        }
        if (cvar[rhs] != NULL) {
            node->rhs = cvar[rhs]->var;
        }
    }
    oprd_holds(node->tar, expr);
}

VISIT(IR_DREF) {
    val_t lhs = oprd_to_val(node->lhs);
    if (cvar[lhs] != NULL) {
        node->lhs = cvar[lhs]->var;
    }
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_LOAD) {
    val_t lhs = oprd_to_val(node->lhs);
    if (cvar[lhs] != NULL) {
        node->lhs = cvar[lhs]->var;
    }
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_STORE) {
    val_t lhs = oprd_to_val(node->lhs);
    if (cvar[lhs] != NULL) {
        node->lhs = cvar[lhs]->var;
    }
}

VISIT(IR_BRANCH) {
    val_t lhs = oprd_to_val(node->lhs);
    val_t rhs = oprd_to_val(node->rhs);
    if (cvar[lhs] != NULL) {
        node->lhs = cvar[lhs]->var;
    }
    if (cvar[rhs] != NULL) {
        node->rhs = cvar[rhs]->var;
    }
}

VISIT(IR_RETURN) {
    val_t expr = oprd_to_val(node->lhs);
    if (cvar[expr] != NULL) {
        node->lhs = cvar[expr]->var;
    }
}

VISIT(IR_CALL) {
    val_t expr = oprd_to_val(node->lhs);
    if (cvar[expr] != NULL) {
        node->lhs = cvar[expr]->var;
    }
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_READ) {
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_WRITE) {
    val_t expr = oprd_to_val(node->lhs);
    if (cvar[expr] != NULL) {
        node->lhs = cvar[expr]->var;
    }
    oprd_holds(node->tar, expr);
}

VISIT(IR_ARG) {
    val_t expr = oprd_to_val(node->lhs);
    if (cvar[expr] != NULL) {
        node->lhs = cvar[expr]->var;
    }
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_DEC);
VISIT_EMPTY(IR_PARAM);
VISIT_EMPTY(IR_LABEL);