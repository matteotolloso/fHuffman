
IDIR = -I ./includes
FFIDIR = -I ./fastflow/
FLAGS=-std=c++20 -O3 -Wall  -Wpedantic -Wno-unused

all :  huff_seq huff_ff

huff_seq : huff_seq.cpp
	g++ -o huff_seq huff_seq.cpp $(FLAGS) $(IDIR)

huff_ff : huff_ff.cpp
	g++ -o huff_ff huff_ff.cpp $(FLAGS) $(FFIDIR) $(IDIR) -pthread