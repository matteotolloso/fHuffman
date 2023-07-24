// sequential implementation of the huffman algorithm

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include "utimer.cpp"
#include <huffman_codes.cpp>
#include <utils.cpp>

#define CODE_POINTS 128



int main(int argc, char** argv){


    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];


    // ********** READ THE FILE **********
    char * mapped_file;
    mmap_file(original_filename, &mapped_file);
    long unsigned dataSize = strlen(mapped_file);

    
    // ********** COUNT THE CHARACTERS **********

    int * counts = new int[CODE_POINTS]{};

    {
    utimer utimer("counting characters");
    for (long unsigned i = 0; i < dataSize; i++) {
        counts[(int)mapped_file[i]]++;
    }
    }


    // ********** BUILD THE HUFFMAN TREE **********

    std::map<char, int> counts_map;
    std::vector<std::string> encoder(CODE_POINTS);
    MinHeapNode* decoder;
    {
    utimer utimer("huffman encoder and decoder creation"); 

    for (int i = 0; i < CODE_POINTS; i++) {
        counts_map[(char)i] = counts[i];
    }       
    
    huffman_codes(counts_map, encoder, decoder);
    }


    // ********** ENCODE THE FILE **********
    
    std::vector<bool> encoded_contents;
    
    {
    utimer utimer("encoding file");

    for (long unsigned i = 0; i < dataSize; i++) {
        for (char bit : encoder[(int)mapped_file[i]]) {
            encoded_contents.push_back(bit == '1');
        }
    }

    }
    
    // ********** WRITE THE ENCODED FILE **********

    std::ofstream encoded(encoded_filename);
    unsigned short padding = 0;

    {
    utimer utimer("writing encoded file");

    // make the size of the encoded file a multiple of 8
    while (encoded_contents.size() % 8 != 0) {
        encoded_contents.push_back(false);
        padding++;
    }

    // write the encoded file building one byte each 8 bit
    for (long unsigned i = 0; i < encoded_contents.size(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = byte << 1;
            if (encoded_contents[i + j]) {
                byte = byte | 1;
            }
        }
        encoded << byte;
    }

    encoded.close();
    }

    return 0;
}
