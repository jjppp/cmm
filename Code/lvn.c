#include "common.h"
#include "ir.h"
#include "map.h"
#include "visitor.h"
#include "symtab.h"
#include "opt.h"
#include <stdio.h>
#include <string.h>

#define RET_TYPE va_list
#define ARG ap
VISITOR_DEF(IR, lvn, RET_TYPE);

typedef uptr          val_t;
typedef struct cvar_t cvar_t;

static hashtab_t hashtab;
static val_t     valcnt;

struct cvar_t {
    oprd_t         var;
    struct cvar_t *next;
};

// canonical variable, val_t => cvar_t*
static map_t cvar_map;
// what oprd is holding, oprd_t.id => val_t
static map_t holding_map;

static i32 val_cmp(const void *lhs, const void *rhs) {
    val_t lv = (val_t) lhs;
    val_t rv = (val_t) rhs;
    if (lv > rv) {
        return 1;
    } else if (lv < rv) {
        return -1;
    }
    return 0;
}

static void lvn_init() { // TODO: mem leak
    memset(&hashtab, 0, sizeof(hashtab));
    valcnt = 1;
    map_init(&cvar_map, val_cmp);
    map_init(&holding_map, oprd_cmp);
}

static void cvar_free(cvar_t *cv) {
    if (!cv) {
        return;
    }
    cvar_free(cv->next);
    free(cv);
}

static void lvn_fini() {
    map_iter(&cvar_map, it) {
        cvar_free(it.val);
    }
}

void do_lvn(cfg_t *cfg) {
    LIST_ITER(cfg->blocks, blk) {
        lvn_init();
        LIST_ITER(blk->instrs.head, instr) {
            VISITOR_DISPATCH(IR, lvn, instr, NULL);
        }
        lvn_fini();
    }
}

static void cvar_insert(val_t val, oprd_t var) {
    cvar_t *cp = zalloc(sizeof(cvar_t));

    cvar_t *cvar = map_find(&cvar_map, (void *) val);
    if (cvar == NULL) {
        map_insert(&cvar_map, (void *) val, cp);
    } else {
        LIST_APPEND(cvar, cp);
    }
    *cp = (cvar_t){
        .var  = var,
        .next = NULL};
}

static void cvar_remove(val_t val, oprd_t var) {
    cvar_t *cp   = NULL;
    cvar_t *cvar = map_find(&cvar_map, (void *) val);

    if (cvar && cvar->var.id == var.id) {
        cp = cvar;
        map_insert(&cvar_map, (void *) val, cvar->next);
        goto done;
    }
    LIST_ITER(cvar, it) {
        if (it->next && it->next->var.id == var.id) {
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
    val_t holding = 0;
    switch (oprd.kind) {
        case OPRD_VAR:
            holding = (val_t) map_find(&holding_map, (void *) oprd.id);
            if (holding) {
                return holding;
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
    val_t holding = (val_t) map_find(&holding_map, (void *) oprd.id);
    if (holding) { // kill oprd
        cvar_remove(holding, oprd);
        map_insert(&holding_map, (void *) oprd.id, NULL);
    }
    cvar_insert(val, oprd);
    map_insert(&holding_map, (void *) oprd.id, (void *) val);
}

static void oprd_rewrite(oprd_t *oprd, val_t val) {
    cvar_t *cvar = map_find(&cvar_map, (void *) val);
    if (cvar != NULL) {
        *oprd = cvar->var;
    }
}

VISIT(IR_ASSIGN) { // tar = lhs
    val_t expr = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, expr);
    oprd_holds(node->tar, expr);
}

VISIT(IR_BINARY) {
    val_t lhs  = oprd_to_val(node->lhs);
    val_t rhs  = oprd_to_val(node->rhs);
    val_t expr = bintab_lookup(node->op, lhs, rhs);
    if (!expr) {
        expr = bintab_insert(node->op, lhs, rhs);
    }
    cvar_t *cvar = map_find(&cvar_map, (void *) expr);
    if (cvar != NULL) {
        node->lhs  = cvar->var;
        node->kind = IR_ASSIGN;
    } else {
        oprd_rewrite(&node->lhs, lhs);
        oprd_rewrite(&node->rhs, rhs);
    }
    oprd_holds(node->tar, expr);
}

VISIT(IR_DREF) {
    val_t lhs = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, lhs);
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_LOAD) {
    val_t lhs = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, lhs);
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_STORE) {
    val_t lhs = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, lhs);
}

VISIT(IR_BRANCH) {
    val_t lhs = oprd_to_val(node->lhs);
    val_t rhs = oprd_to_val(node->rhs);
    oprd_rewrite(&node->lhs, lhs);
    oprd_rewrite(&node->rhs, rhs);
}

VISIT(IR_RETURN) {
    val_t expr = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, expr);
}

VISIT(IR_CALL) {
    val_t expr = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, expr);
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_READ) {
    oprd_holds(node->tar, unrtab_insert(node->tar));
}

VISIT(IR_WRITE) {
    val_t expr = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, expr);
    oprd_holds(node->tar, expr);
}

VISIT(IR_ARG) {
    val_t expr = oprd_to_val(node->lhs);
    oprd_rewrite(&node->lhs, expr);
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_DEC);
VISIT_EMPTY(IR_PARAM);
VISIT_EMPTY(IR_LABEL);