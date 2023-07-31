
IN_FILE="./dataset/8gb.txt"
N_THREADS=16

 # for i in {1..10}; do LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_seq $IN_FILE ./outputs/test_seq.txt ; done


for i in {1..10} ; do
    LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_ff $IN_FILE ./outputs/test_ff.txt  $N_THREADS ;
done