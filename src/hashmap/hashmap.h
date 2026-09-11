#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct HashMap HashMap;

typedef struct HashMapEntry {
    uint8_t* key;
    size_t key_len;
    uint8_t* value;
    size_t value_len;
} HashMapEntry;

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
 * @param map       Target hashmap. Must not be nullptr.
 * @param key       Pointer to the key bytes. Copied internally. May be
 *                  nullptr only if key_len is 0.
 * @param key_len   Length of the key in bytes.
 * @param value     Pointer to the value bytes. Copied internally. May be
 *                  nullptr only if value_len is 0.
 * @param value_len Length of the value in bytes.
 * @return true on success, false on allocation failure or invalid
 *         arguments (map is nullptr, or key/value is nullptr while its
 *         corresponding length is greater than 0).
 */
bool hashmap_put(HashMap* map, const uint8_t* key, size_t key_len,
                  const uint8_t* value, size_t value_len);

/**
 * @brief Looks up the value stored for the given key.
 *
 * @param map       The hashmap to search. Must not be nullptr.
 * @param key       Pointer to the key bytes. May be nullptr only if
 *                  key_len is 0.
 * @param key_len   Length of the key in bytes.
 * @param out_entry Set to point at the stored key/value bytes if the key
 *                  is found. Must not be nullptr. The pointers in
 *                  out_entry refer directly to memory owned by the
 *                  hashmap (not a copy); do not free them, and treat
 *                  them as valid only until the entry is overwritten,
 *                  removed, or the map is destroyed.
 * @return true if the key was found, false otherwise or if arguments are
 *         invalid (map, out_entry, or key while key_len is greater than
 *         0, is nullptr).
 */
bool hashmap_get(const HashMap* map, const uint8_t* key, size_t key_len, HashMapEntry* out_entry);

/**
 * @brief Removes the entry for the given key, if present.
 *
 * @param map         Target hashmap. Must not be nullptr.
 * @param key         Pointer to the key bytes. May be nullptr only if
 *                     key_len is 0.
 * @param key_len     Length of the key in bytes.
 * @param out_removed Optional. If not nullptr, set to a copy of the
 *                     removed entry's key/value bytes. Unlike
 *                     hashmap_get, this memory belongs to the caller and
 *                     must be freed (e.g. via hashmap_entry_release). If
 *                     the internal copy fails (out of memory), out_removed
 *                     is left as {0} even though the entry was still
 *                     successfully removed from the map.
 * @return true if an entry was found and removed, false otherwise or if
 *         map is nullptr, or key is nullptr while key_len is greater
 *         than 0.
 */
bool hashmap_remove(HashMap* map, const uint8_t* key, size_t key_len, HashMapEntry* out_removed);

/**
 * @brief Returns the current number of buckets in the hashmap.
 *
 * Exposed primarily for testing/introspection (e.g. verifying resize
 * behavior). Not required for normal put/get/remove usage.
 *
 * @param map The hashmap to query.
 * @return Current bucket capacity.
 */
size_t hashmap_capacity(const HashMap* map);

/**
 * @brief Releases the memory owned by an entry returned via hashmap_remove's
 *        out_removed parameter.
 *
 * @param entry The entry to release. May be nullptr (no-op). After the
 *              call, entry->key and entry->value are set to nullptr and
 *              key_len/value_len to 0, so calling this function again on
 *              the same entry is safe (a no-op) rather than a double free.
 */
void hashmap_entry_release(HashMapEntry* entry);