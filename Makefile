
IDIR = -I ./includes
FFIDIR = -I ./fastflow/
FLAGS= -Wall -Wpedantic -Wno-unused -O3 -I /usr/local/include -std=c++17 

all :  huff_seq huff_ff

huff_seq : huff_seq.cpp
	g++ -o huff_seq huff_seq.cpp $(FLAGS) $(IDIR)

huff_ff : huff_ff.cpp
	g++ -o huff_ff huff_ff.cpp $(FLAGS) $(FFIDIR) $(IDIR) -pthread