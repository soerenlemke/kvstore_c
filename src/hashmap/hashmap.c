#include "hashmap.h"

#include <stdlib.h>

// TODO: think about more performant implementations

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