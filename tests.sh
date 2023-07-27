
IN_FILE="./dataset/1gb.txt"
N_THREADS=1

for i in {1..10}; do LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_ff $IN_FILE ./outputs/test_ff.txt  $N_THREADS ; done #|  grep "read and count" 
