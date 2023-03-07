#include "map.h"

static node_t *node_alloc(const void *key, void *val) {
    node_t *node = zalloc(sizeof(node_t));
    node->key    = key;
    node->val    = val;
    node->h      = 1;
    return node;
}

void map_init(map_t *map, i32 (*cmp)(const void *lhs, const void *rhs)) {
    ASSERT(map != NULL, "map NULL");
    ASSERT(cmp != NULL, "cmp NULL");
    map->cmp  = cmp;
    map->root = NULL;
}

void map_fini(map_t *map) {
    // TODO: mem leak
    TODO("map_fini");
}

static node_t *map_find_helper(map_t *map, node_t *node, const void *key) {
    if (!node) {
        return NULL;
    }
    i32 cmp_val = map->cmp(key, node->key);
    if (!cmp_val) {
        return node;
    } else if (cmp_val > 0) {
        return map_find_helper(map, node->rhs, key);
    } else {
        return map_find_helper(map, node->lhs, key);
    }
}

void *map_find(map_t *map, const void *key) {
    node_t *node = map_find_helper(map, map->root, key);
    if (!node) {
        return NULL;
    }
    return node->val;
}

static i32 get_height(node_t *node) {
    return node ? node->h : 0;
}

static i32 get_balance(node_t *node) {
    ASSERT(node, "get_balance NULL");
    return get_height(node->lhs) - get_height(node->rhs);
}

static void calc_height(node_t *node) {
    node->h = max(get_height(node->lhs), get_height(node->rhs)) + 1;
}

static void node_validate(node_t *node) {
    if (!node) return;
    node_validate(node->lhs);
    node_validate(node->rhs);
    u32 h = node->h;
    calc_height(node);
    ASSERT(h == node->h, "height");
}

static node_t *rotate_R(node_t *const node) {
    node_t *lhs = node->lhs;
    node->lhs   = lhs->rhs;
    lhs->rhs    = node;
    calc_height(node);
    calc_height(lhs);
    return lhs;
}

static node_t *rotate_L(node_t *const node) {
    node_t *rhs = node->rhs;
    node->rhs   = rhs->lhs;
    rhs->lhs    = node;
    calc_height(node);
    calc_height(rhs);
    return rhs;
}

static node_t *rebalance(node_t *node) {
    calc_height(node);
    i32 b = get_balance(node);
    if (b == 2) {
        if (get_balance(node->lhs) < 0) {
            node->lhs = rotate_L(node->lhs);
        }
        return rotate_R(node);
    } else if (b == -2) {
        if (get_balance(node->rhs) == 1) {
            node->rhs = rotate_R(node->rhs);
        }
        return rotate_L(node);
    }
    return node;
}

static node_t *map_insert_helper(map_t *map, node_t *node, const void *key) {
    i32 cmp_val = map->cmp(key, node->key);
    if (!cmp_val) {
        return node;
    }
    node_t *new = NULL;
    if (cmp_val > 0) {
        if (!node->rhs) {
            new       = node_alloc(key, NULL);
            node->rhs = new;
        } else {
            new = map_insert_helper(map, node->rhs, key);
        }
    } else {
        if (!node->lhs) {
            new       = node_alloc(key, NULL);
            node->lhs = new;
        } else {
            new = map_insert_helper(map, node->lhs, key);
        }
    }
    if (node->lhs) node->lhs = rebalance(node->lhs);
    if (node->rhs) node->rhs = rebalance(node->rhs);
    calc_height(node);
    return new;
}

void map_insert(map_t *map, const void *key, void *val) {
    ASSERT(key != NULL, "insert key NULL");
    if (!map->root) {
        map->root = node_alloc(key, val);
    } else {
        node_t *node = map_insert_helper(map, map->root, key);
        ASSERT(node != NULL, "insert NULL");
        node->val = val;
    }
    map->root = rebalance(map->root);
    node_validate(map->root);
}

void map_remove(map_t *map, const void *key) {
    map_insert(map, key, NULL);
}

void set_init(set_t *set, i32 (*cmp)(const void *lhs, const void *rhs)) {
    map_init(set, cmp);
}

void set_fini(set_t *set) {
    TODO("set_fini");
}

void set_insert(set_t *set, void *elem) {
    map_insert(set, elem, elem);
}

bool set_contains(set_t *set, const void *elem) {
    return map_find(set, elem) != NULL;
}

bool set_eq(set_t *lhs, set_t *rhs) {
    if (lhs == rhs) return true;
    set_iter(lhs, it) {
        if (it.val && !set_contains(rhs, it.val)) {
            return false;
        }
    }
    set_iter(rhs, it) {
        if (it.val && !set_contains(lhs, it.val)) {
            return false;
        }
    }
    return true;
}

bool map_eq(map_t *lhs, map_t *rhs) {
    if (lhs == rhs) return true;
    map_iter(lhs, it) {
        if (it.val && !map_find(rhs, it.key)) {
            return false;
        }
    }
    map_iter(rhs, it) {
        if (it.val && !map_find(rhs, it.key)) {
            return false;
        }
    }
    return true;
}

void set_cpy(set_t *dst, set_t *src) {
    // TODO: mem leak
    set_init(dst, dst->cmp);
    set_iter(src, it) {
        set_insert(dst, it.val);
    }
}

void map_cpy(map_t *dst, map_t *src) {
    // TODO: mem leak
    map_init(dst, dst->cmp);
    map_iter(src, it) {
        map_insert(dst, it.key, it.val);
    }
}

void set_remove(set_t *set, const void *elem) {
    map_remove(set, elem);
}

void set_merge(set_t *into, const set_t *rhs) {
    set_iter(rhs, it) {
        set_insert(into, it.val);
    }
}

void map_merge(map_t *into, const map_t *rhs) {
    map_iter(rhs, it) {
        map_insert(into, it.key, it.val);
    }
}

map_iter_t map_iter_init(const map_t *map) {
    map_iter_t iter = (map_iter_t){
        .node = map->root,
        .done = false};
    if (!iter.node) { // empty map
        iter.done = true;
    } else {
        map->root->next = NULL;
        if (!iter.val) { // skip initial NULLs
            map_iter_next(&iter);
        }
    }
    return iter;
}

void map_iter_next(map_iter_t *iter) {
    node_t *front = iter->node;
    if (!front) {
        iter->key  = NULL;
        iter->val  = NULL;
        iter->done = true;
        return;
    }
    iter->node = front->next;
    if (front->lhs) {
        front->lhs->next = iter->node;
        iter->node       = front->lhs;
    }
    if (front->rhs) {
        front->rhs->next = iter->node;
        iter->node       = front->rhs;
    }
    if (front->val) {
        iter->key = front->key;
        iter->val = front->val;
    } else {
        map_iter_next(iter);
    }
}

set_iter_t set_iter_init(const set_t *set) {
    return map_iter_init(set);
}

void set_iter_next(set_iter_t *iter) {
    map_iter_next(iter);
}