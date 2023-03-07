#pragma once
#include "common.h"
#include <stdbool.h>

typedef struct node_t     node_t;
typedef struct map_t      map_t;
typedef struct map_iter_t map_iter_t;
typedef struct mapent_t   mapent_t;
typedef struct map_t      set_t;
typedef struct map_iter_t set_iter_t;

struct map_t {
    i32 (*cmp)(const void *lhs, const void *rhs);
    node_t *root;
    u32     size;
};

struct mapent_t {
    const void *key;
    void       *val;
};

struct map_iter_t {
    node_t *node;
};

struct node_t {
    node_t     *next; // for iterator
    node_t     *lhs, *rhs;
    i32         h;
    const void *key;
    void       *val;
};

void map_init(map_t *map, i32 (*cmp)(const void *lhs, const void *rhs));

void map_fini(map_t *map);

void *map_find(map_t *map, const void *key);

void map_insert(map_t *map, const void *key, void *val);

void map_iter_init(map_t *map, map_iter_t *iter);

mapent_t map_iter_next(map_iter_t *iter);

void set_init(set_t *set, i32 (*cmp)(const void *lhs, const void *rhs));

void set_fini(set_t *set);

bool set_contains(set_t *set, const void *elem);

void set_remove(set_t *set, const void *elem);

void set_merge(set_t *into, const set_t *rhs);

void set_iter_init(set_t *set, set_iter_t *iter);

void *set_iter_next(set_iter_t *iter);