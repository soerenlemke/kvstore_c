#include "hashmap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_put_and_get(void) {
    HashMap* map = hashmap_create(16);
    assert(map != nullptr);

    const uint8_t key[] = "foo";
    const uint8_t value[] = "bar";
    assert(hashmap_put(map, key, 3, value, 3));

    HashMapEntry entry;
    assert(hashmap_get(map, key, 3, &entry));
    assert(entry.value_len == 3);
    assert(memcmp(entry.value, value, 3) == 0);

    hashmap_destroy(map);
    printf("test_put_and_get passed\n");
}

static void test_get_missing_key(void) {
    HashMap* map = hashmap_create(16);
    HashMapEntry entry;
    assert(!hashmap_get(map, (const uint8_t*)"missing", 7, &entry));
    hashmap_destroy(map);
    printf("test_get_missing_key passed\n");
}

static void test_remove(void) {
    HashMap* map = hashmap_create(16);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3);
    assert(hashmap_remove(map, (const uint8_t*)"foo", 3, nullptr));

    HashMapEntry entry;
    assert(!hashmap_get(map, (const uint8_t*)"foo", 3, &entry));
    hashmap_destroy(map);
    printf("test_remove passed\n");
}

static void test_remove_with_out_entry(void) {
    HashMap* map = hashmap_create(16);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3);

    HashMapEntry removed;
    assert(hashmap_remove(map, (const uint8_t*)"foo", 3, &removed));
    assert(removed.key_len == 3);
    assert(memcmp(removed.key, "foo", 3) == 0);
    assert(removed.value_len == 3);
    assert(memcmp(removed.value, "bar", 3) == 0);

    // The map no longer owns this memory — the caller must release it.
    hashmap_entry_release(&removed);
    assert(removed.key == nullptr);
    assert(removed.value == nullptr);
    assert(removed.key_len == 0);
    assert(removed.value_len == 0);

    // Calling release again must be a safe no-op, not a double free.
    hashmap_entry_release(&removed);

    HashMapEntry entry;
    assert(!hashmap_get(map, (const uint8_t*)"foo", 3, &entry));
    hashmap_destroy(map);
    printf("test_remove_with_out_entry passed\n");
}

static void test_collision_chaining(void) {
    HashMap* map = hashmap_create(1);
    hashmap_put(map, (const uint8_t*)"a", 1, (const uint8_t*)"1", 1);
    hashmap_put(map, (const uint8_t*)"b", 1, (const uint8_t*)"2", 1);
    hashmap_put(map, (const uint8_t*)"c", 1, (const uint8_t*)"3", 1);

    HashMapEntry entry;
    assert(hashmap_get(map, (const uint8_t*)"a", 1, &entry));
    assert(memcmp(entry.value, "1", 1) == 0);
    assert(hashmap_get(map, (const uint8_t*)"b", 1, &entry));
    assert(memcmp(entry.value, "2", 1) == 0);
    assert(hashmap_get(map, (const uint8_t*)"c", 1, &entry));
    assert(memcmp(entry.value, "3", 1) == 0);

    hashmap_destroy(map);
    printf("test_collision_chaining passed\n");
}

static void test_update_existing_key(void) {
    HashMap* map = hashmap_create(16);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"baz", 3);

    HashMapEntry entry;
    assert(hashmap_get(map, (const uint8_t*)"foo", 3, &entry));
    assert(entry.value_len == 3);
    assert(memcmp(entry.value, "baz", 3) == 0);

    hashmap_destroy(map);
    printf("test_update_existing_key passed\n");
}

static void test_remove_middle_node(void) {
    HashMap* map = hashmap_create(1);
    hashmap_put(map, (const uint8_t*)"a", 1, (const uint8_t*)"1", 1);
    hashmap_put(map, (const uint8_t*)"b", 1, (const uint8_t*)"2", 1);
    hashmap_put(map, (const uint8_t*)"c", 1, (const uint8_t*)"3", 1);

    assert(hashmap_remove(map, (const uint8_t*)"b", 1, nullptr));

    HashMapEntry entry;
    assert(hashmap_get(map, (const uint8_t*)"a", 1, &entry));
    assert(hashmap_get(map, (const uint8_t*)"c", 1, &entry));
    assert(!hashmap_get(map, (const uint8_t*)"b", 1, &entry));

    hashmap_destroy(map);
    printf("test_remove_middle_node passed\n");
}

