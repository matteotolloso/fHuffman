
IN_FILE="./dataset/8gb.txt"
N_THREADS=1

 # for i in {1..10}; do LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_seq $IN_FILE ./outputs/test_seq.txt ; done


for i in {2..32..2} ; do
    time LD_PRELOAD=/home/m.tolloso/libs/lib/libjemalloc.so ./build/huff_thr ./dataset/ ./outputs/test_thr.txt  $i ;
done