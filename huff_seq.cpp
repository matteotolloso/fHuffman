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


bool equals(std::string original_filename, std::string decoded_filename){
    utimer timer("queals check");

    std::ifstream original(original_filename);
    std::ifstream reconstructed(decoded_filename);
    
    std::string original_line;
    std::string reconstructed_line;

    while(std::getline(original, original_line) && std::getline(reconstructed, reconstructed_line)) {
        if (original_line != reconstructed_line) {
            std::cout << "The original file and the reconstructed file are different" << std::endl;
            return false;
        }
    }

    return true;

}


int main(int argc, char** argv){


    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];
    // std::string decoded_filename = argv[3];

    // std::string original_filename = "./dataset/test.txt";
    // std::string encoded_filename = "./outputs/test.txt";
    // std::string decoded_filename = argv[3];
    
    std::vector<std::string> encoder(CODE_POINTS);
    MinHeapNode* decoder;


    // ********** READ THE FILE **********
    char * mapped_file;
    int * counts = new int[CODE_POINTS]{};
    long unsigned dataSize = 0;
    {
    utimer utimer("mapping file");
    mmap_file(original_filename, &mapped_file);
    dataSize = strlen(mapped_file);

    }
    
    // ********** COUNT THE CHARACTERS **********
    {
    utimer utimer("counting characters");
    for (long unsigned i = 0; i < dataSize; i++) {
        counts[(int)mapped_file[i]]++;
    }
    }


    // ********** BUILD THE HUFFMAN TREE **********

    std::map<char, int> counts_map;
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


    

    // check if the original file and the reconstructed file are the same
    // if (equals(original_filename, decoded_filename))
    //     std::cout << "The original file and the reconstructed file are the same" << std::endl;
    // else
    //     std::cout << "The original file and the reconstructed file are different" << std::endl;

    return 0;
}
