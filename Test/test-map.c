#include "common.h"
#include "map.h"
#include <string.h>

static void test_as_array() {
    map_t map;
    map_init(&map);

    map_insert(&map, (void *) 1, (void *) 1);
    ASSERT(map_find(&map, (void *) 1) == (void *) 1, "insert failed");
    map_insert(&map, (void *) 1, (void *) 9);
    ASSERT(map_find(&map, (void *) 1) == (void *) 9, "insert failed");
    map_insert(&map, (void *) 4, (void *) 1);
    ASSERT(map_find(&map, (void *) 4) == (void *) 1, "insert failed");
    map_insert(&map, (void *) 5, (void *) 9);
    ASSERT(map_find(&map, (void *) 5) == (void *) 9, "insert failed");
    map_insert(&map, (void *) 1, (void *) 8);
    ASSERT(map_find(&map, (void *) 1) == (void *) 8, "insert failed");
    map_insert(&map, (void *) 4, (void *) 1);
    ASSERT(map_find(&map, (void *) 4) == (void *) 1, "insert failed");

    map_iter(&map, it) {
        printf("%lu", (uptr) it.val);
    }
    puts("");
    map_fini(&map);
}

static void test_set() {
    set_t set;
    set_init(&set);

    set_insert(&set, (void *) 1);
    set_iter(&set, it) printf("%lu ", (uptr) it.val);
    puts("");
    set_insert(&set, (void *) 4);
    set_iter(&set, it) printf("%lu ", (uptr) it.val);
    puts("");
    set_insert(&set, (void *) 2);

    ASSERT(set_contains(&set, (void *) 2), "missing 2");
    set_iter(&set, it) printf("%lu ", (uptr) it.val);
    puts("");
    set_fini(&set);
}

static void test_merge() {
    set_t lhs, rhs;
    set_init(&lhs);
    set_init(&rhs);

    set_insert(&lhs, (void *) 1);
    set_insert(&lhs, (void *) 2);
    set_insert(&lhs, (void *) 3);

    set_insert(&rhs, (void *) 2);
    set_insert(&rhs, (void *) 4);
    set_insert(&rhs, (void *) 3);

    set_merge(&lhs, &rhs);

    set_iter(&lhs, it) {
        printf("%lu\n", (uptr) it.val);
    }
    ASSERT(lhs.size == 4, "lhs size err");
}

static void print_seq(node_t *node) {
    if (!node) return;
    print_seq(node->lhs);
    printf("%lu ", (uptr) node->key);
    print_seq(node->rhs);
}

static void test_seq() {
    set_t set;
    set_init(&set);

    set_insert(&set, (void *) 5);
    print_seq(set.root);
    puts("");
    set_insert(&set, (void *) 4);
    print_seq(set.root);
    puts("");
    set_insert(&set, (void *) 2);
    print_seq(set.root);
    puts("");
    set_insert(&set, (void *) 3);
    print_seq(set.root);
    puts("");
    set_remove(&set, (void *) 5);
    print_seq(set.root);
    puts("");
    set_remove(&set, (void *) 4);
    print_seq(set.root);
    puts("");
}

static void test_to_array() {
    map_t map;
    map_init(&map);
    map_insert(&map, (void *) 1, (void *) 1);
    map_insert(&map, (void *) 1, (void *) 9);
    map_insert(&map, (void *) 4, (void *) 1);
    map_insert(&map, (void *) 5, (void *) 9);
    map_insert(&map, (void *) 1, (void *) 8);
    map_insert(&map, (void *) 4, (void *) 1);
    map_insert(&map, (void *) 6, (void *) 22);
    map_insert(&map, (void *) 2, (void *) 3);
    map_insert(&map, (void *) 9, (void *) 7);
    map_insert(&map, (void *) 34, (void *) 21);

    mapent_t *entries = zalloc(map.size * sizeof(mapent_t));
    map_to_array(&map, entries);
    map_t new_map;
    map_from_array(&new_map, map.size, entries);
    ASSERT(map_eq(&map, &new_map), "map not EQUAL");
    zfree(entries);
}

int main() {
    test_as_array();
    test_set();
    test_merge();
    test_seq();
    test_to_array();
    puts("PASSED");
}