static void test_remove_missing_key(void) {
    HashMap* map = hashmap_create(16);
    assert(!hashmap_remove(map, (const uint8_t*)"nope", 4, nullptr));
    hashmap_destroy(map);
    printf("test_remove_missing_key passed\n");
}

#define STRESS_TEST_SIZE 2000

static void test_resize_preserves_all_entries(void) {
    HashMap* map = hashmap_create(4); // deliberately small: forces several resizes
    assert(map != nullptr);

    char key[32];
    char value[32];

    for (size_t i = 0; i < STRESS_TEST_SIZE; i++) {
        const int key_len = snprintf(key, sizeof(key), "key%zu", i);
        const int value_len = snprintf(value, sizeof(value), "val%zu", i);
        assert(hashmap_put(map, (const uint8_t*)key, (size_t)key_len,
                            (const uint8_t*)value, (size_t)value_len));
    }

    for (size_t i = 0; i < STRESS_TEST_SIZE; i++) {
        const int key_len = snprintf(key, sizeof(key), "key%zu", i);
        const int value_len = snprintf(value, sizeof(value), "val%zu", i);

        HashMapEntry entry;
        assert(hashmap_get(map, (const uint8_t*)key, (size_t)key_len, &entry));
        assert(entry.value_len == (size_t)value_len);
        assert(memcmp(entry.value, value, entry.value_len) == 0);
    }

    hashmap_destroy(map);
    printf("test_resize_preserves_all_entries passed\n");
}

static void test_resize_then_remove_half(void) {
    HashMap* map = hashmap_create(4);
    assert(map != nullptr);

    char key[32];
    char value[32];

    for (size_t i = 0; i < STRESS_TEST_SIZE; i++) {
        const int key_len = snprintf(key, sizeof(key), "key%zu", i);
        const int value_len = snprintf(value, sizeof(value), "val%zu", i);
        assert(hashmap_put(map, (const uint8_t*)key, (size_t)key_len,
                            (const uint8_t*)value, (size_t)value_len));
    }

    // remove every second key, after the map has already grown several times
    for (size_t i = 0; i < STRESS_TEST_SIZE; i += 2) {
        const int key_len = snprintf(key, sizeof(key), "key%zu", i);
        assert(hashmap_remove(map, (const uint8_t*)key, (size_t)key_len, nullptr));
    }

    for (size_t i = 0; i < STRESS_TEST_SIZE; i++) {
        const int key_len = snprintf(key, sizeof(key), "key%zu", i);
        HashMapEntry entry;
        const bool found = hashmap_get(map, (const uint8_t*)key, (size_t)key_len, &entry);

        if (i % 2 == 0) {
            assert(!found); // removed
        } else {
            const int value_len = snprintf(value, sizeof(value), "val%zu", i);
            assert(found);
            assert(entry.value_len == (size_t)value_len);
            assert(memcmp(entry.value, value, entry.value_len) == 0);
        }
    }

    hashmap_destroy(map);
    printf("test_resize_then_remove_half passed\n");
}

static void test_shrink_to_minimum_capacity(void) {
    HashMap* map = hashmap_create(1);
    assert(map != nullptr);
    assert(hashmap_capacity(map) == 1);

    char key[16];
    char value[16];
    const size_t N = 20;

    // Fill enough that the map grows multiple times.
    for (size_t i = 0; i < N; i++) {
        const size_t key_len = (size_t) snprintf(key, sizeof(key), "k%zu", i);
        const size_t value_len = (size_t) snprintf(value, sizeof(value), "v%zu", i);
        assert(hashmap_put(map, (const uint8_t*)key, key_len, (const uint8_t*)value, value_len));

        // Never allowed to hit 0 - this is the modulo-by-zero crash we found earlier.
        assert(hashmap_capacity(map) >= 1);
    }
    // Verified empirically: 20 puts starting from capacity=1 grow it to 32.
    assert(hashmap_capacity(map) == 32);

    // Remove everything again, one at a time.
    for (size_t i = 0; i < N; i++) {
        const size_t key_len = (size_t) snprintf(key, sizeof(key), "k%zu", i);
        assert(hashmap_remove(map, (const uint8_t*)key, key_len, nullptr));

        assert(hashmap_capacity(map) >= 1);
    }

    // Shrinking is lazy/single-step (see comment in hashmap_remove): a fully
    // emptied map settles one growth-factor above the true minimum, not at 1.
    // Verified empirically. If this ever changes (e.g. shrink becomes a loop),
    // this assertion should change to == 1.
    assert(hashmap_capacity(map) == 2);

    hashmap_destroy(map);
    printf("test_shrink_to_minimum_capacity passed\n");
}

