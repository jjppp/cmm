#include "map.h"

static node_t *node_alloc(const void *key, void *val) {
    node_t *node = zalloc(sizeof(node_t));
    node->key    = key;
    node->val    = val;
    return node;
}

void map_init(map_t *map, i32 (*cmp)(const void *lhs, const void *rhs)) {
    ASSERT(map != NULL, "map NULL");
    ASSERT(cmp != NULL, "cmp NULL");
    map->cmp  = cmp;
    map->size = 0;
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

static node_t *rotate_R(node_t *const node) {
    node_t *lhs = node->lhs;
    node->lhs   = lhs->rhs;
    lhs->rhs    = node;
    return lhs;
}

static node_t *rotate_L(node_t *const node) {
    node_t *rhs = node->rhs;
    node->rhs   = rhs->lhs;
    rhs->lhs    = node;
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
            rotate_R(node->rhs);
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
    map->size++;
    map->root = rebalance(map->root);
}

void set_init(set_t *set, i32 (*cmp)(const void *lhs, const void *rhs)) {
    map_init(set, cmp);
}

void set_fini(set_t *set) {
    TODO("set_fini");
}

bool set_contains(set_t *set, const void *elem) {
    return map_find(set, elem) != NULL;
}

void set_remove(set_t *set, const void *elem) {
    map_insert(set, elem, NULL);
}

void set_merge(set_t *into, const set_t *rhs) {
    TODO("set_merge");
}

void map_iter_init(map_t *map, map_iter_t *iter) {
    iter->node = map->root;
}

mapent_t map_iter_next(map_iter_t *iter) {
    node_t *front = iter->node;
    if (!front) {
        return (mapent_t){0};
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
    return (mapent_t){
        .key = front->key,
        .val = front->val};
}

void set_iter_init(set_t *set, set_iter_t *iter) {
    map_iter_init(set, iter);
}

void *set_iter_next(set_iter_t *iter) {
    return map_iter_next(iter).val;
}