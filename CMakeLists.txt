project(fHuffman)

cmake_minimum_required(VERSION 3.22)

set(DEBUG 0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wno-unused -O3")

add_compile_definitions(DEBUG=${DEBUG})

include_directories(./includes)
include_directories(./fastflow)
include_directories(./argparse/include)


add_executable(huff_seq huff_seq.cpp)
add_executable(huff_ff huff_ff.cpp)







