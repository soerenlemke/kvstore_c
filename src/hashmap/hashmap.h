#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct HashMap HashMap;

/**
 * @brief Creates a new hashmap with the given initial capacity.
 *
 * @param initial_capacity Number of initial buckets. Must be greater than 0.
 * @return Pointer to the new HashMap, or nullptr on allocation failure
 *         or if initial_capacity is 0. The caller is responsible for
 *         freeing it later via hashmap_destroy().
 */
HashMap* hashmap_create(size_t initial_capacity);

/**
 * @brief Frees a hashmap and all entries it contains.
 *
 * @param map The hashmap to free. May be nullptr (no-op).
 */
void hashmap_destroy(HashMap* map);

/**
 * @brief Inserts a new entry or updates an existing one.
 *
 * @param map       Target hashmap.
 * @param key       Pointer to the key bytes. Copied internally.
 * @param key_len   Length of the key in bytes.
 * @param value     Pointer to the value bytes. Copied internally.
 * @param value_len Length of the value in bytes.
 * @return true on success, false on allocation failure.
 */
bool hashmap_put(HashMap* map, const uint8_t* key, size_t key_len,
                  const uint8_t* value, size_t value_len);

/**
 * @brief Looks up the value stored for the given key.
 *
 * @param map            The hashmap to search.
 * @param key            Pointer to the key bytes.
 * @param key_len        Length of the key in bytes.
 * @param out_value      Set to point at the stored value bytes if found.
 *                        Owned by the hashmap; do not free.
 * @param out_value_len  Set to the length of the stored value if found.
 * @return true if the key was found, false otherwise.
 */
bool hashmap_get(const HashMap* map, const uint8_t* key, size_t key_len,
                  uint8_t** out_value, size_t* out_value_len);

/**
 * @brief Removes the entry for the given key, if present.
 *
 * @param map     Target hashmap.
 * @param key     Pointer to the key bytes.
 * @param key_len Length of the key in bytes.
 * @return true if an entry was found and removed, false otherwise.
 */
bool hashmap_remove(HashMap* map, const uint8_t* key, size_t key_len);