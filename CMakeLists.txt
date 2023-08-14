project(fHuffman)

cmake_minimum_required(VERSION 3.22)

set(DEBUG 0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wno-unused -O3")

add_compile_definitions(DEBUG=${DEBUG})

include_directories(./includes)
include_directories(./fastflow)


add_executable(huff_seq ./src/huff_seq.cpp)
add_executable(huff_ff ./src/huff_ff.cpp)
add_executable(huff_thr ./src/huff_thr.cpp)







