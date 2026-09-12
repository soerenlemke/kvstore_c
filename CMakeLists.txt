cmake_minimum_required(VERSION 4.2)
project(kvstore_c C)

set(CMAKE_C_STANDARD 23)

if (MSVC)
    add_compile_options(/std:clatest)
elseif (NOT MINGW)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

add_library(hashmap
        src/hashmap/hashmap.c
)
target_include_directories(hashmap PUBLIC src/hashmap)

add_executable(kvstore_c
        src/main.c
)
target_link_libraries(kvstore_c hashmap)

# Test executable
enable_testing()
add_executable(hashmap_tests
        tests/hashmap_tests.c
)
target_link_libraries(hashmap_tests hashmap)
add_test(NAME hashmap_tests COMMAND hashmap_tests)