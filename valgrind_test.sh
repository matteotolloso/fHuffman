
IN_FILE="./dataset/2_25.txt"
N_THREADS=32


cd build && cmake .. && make && cd .. &&


echo -e "\nTest with native threads\n" &&
valgrind -s --leak-check=full --track-origins=yes ./build/huff_thr $IN_FILE ./outputs/test_thr.txt  $N_THREADS &&
echo -e "-----------------------------------\n" && 

echo -e "\nTest with fastflow\n" &&
valgrind -s --leak-check=full --track-origins=yes ./build/huff_ff $IN_FILE ./outputs/test_ff.txt  $N_THREADS &&
echo -e "-----------------------------------\n" &&

echo -e "\nTest sequential\n" &&
valgrind -s --leak-check=full --track-origins=yes ./build/huff_seq $IN_FILE ./outputs/test_seq.txt  &&
echo -e "-----------------------------------\n" 
