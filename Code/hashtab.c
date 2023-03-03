#include "hashtab.h"
#include <string.h>

static u32 str_hash(const char *s) {
    u32 hash = 0;
    for (const char *p = s; *p; p++) {
        hash = (hash * 17 + *p) % MAX_SYM;
    }
    return hash;
}

hashent_t *hash_lookup(hashtab_t *hashtab, const char *str) {
    hashent_t *bucket = hashtab->bucket;
    u32        hash   = str_hash(str);
    while (bucket[hash].ptr && strcmp(bucket[hash].str, str)) {
        hash = (hash + 1) % MAX_SYM;
    }
    return &bucket[hash];
}