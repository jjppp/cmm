#pragma once
#include "common.h"

#define MAX_TAB_SIZE 8192

typedef struct hashent_t {
    char *str;
    void *ptr;
} hashent_t;

typedef struct {
    hashent_t bucket[MAX_TAB_SIZE];
    u32       size;
} hashtab_t;

hashent_t *hash_lookup(hashtab_t *hashtab, const char *str);
