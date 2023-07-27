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

    utimer tt("total time");

    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];

    
    // ********** READ AND COUNT **********
    
    char * mapped_file;
    long long dataSize = mmap_file(original_filename, &mapped_file); 

    int * counts = new int[CODE_POINTS]{};

    {
    utimer utimer("read and count");
    for (long long i = 0; i < dataSize; i++) {
        counts[(int)mapped_file[i]]++;
    }
    }


    // ********** BUILD THE HUFFMAN TREE **********

    std::map<char, long long unsigned> counts_map;
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

    for (long long i = 0; i < dataSize; i++) {
        for (char bit : encoder[(int)mapped_file[i]]) {
            encoded_contents.push_back(bit == '1');
        }
    }

    }
    
    // ********** COMPRESSING AND WRITING **********


    long encoded_compressed_size = 0;
    char * mapped_output_file;
    unsigned short padding = 0;
    unsigned long write_index = 0;

    {
    utimer utimer("compressing and writing");

    // make the size of the encoded file a multiple of 8
    while (encoded_contents.size() % 8 != 0) {
        encoded_contents.push_back(false);
        padding++;
    }

    encoded_compressed_size = encoded_contents.size() / 8;
    mmap_file_write(encoded_filename, encoded_compressed_size, mapped_output_file);
    
    // write the encoded file building one byte each 8 bit
    for (long unsigned i = 0; i < encoded_contents.size(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = byte << 1;
            if (encoded_contents[i + j]) {
                byte = byte | 1;
            }
        }
        mapped_output_file[write_index++] = byte;
    }

    mmap_file_sync(mapped_output_file, encoded_compressed_size);

    }

    return 0;
}
