#include "cfg.h"
#include "common.h"
#include "cp.h"
#include "ir.h"
#include "map.h"
#include <stdio.h>
#include <string.h>

#define RET_TYPE cp_data_t *
#define ARG out
VISITOR_DEF(IR, cp, RET_TYPE);

static fact_t NAC   = (fact_t){.kind = FACT_NAC};
static fact_t UNDEF = (fact_t){.kind = FACT_UNDEF};

static void const_prop(IR_t *node, data_t *data) {
    ASSERT(data->magic == MAGIC, "data magic");
    VISITOR_DISPATCH(IR, cp, node, data);
}

static void data_init(cp_data_t *data) {
    data->super.magic = MAGIC;
    map_init(&data->facts);
}

static void data_fini(cp_data_t *data) {
    data->super.magic = MAGIC;
    map_fini(&data->facts);
}

static fact_t const_alloc(u32 val) {
    return (fact_t){.kind = FACT_CONST, .val = val};
}

static fact_t fact_merge(const fact_t lhs, const fact_t rhs) {
    if (lhs.kind == FACT_NAC || rhs.kind == FACT_NAC) {
        return NAC;
    }
    if (lhs.kind == FACT_UNDEF) {
        return rhs;
    }
    if (rhs.kind == FACT_UNDEF) {
        return lhs;
    }
    return (lhs.val == rhs.val) ? lhs : NAC;
}

static fact_t fact_compute(op_kind_t op, const fact_t lhs, const fact_t rhs) {
    if (lhs.kind == FACT_UNDEF || rhs.kind == FACT_UNDEF) return UNDEF;
    if (lhs.kind == FACT_NAC && rhs.kind == FACT_NAC) return NAC;
    if (lhs.kind == FACT_CONST && rhs.kind == FACT_CONST) {
        switch (op) {
            case OP_ADD: return const_alloc(lhs.val + rhs.val);
            case OP_SUB: return const_alloc(lhs.val - rhs.val);
            case OP_MUL: return const_alloc(lhs.val * rhs.val);
            case OP_DIV: return const_alloc(lhs.val / rhs.val);
            default: UNREACHABLE;
        }
    }
#define IS_CONST(FACT, VAL) (((FACT).kind == FACT_CONST) && ((FACT).val == (VAL)))
    switch (op) {
        case OP_ADD: {
            if (IS_CONST(lhs, 0)) return rhs;
            if (IS_CONST(rhs, 0)) return lhs;
            return NAC;
        }
        case OP_SUB: {
            if (IS_CONST(rhs, 0)) return lhs;
            return NAC;
        }
        case OP_MUL: {
            if (IS_CONST(lhs, 0) || IS_CONST(rhs, 0)) return const_alloc(0);
            return NAC;
        }
        case OP_DIV: {
            if (IS_CONST(rhs, 1)) return lhs;
            return NAC;
        }
        default: UNREACHABLE;
    }
    UNREACHABLE;
}

// This is really ugly, but hopefully it works...
static bool merge(cp_data_t *into, const cp_data_t *rhs) {
    ASSERT(rhs->super.magic == MAGIC, "rhs magic");
    bool changed = false;

    mapent_t *entries_into = zalloc(into->facts.size * sizeof(mapent_t));
    mapent_t *entries_rhs  = zalloc(rhs->facts.size * sizeof(mapent_t));

    u32 len_into = map_to_array(&into->facts, entries_into);
    u32 len_rhs  = map_to_array(&rhs->facts, entries_rhs);

    mapent_t *entries = zalloc((len_into + len_rhs) * sizeof(mapent_t));

    u32 i = 0, j = 0, len = 0;
    while (i < len_into && j < len_rhs) {
        mapent_t ent_into = entries_into[i];
        mapent_t ent_rhs  = entries_rhs[j];
        if ((uptr) ent_into.key < (uptr) ent_rhs.key) {
            entries[len++] = ent_into;
            i++;
        } else if ((uptr) ent_into.key > (uptr) ent_rhs.key) {
            entries[len++] = ent_rhs;
            j++;
        } else {
            entries[len++] = (mapent_t){
                .key = ent_into.key,
                .val = (void *) fact_merge((fact_t){.rep = (uptr) ent_into.val}, (fact_t){.rep = (uptr) ent_rhs.val}).rep};
            i++;
            j++;
        }
    }
    while (i < len_into) {
        entries[len++] = entries_into[i++];
    }
    while (j < len_rhs) {
        entries[len++] = entries_rhs[j++];
    }
    map_from_array(&into->facts, len, entries);
    zfree(entries_rhs);
    zfree(entries_into);
    zfree(entries);
    return changed;
}