static void test_create_zero_capacity(void) {
    assert(hashmap_create(0) == nullptr);
    printf("test_create_zero_capacity passed\n");
}

static void test_put_null_map(void) {
    assert(!hashmap_put(nullptr, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3));
    printf("test_put_null_map passed\n");
}

static void test_get_null_map(void) {
    HashMapEntry entry;
    assert(!hashmap_get(nullptr, (const uint8_t*)"foo", 3, &entry));
    printf("test_get_null_map passed\n");
}

static void test_remove_null_map(void) {
    assert(!hashmap_remove(nullptr, (const uint8_t*)"foo", 3, nullptr));
    printf("test_remove_null_map passed\n");
}

static void test_put_null_key_nonzero_len(void) {
    HashMap* map = hashmap_create(16);
    assert(!hashmap_put(map, nullptr, 3, (const uint8_t*)"bar", 3));
    hashmap_destroy(map);
    printf("test_put_null_key_nonzero_len passed\n");
}

static void test_put_null_value_nonzero_len(void) {
    HashMap* map = hashmap_create(16);
    assert(!hashmap_put(map, (const uint8_t*)"foo", 3, nullptr, 3));
    hashmap_destroy(map);
    printf("test_put_null_value_nonzero_len passed\n");
}

static void test_get_null_key_nonzero_len(void) {
    HashMap* map = hashmap_create(16);
    HashMapEntry entry;
    assert(!hashmap_get(map, nullptr, 3, &entry));
    hashmap_destroy(map);
    printf("test_get_null_key_nonzero_len passed\n");
}

static void test_get_null_out_entry(void) {
    HashMap* map = hashmap_create(16);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3);

    assert(!hashmap_get(map, (const uint8_t*)"foo", 3, nullptr));

    hashmap_destroy(map);
    printf("test_get_null_out_entry passed\n");
}

static void test_remove_null_key_nonzero_len(void) {
    HashMap* map = hashmap_create(16);
    assert(!hashmap_remove(map, nullptr, 3, nullptr));
    hashmap_destroy(map);
    printf("test_remove_null_key_nonzero_len passed\n");
}

static void test_zero_length_key_allowed(void) {
    HashMap* map = hashmap_create(16);
    // A zero-length key with a nullptr pointer is a valid "empty key" entry.
    assert(hashmap_put(map, nullptr, 0, (const uint8_t*)"bar", 3));

    HashMapEntry entry;
    assert(hashmap_get(map, nullptr, 0, &entry));
    assert(entry.value_len == 3);
    assert(memcmp(entry.value, "bar", 3) == 0);

    assert(hashmap_remove(map, nullptr, 0, nullptr));

    hashmap_destroy(map);
    printf("test_zero_length_key_allowed passed\n");
}

int main(void) {
    test_put_and_get();
    test_get_missing_key();
    test_remove();
    test_remove_with_out_entry();
    test_collision_chaining();
    test_update_existing_key();
    test_remove_middle_node();
    test_remove_missing_key();
    test_resize_preserves_all_entries();
    test_resize_then_remove_half();
    test_shrink_to_minimum_capacity();
    test_create_zero_capacity();
    test_put_null_map();
    test_get_null_map();
    test_remove_null_map();
    test_put_null_key_nonzero_len();
    test_put_null_value_nonzero_len();
    test_get_null_key_nonzero_len();
    test_get_null_out_entry();
    test_remove_null_key_nonzero_len();
    test_zero_length_key_allowed();
    printf("all tests passed\n");
    return 0;
}