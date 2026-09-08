#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct HashMap HashMap;

HashMap* hashmap_create(size_t initial_capacity);
void hashmap_destroy(HashMap* map);

bool hashmap_put(HashMap* map, const uint8_t* key, size_t key_len, const uint8_t* value, size_t value_len);
bool hashmap_get(const HashMap* map, const uint8_t* key, size_t key_len, uint8_t** out_value, size_t* out_value_len);
bool hashmap_remove(HashMap* map, const uint8_t* key, size_t key_len);