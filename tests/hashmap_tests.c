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

    uint8_t* out_value;
    size_t out_len;
    assert(hashmap_get(map, key, 3, &out_value, &out_len));
    assert(out_len == 3);
    assert(memcmp(out_value, value, 3) == 0);

    hashmap_destroy(map);
    printf("test_put_and_get passed\n");
}

static void test_get_missing_key(void) {
    HashMap* map = hashmap_create(16);
    uint8_t* out_value;
    size_t out_len;
    assert(!hashmap_get(map, (const uint8_t*)"missing", 7, &out_value, &out_len));
    hashmap_destroy(map);
    printf("test_get_missing_key passed\n");
}

static void test_remove(void) {
    HashMap* map = hashmap_create(16);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3);
    assert(hashmap_remove(map, (const uint8_t*)"foo", 3));

    uint8_t* out_value;
    size_t out_len;
    assert(!hashmap_get(map, (const uint8_t*)"foo", 3, &out_value, &out_len));
    hashmap_destroy(map);
    printf("test_remove passed\n");
}
static void test_collision_chaining(void) {
    HashMap* map = hashmap_create(1); // erzwingt Kollisionen
    hashmap_put(map, (const uint8_t*)"a", 1, (const uint8_t*)"1", 1);
    hashmap_put(map, (const uint8_t*)"b", 1, (const uint8_t*)"2", 1);
    hashmap_put(map, (const uint8_t*)"c", 1, (const uint8_t*)"3", 1);

    uint8_t* out_value;
    size_t out_len;
    assert(hashmap_get(map, (const uint8_t*)"a", 1, &out_value, &out_len));
    assert(hashmap_get(map, (const uint8_t*)"b", 1, &out_value, &out_len));
    assert(hashmap_get(map, (const uint8_t*)"c", 1, &out_value, &out_len));

    hashmap_destroy(map);
    printf("test_collision_chaining passed\n");
}

static void test_update_existing_key(void) {
    HashMap* map = hashmap_create(16);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"bar", 3);
    hashmap_put(map, (const uint8_t*)"foo", 3, (const uint8_t*)"baz", 3); // überschreiben

    uint8_t* out_value;
    size_t out_len;
    assert(hashmap_get(map, (const uint8_t*)"foo", 3, &out_value, &out_len));
    assert(out_len == 3);
    assert(memcmp(out_value, "baz", 3) == 0); // NICHT mehr "bar"

    hashmap_destroy(map);
    printf("test_update_existing_key passed\n");
}

static void test_remove_middle_node(void) {
    HashMap* map = hashmap_create(1); // erzwingt Kollisionen, also eine echte Liste
    hashmap_put(map, (const uint8_t*)"a", 1, (const uint8_t*)"1", 1);
    hashmap_put(map, (const uint8_t*)"b", 1, (const uint8_t*)"2", 1);
    hashmap_put(map, (const uint8_t*)"c", 1, (const uint8_t*)"3", 1);

    assert(hashmap_remove(map, (const uint8_t*)"b", 1)); // mittendrin löschen

    uint8_t* out_value;
    size_t out_len;
    assert(hashmap_get(map, (const uint8_t*)"a", 1, &out_value, &out_len)); // a noch da
    assert(hashmap_get(map, (const uint8_t*)"c", 1, &out_value, &out_len)); // c noch da
    assert(!hashmap_get(map, (const uint8_t*)"b", 1, &out_value, &out_len)); // b weg

    hashmap_destroy(map);
    printf("test_remove_middle_node passed\n");
}

static void test_remove_missing_key(void) {
    HashMap* map = hashmap_create(16);
    assert(!hashmap_remove(map, (const uint8_t*)"nope", 4));
    hashmap_destroy(map);
    printf("test_remove_missing_key passed\n");
}

int main(void) {
    test_put_and_get();
    test_get_missing_key();
    test_remove();
    test_collision_chaining();
    test_update_existing_key();
    test_remove_middle_node();
    test_remove_missing_key();
    printf("all tests passed\n");
    return 0;
}