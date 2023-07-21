// sequential implementation of the huffman algorithm

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include "utimer.cpp"
#include <huffman_codes.cpp>

#define CODE_POINTS 128

std::string read_file(std::string filename){
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/*
    equals
    @param original_filename: the name of the original file
    @param decoded_filename: the name of the reconstructed file
    @return: true if the original file and the reconstructed file are the same, false otherwise
    @description: check if the original file and the reconstructed file are the same
*/
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


    // std::string original_filename = argv[1];
    // std::string encoded_filename = argv[2];
    // std::string decoded_filename = argv[3];

    std::string original_filename = "./dataset/test.txt";
    std::string encoded_filename = "./outputs/test.txt";
    // std::string decoded_filename = argv[3];

    std::string contents;
    std::map<char, int> counts;

    std::vector<std::string> encoder(CODE_POINTS);
    MinHeapNode* decoder;


    // ********** READ THE FILE **********
    {
        utimer utimer("reading file");

        contents = read_file(original_filename);

    }
    

    // ********** COUNT THE CHARACTERS **********
    {
        utimer utimer("counting characters");

        for (char ch : contents) {
            counts[(int)ch]++; 
        }
    }



    // ********** BUILD THE HUFFMAN TREE **********
    {
        utimer utimer("huffman encoder and decoder creation");        
        
        huffman_codes(counts, encoder, decoder);
    }

    // ********** ENCODE THE FILE **********
    
    std::ofstream encoded(encoded_filename);
    std::vector<bool> encoded_contents;
    {
        utimer utimer("encoding file");

        for (char ch : contents) {
            for (char bit : encoder[(int)ch]) {
                encoded_contents.push_back(bit == '1');
            }
        }
    }
    
    // ********** WRITE THE ENCODED FILE **********

    {
        utimer utimer("writing encoded file");

        int encoded_size = encoded_contents.size();

        // make the size of the encoded file a multiple of 8
        while (encoded_size % 8 != 0) {
            encoded_contents.push_back(false);
            encoded_size++;
        }

        // write the encoded file building one byte each 8 bit
        for (int i = 0; i < encoded_contents.size(); i += 8) {
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
