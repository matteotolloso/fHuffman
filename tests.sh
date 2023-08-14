
IN_FILE="./dataset/8gb.txt"
N_THREADS=32

 # for i in {1..10}; do LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_seq $IN_FILE ./outputs/test_seq.txt ; done


for i in {25..33} ; do
    LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_seq ./dataset/2_$i.txt ./outputs/test_ff.txt;
done