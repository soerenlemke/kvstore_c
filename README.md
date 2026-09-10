# kvstore_c

A small, dependency-free key-value store written in C23, built around a hash
map with separate chaining. Designed to be usable both as a standalone
application and as a library embedded in other C projects.

Keys and values are treated as raw byte buffers (`uint8_t*` + length), so the
store works with arbitrary binary data, not just null-terminated strings.

> This is a personal learning project — see [SECURITY.md](SECURITY.md) for
> the (informal) support policy.

## Features

- Hash map with separate chaining (FNV-1a hashing, 64-bit)
- Automatic growth (load factor 0.75, 2x growth) and shrink on removal
- Opaque `HashMap` struct — internals are hidden behind the public API
- Byte-oriented API (`uint8_t*` + `size_t` length) for both keys and values
- No external dependencies — pure C23 and the standard library
- Unit tests covering basic operations, collisions, and resize/shrink behavior

## Status

The hashmap itself (`src/hashmap`) is functional and tested. The standalone
CLI application (`src/main.c`) is currently a placeholder and not yet wired
up to the hashmap — using the store as a library is the supported path today.

## Requirements

- CMake >= 4.2
- A C23-capable compiler (e.g. recent GCC or Clang)

## Building

```bash
git clone https://github.com/soerenlemke/kvstore_c.git
cd kvstore_c
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

The `Debug` build type enables AddressSanitizer automatically. Use `Release`
for a build without sanitizer instrumentation.

## Running the tests

```bash
cd build
ctest --output-on-failure
```

Or run the test binary directly:

```bash
./hashmap_tests
```

## Using it as a library

Link against the `hashmap` CMake target and include `hashmap.h`:

```c
#include "hashmap.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    HashMap* map = hashmap_create(16);
    if (map == nullptr) {
        return 1;
    }

    const uint8_t key[]   = "name";
    const uint8_t value[] = "kvstore_c";
    hashmap_put(map, key, strlen((const char*)key), value, strlen((const char*)value));

    uint8_t* out_value;
    size_t out_len;
    if (hashmap_get(map, key, strlen((const char*)key), &out_value, &out_len)) {
        printf("%.*s\n", (int)out_len, out_value);
    }

    hashmap_remove(map, key, strlen((const char*)key));
    hashmap_destroy(map);
    return 0;
}
```

### API overview

| Function            | Description                                              |
| -------------------- | --------------------------------------------------------- |
| `hashmap_create`     | Allocates a new map with a given initial bucket capacity |
| `hashmap_destroy`    | Frees a map and all entries it contains                  |
| `hashmap_put`        | Inserts or updates a key-value entry                      |
| `hashmap_get`        | Looks up the value for a key                              |
| `hashmap_remove`     | Removes an entry by key                                   |
| `hashmap_capacity`   | Returns the current bucket capacity (mainly for testing)  |

Values returned by `hashmap_get` are owned by the map — do not `free()` them
directly; they remain valid until the entry is overwritten or removed, or the
map is destroyed.

Full parameter documentation is in [`src/hashmap/hashmap.h`](src/hashmap/hashmap.h).

## Design notes

- The hash of each key is cached on its node, so resizing re-hashes buckets
  without recomputing hashes.
- If a resize allocation fails during `hashmap_put` or `hashmap_remove`, the
  operation itself still succeeds — the map simply stays at its current
  capacity and degrades in performance rather than returning an error.
- Separate chaining was kept over open addressing after an external code
  review, since the project targets a byte-oriented KV store rather than a
  generic container library. Open addressing may be explored later as a
  performance experiment.

## Further reading

I write about the design decisions behind this project on my blog:
[Building a Generic Hashmap in C](http://localhost:4321/blog/blog/building-a-generic-hashmap-in-c/).

## License

MIT — see [LICENSE](LICENSE).

## Reporting issues / security

See [SECURITY.md](SECURITY.md) for how to report bugs or vulnerabilities.
