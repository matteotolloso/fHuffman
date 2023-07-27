
IN_FILE="./dataset/1gb.txt"
N_THREADS=32

for i in {1..10}; do LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_thr $IN_FILE ./outputs/test_thr.txt  $N_THREADS ; done #|  grep "read and count" 
