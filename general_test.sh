#!/bin/bash

IN_FILE="./dataset/2_25.txt"
N_THREADS=32
JMALLOC_PATH="/home/m.tolloso/libs/lib/libjemalloc.so"


cd build && make && cd .. &&

echo -e "\nTest with native threads\n" &&
time  LD_PRELOAD=$JMALLOC_PATH ./build/huff_thr $IN_FILE ./outputs/test_thr.txt  $N_THREADS &&
echo -e "-----------------------------------\n" && 

echo -e "\nTest with fastflow\n" &&
time  LD_PRELOAD=$JMALLOC_PATH ./build/huff_ff $IN_FILE ./outputs/test_ff.txt  $N_THREADS &&
echo -e "-----------------------------------\n" &&

echo -e "\nTest sequential\n" &&
time ./build/huff_seq $IN_FILE ./outputs/test_seq.txt &&
echo -e "-----------------------------------\n" &&

diff ./outputs/test_ff.txt ./outputs/test_seq.txt &&
diff ./outputs/test_thr.txt ./outputs/test_seq.txt 