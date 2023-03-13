#pragma once
#include "common.h"
#include <stdbool.h>

typedef struct node_t     node_t;
typedef struct map_t      map_t;
typedef struct map_iter_t map_iter_t;
typedef struct mapent_t   mapent_t;
typedef struct map_t      set_t;
typedef struct map_iter_t set_iter_t;

#define map_iter(MAP, IT)                      \
    for (map_iter_t IT = map_iter_init((MAP)); \
         !(IT).done;                           \
         map_iter_next(&(IT)))

#define set_iter(MAP, IT)                      \
    for (set_iter_t IT = set_iter_init((MAP)); \
         !(IT).done;                           \
         set_iter_next(&(IT)))

struct map_t {
    node_t *root;
    u32     size;
};

struct mapent_t {
    const void *key;
    void       *val;
};

struct map_iter_t {
    const void *key;
    void       *val;
    node_t     *node;
    bool        done;
};

struct node_t {
    node_t     *next; // for iterator
    node_t     *lhs, *rhs;
    i32         h;
    const void *key;
    void       *val;
};

void map_init(map_t *map);

void map_fini(map_t *map);

void *map_find(const map_t *map, const void *key);

void map_insert(map_t *map, const void *key, void *val);

void map_remove(map_t *map, const void *key);

void map_merge(map_t *into, const map_t *rhs);

void map_intersect(map_t *into, const map_t *rhs);

bool map_eq(map_t *lhs, map_t *rhs);

void map_cpy(map_t *dst, map_t *src);

map_iter_t map_iter_init(const map_t *map);

void map_iter_next(map_iter_t *iter);

void set_init(set_t *set);

void set_fini(set_t *set);

void set_insert(set_t *set, void *elem);

bool set_contains(const set_t *set, const void *elem);

void set_remove(set_t *set, const void *elem);

bool set_merge(set_t *into, const set_t *rhs);

void set_intersect(set_t *into, const set_t *rhs);

set_iter_t set_iter_init(const set_t *set);

bool set_eq(set_t *lhs, set_t *rhs);

void set_cpy(set_t *dst, set_t *src);

void set_iter_next(set_iter_t *iter);

u32 map_to_array(const map_t *map, mapent_t *entries);

void map_from_array(map_t *map, u32 len, mapent_t *entries);