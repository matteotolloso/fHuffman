project(fHuffman)

cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wno-unused -O3")

include_directories(../include)
include_directories(../fastflow)

add_executable(huff_seq huff_seq.cpp)
add_executable(huff_ff huff_ff.cpp)







