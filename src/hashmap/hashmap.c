#include "hashmap.h"

#include <stdlib.h>
#include <string.h>

// TODO: no resizing yet — load factor grows unbounded, chains get long over time
// TODO: consider open addressing instead of separate chaining for cache locality

typedef struct HashMapNode {
    uint8_t* key;
    size_t key_len;
    uint8_t* value;
    size_t value_len;
    struct HashMapNode* next;
} HashMapNode;

static void hashmap_node_free(HashMapNode* node) {
    free(node->key);
    free(node->value);
    free(node);
}

typedef struct HashMap {
    HashMapNode** buckets;
    size_t capacity;
    size_t count;
} HashMap;

/**
 * @brief Computes an FNV-1a hash over the given bytes.
 *
 * @param key     Bytes to hash.
 * @param key_len Number of bytes to hash.
 * @return 64-bit hash value. Not cryptographically secure.
 */
static size_t hash_bytes(const uint8_t* key, const size_t key_len) {
    size_t hash = 1469598103934665603ULL;
    for (size_t i = 0; i < key_len; i++) {
        hash ^= key[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

// helper function for comparing keys
static bool keys_equal(const uint8_t* a, const size_t a_len, const uint8_t* b, const size_t b_len) {
    if (a_len != b_len) {
        return false;
    }
    return memcmp(a, b, a_len) == 0;
}

/**
 * @brief Allocates a copy of the given bytes.
 *
 * @param data Bytes to copy.
 * @param len  Number of bytes to copy.
 * @return Pointer to the newly allocated copy, or nullptr on allocation
 *         failure. Caller owns the returned memory and must free() it.
 */
static uint8_t* duplicate_bytes(const uint8_t* data, size_t len) {
    uint8_t* copy = malloc(len);
    if (copy == nullptr) {
        return nullptr;
    }
    memcpy(copy, data, len);
    return copy;
}

HashMap* hashmap_create(const size_t initial_capacity) {
    if (initial_capacity == 0) {
        return nullptr;
    }

    const auto map = (HashMap*) malloc(sizeof(HashMap));
    if (map == nullptr) {
        return nullptr;
    }

    map->buckets = (HashMapNode**) calloc(initial_capacity, sizeof(HashMapNode*));
    if (map->buckets == nullptr) {
        free(map);
        return nullptr;
    }

    map->capacity = initial_capacity;
    map->count = 0;

    return map;
}

void hashmap_destroy(HashMap* map) {
    if (map == nullptr) {
        return;
    }

    for (size_t i = 0; i < map->capacity; i++) {
        auto node = map->buckets[i];
        while (node != nullptr) {
            const auto next = node->next;
            hashmap_node_free(node);
            node = next;
        }
    }

    free(map->buckets);
    free(map);
}

bool hashmap_put(HashMap* map, const uint8_t* key, size_t key_len, const uint8_t* value, size_t value_len) {
    const size_t index = hash_bytes(key, key_len) % map->capacity;

    // check if node already exists
    HashMapNode* node = map->buckets[index];
    while (node != nullptr) {
        if (keys_equal(node->key, node->key_len, key, key_len)) {
            uint8_t* new_value_copy = duplicate_bytes(value, value_len);
            if (new_value_copy == nullptr) {
                return false;
            }
            free(node->value);
            node->value = new_value_copy;
            node->value_len = value_len;
            return true;
        }
        node = node->next;
    }

    // create new node if none is found
    HashMapNode* new_node = malloc(sizeof(HashMapNode));
    if (new_node == nullptr) {
        return false;
    }

    new_node->key = duplicate_bytes(key, key_len);
    if (new_node->key == nullptr) {
        free(new_node);
        return false;
    }
    new_node->key_len = key_len;

    new_node->value = duplicate_bytes(value, value_len);
    if (new_node->value == nullptr) {
        free(new_node->key);
        free(new_node);
        return false;
    }
    new_node->value_len = value_len;

    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    map->count++;

    return true;
}

bool hashmap_get(const HashMap* map, const uint8_t* key, size_t key_len, uint8_t** out_value, size_t* out_value_len) {
    const size_t index = hash_bytes(key, key_len) % map->capacity;

    HashMapNode* node = map->buckets[index];
    while (node != nullptr) {
        if (keys_equal(node->key, node->key_len, key, key_len)) {
            *out_value = node->value;
            *out_value_len = node->value_len;
            return true;
        }
        node = node->next;
    }
    return false;
}