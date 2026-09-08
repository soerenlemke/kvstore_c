cmake_minimum_required(VERSION 4.2)
project(kvstore_c C)

set(CMAKE_C_STANDARD 23)

add_executable(kvstore_c src/main.c
        src/hashmap/hashmap.c
        src/hashmap/hashmap.h
)