static void *data_at(void *ptr, u32 index) {
    return &(((cp_data_t *) ptr)[index]);
}

static bool data_eq(data_t *lhs, data_t *rhs) {
    return map_eq(
        &((cp_data_t *) lhs)->facts,
        &((cp_data_t *) rhs)->facts);
}

static void data_cpy(data_t *dst, data_t *src) {
    map_fini(&((cp_data_t *) dst)->facts);
    map_cpy(&((cp_data_t *) dst)->facts, &((cp_data_t *) src)->facts);
}

static void data_mov(data_t *dst, data_t *src) {
    swap(((cp_data_t *) dst)->facts, ((cp_data_t *) src)->facts);
}

void do_cp(cfg_t *cfg) {
    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = const_prop,
        .transfer_block = NULL,
        .DSIZE          = sizeof(cp_data_t),
        .DMAGIC         = MAGIC,
        .data_init      = (void *) data_init,
        .data_fini      = (void *) data_fini,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_cpy       = data_cpy,
        .data_mov       = data_mov,
        .data_in        = zalloc(sizeof(cp_data_t) * cfg->nnode),
        .data_out       = zalloc(sizeof(cp_data_t) * cfg->nnode)};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
        df.data_init(df.data_at(df.data_out, blk->id));
    }
    dataflow_init(&df);
    df.solve(cfg);

    LIST_ITER(cfg->blocks, blk) {
        cp_data_t *pd = (cp_data_t *) df.data_at(df.data_in, blk->id);
        LIST_ITER(blk->instrs.head, ir) {
            df.transfer_instr(ir, (data_t *) pd);
            cp_rewrite(ir, pd);
        }
        // ir_remove_mark(&blk->instrs);
        data_fini(df.data_at(df.data_in, blk->id));
        data_fini(df.data_at(df.data_out, blk->id));
    }
    zfree(df.data_in);
    zfree(df.data_out);
}

void fact_insert(cp_data_t *out, oprd_t oprd, fact_t fact) {
    map_insert(&out->facts, (void *) oprd.id, (void *) fact.rep);
}

fact_t fact_get(cp_data_t *out, oprd_t oprd) {
    switch (oprd.kind) {
        case OPRD_VAR: return (fact_t){.rep = (uptr) map_find(&out->facts, (void *) oprd.id)};
        case OPRD_LIT: return const_alloc(oprd.val);
    }
    UNREACHABLE;
}

VISIT(IR_ASSIGN) {
    fact_insert(out, node->tar, fact_get(out, node->lhs));
}

VISIT(IR_BINARY) {
    fact_insert(out, node->tar,
                fact_compute(
                    node->op,
                    fact_get(out, node->lhs),
                    fact_get(out, node->rhs)));
}

VISIT(IR_DREF) {
    fact_insert(out, node->tar, NAC);
}

VISIT(IR_LOAD) {
    fact_insert(out, node->tar, NAC);
}

VISIT(IR_CALL) {
    fact_insert(out, node->tar, NAC);
}

VISIT(IR_READ) {
    fact_insert(out, node->tar, NAC);
}

VISIT(IR_WRITE) {
    fact_insert(out, node->tar, const_alloc(0));
}

VISIT(IR_DEC) {
    fact_insert(out, node->tar, NAC);
}

VISIT(IR_PARAM) { // intra-procedural, safe
    fact_insert(out, node->lhs, NAC);
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);
VISIT_EMPTY(IR_BRANCH);
VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_ARG);