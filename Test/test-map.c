#include "common.h"
#include "map.h"
#include <string.h>

static void test_insert() {
    static char *str_arr[] = {
        "ABC", "123", "__009"};
    map_t map;
    map_init(&map, (void *) strcmp);

    for (i32 i = 0; i < ARR_LEN(str_arr); i++) {
        map_insert(&map, str_arr[i], str_arr[i]);
    }
    ASSERT(map_find(&map, "ABC") == str_arr[0], "ABC should be present");
    map_insert(&map, "ABC", test_insert);
    ASSERT(map_find(&map, "ABC") == test_insert, "ABC should be modified");
    ASSERT(map_find(&map, "114514") == NULL, "114514 should not be found");
}

static i32 cmp(const void *lhs, const void *rhs) {
    return ((uptr) lhs) - ((uptr) rhs);
}

static void test_as_array() {
    map_t map;
    map_init(&map, cmp);

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

    map_iter_t iter;
    map_iter_init(&map, &iter);
    for (mapent_t it = map_iter_next(&iter);
         it.val != NULL;
         it = map_iter_next(&iter)) {
        printf("%lu", (uptr) it.val);
    }
    puts("");
}

int main() {
    test_insert();
    test_as_array();
    puts("PASSED");
}