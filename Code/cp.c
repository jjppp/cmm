#include "cfg.h"
#include "common.h"
#include "cp.h"
#include "ir.h"
#include "live.h"
#include "map.h"
#include <stdio.h>
#include <string.h>

#define RET_TYPE cp_data_t *
#define ARG out
VISITOR_DEF(IR, cp, RET_TYPE);

static fact_t NAC   = (fact_t){.kind = FACT_NAC};
static fact_t UNDEF = (fact_t){.kind = FACT_UNDEF};

static dataflow live_df;

static void const_prop(IR_t *node, void *data) {
    VISITOR_DISPATCH(IR, cp, node, data);
}

static void data_init(cp_data_t *data) {
    map_init(&data->facts);
}

static void data_fini(cp_data_t *data) {
    map_fini(&data->facts);
}

static fact_t const_alloc(i64 val) {
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
            case OP_ADD: return const_alloc((i64) lhs.val + (i64) rhs.val);
            case OP_SUB: return const_alloc((i64) lhs.val - (i64) rhs.val);
            case OP_MUL: return const_alloc((i64) lhs.val * (i64) rhs.val);
            case OP_DIV: {
                if (rhs.val == 0) return UNDEF;
                return const_alloc((i64) lhs.val / (i64) rhs.val);
            }
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
            if (IS_CONST(rhs, 0)) return UNDEF;
            return NAC;
        }
        default: UNREACHABLE;
    }
    UNREACHABLE;
}

// This is really ugly, but hopefully it works...
static bool merge(cp_data_t *into, const cp_data_t *rhs) {
    bool changed = false;

    static mapent_t entries_into[32768];
    static mapent_t entries_rhs[32768];
    static mapent_t entries[65536];

    u32 len_into = map_to_array(&into->facts, entries_into);
    u32 len_rhs  = map_to_array(&rhs->facts, entries_rhs);

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
            changed = true;
        } else {
            fact_t fact = fact_merge((fact_t){.rep = (uptr) ent_into.val}, (fact_t){.rep = (uptr) ent_rhs.val});
            if (fact.rep != (uptr) ent_into.key) {
                changed = true;
            }
            entries[len++] = (mapent_t){
                .key = ent_into.key,
                .val = (void *) fact.rep};
            i++;
            j++;
        }
    }
    while (i < len_into) {
        entries[len++] = entries_into[i++];
    }
    while (j < len_rhs) {
        entries[len++] = entries_rhs[j++];
        changed        = true;
    }
    map_fini(&into->facts);
    map_from_array(&into->facts, len, entries);
    ASSERT(i <= sizeof(entries_into), "entries_into overflow");
    ASSERT(j <= sizeof(entries_rhs), "entries_rhs overflow");
    ASSERT(len <= sizeof(entries), "entries overflow");
    return changed;
}

static void *data_at(void *ptr, u32 index) {
    return &(((cp_data_t *) ptr)[index]);
}

static bool data_eq(void *lhs, void *rhs) {
    return map_eq(
        &((cp_data_t *) lhs)->facts,
        &((cp_data_t *) rhs)->facts);
}

static void data_cpy(void *dst, void *src) {
    map_fini(&((cp_data_t *) dst)->facts);
    map_cpy(&((cp_data_t *) dst)->facts, &((cp_data_t *) src)->facts);
}

static void data_mov(void *dst, void *src) {
    swap(((cp_data_t *) dst)->facts, ((cp_data_t *) src)->facts);
}

static void transfer_block(block_t *blk, void *data_in) {
    static mapent_t entries[65536];

    LIST_ITER(blk->instrs.head, ir) {
        const_prop(ir, data_in);
    }
    live_data_t *pd = live_df.data_at(live_df.data_in, blk->id);

    map_t *facts = &((cp_data_t *) data_in)->facts;
    u32    size  = map_to_array(facts, entries);
    for (u32 i = 0; i < size; i++) {
        const void *oprd = entries[i].key;
        if (!set_contains(&pd->used, oprd)) { // remove dead facts
            map_remove(facts, oprd);
        }
    }
}

dataflow do_cp(void *data_in, void *data_out, cfg_t *cfg) {
    live_data_t *live_data_in  = zalloc(sizeof(live_data_t) * cfg->nnode);
    live_data_t *live_data_out = zalloc(sizeof(live_data_t) * cfg->nnode);

    live_df = do_live(live_data_in, live_data_out, cfg);

    dataflow df = (dataflow){
        .dir            = DF_FORWARD,
        .merge          = (void *) merge,
        .transfer_instr = const_prop,
        .transfer_block = transfer_block,
        .DSIZE          = sizeof(cp_data_t),
        .data_init      = (void *) data_init,
        .data_fini      = (void *) data_fini,
        .data_at        = data_at,
        .data_eq        = data_eq,
        .data_cpy       = data_cpy,
        .data_mov       = data_mov,
        .data_in        = data_in,
        .data_out       = data_out};
    LIST_ITER(cfg->blocks, blk) {
        df.data_init(df.data_at(df.data_in, blk->id));
    }
    dataflow_init(&df);
    df.solve(cfg);

    LIST_ITER(cfg->blocks, blk) {
        live_df.data_fini(live_df.data_at(live_df.data_in, blk->id));
        live_df.data_fini(live_df.data_at(live_df.data_out, blk->id));
    }
    zfree(live_data_in);
    zfree(live_data_out);
    return df;
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
    fact_insert(out, node->tar, NAC);
}

VISIT_UNDEF(IR_NULL);

VISIT_EMPTY(IR_GOTO);
VISIT_EMPTY(IR_LABEL);
VISIT_EMPTY(IR_BRANCH);
VISIT_EMPTY(IR_RETURN);
VISIT_EMPTY(IR_STORE);
VISIT_EMPTY(IR_ARG);