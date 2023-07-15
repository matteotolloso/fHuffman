// sequential implementation of the huffman algorithm

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include "utimer.cpp"

#define NUM_CHARS 128

std::string read_file(std::string filename){
    std::ifstream infile(filename);
    std::string contents((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    return contents;
}

/*
    build_encoder
    @param filename: the name of the file to be encoded
    @param code: the vector of strings that will contain the code for each character
    @return: void
    @description: build the encoder, i.e. the vector of strings that will contain the code for each character
*/
void build_encoder(std::string contents, std::vector<int>& code) {
    
    utimer timer("build encoder");
    
    std::vector<int> count(NUM_CHARS, 0);
    
    // count the number of occurrences of each character
    for (char ch : contents) {
        count[(int)ch]++; 
    }

    // build an array of tuples (count, character)
    std::vector<std::pair<int, char>> v(NUM_CHARS);
    
    for (int i = 0; i < NUM_CHARS; i++) {
        v[i] = std::make_pair(count[i], i) ;
    }

    // sort the array in descending order
    std::sort(
        v.begin(), 
        v.end(), 
        [](const std::pair<int, char>& a, const std::pair<int, char>& b) -> bool {
            return a.first > b.first;
        }
    );

    // build the code
    for (int i = 0; i < NUM_CHARS; i++) {
        code[v[i].second] = i;
    }
    
}

/*
    build_decoder
    @param decoder: the vector that given the length of the code returns the character
    @param encoder: the vector of strings that contains the code for each character
    @return: void
    @description: build the decoder
*/
void build_decoder(std::vector<std::string>& decoder, std::vector<int> encoder) {

    utimer timer("build decoder");
    
    std::vector<std::pair<int, char>> v(NUM_CHARS);

    // for each character, store the length of its code minus 1 (to be used as index in the decoder)
    for (int i = 0; i < NUM_CHARS; i++) {
        v[i] = std::make_pair(encoder[i], (char) i) ;
    }

    // sort the array in descending order
    std::sort(
        v.begin(), 
        v.end(), 
        [](const std::pair<int, char>& a, const std::pair<int, char>& b) -> bool {
            return a.first < b.first;
        }
    );

    // build the decoder
    for (int i = 0; i < NUM_CHARS; i++) {
        decoder[i] = v[i].second;
    }

}

/*
    encode
    @param ascii_file: the name of the file to be encoded
    @param encoded_file: the name of the file that will contain the encoded text
    @param encoder: the vector of strings that contains the code for each character
    @return: void
    @description: encode the text
*/
void encode(std::string contents, std::string encoded_file, std::vector<int> encoder) {

    utimer timer("encode file");

    std::ofstream outfile(encoded_file, std::ios::binary);

    std::vector<bool> bits;

    // read the file and store the bits in a vector

    for (char ch : contents) {
        int code = encoder[(int)ch];
        while (code > 0) {
            bits.push_back(1);
            code--;
        }
        bits.push_back(0);
    }

    // TODO find a better way to write the bits in the file
    // write the bits in the file
    for (bool bit : bits) {
        outfile << bit;
    }
}

/*
    decode
    @param encoded_file: the name of the file that contains the encoded text
    @param reconstructed_file: the name of the file that will contain the decoded text
    @param decoder: the vector that given the length of the code returns the character
    @return: void
    @description: decode the text
*/
void decode(std::string encoded_file, std::string reconstructed_file, std::vector<std::string>& decoder) {

    utimer timer("decode file");
    
    std::ifstream infile(encoded_file);  

    std::ofstream outfile(reconstructed_file);

    // read the file separating by the ones
    std::string code;
    while(std::getline(infile, code, '0')){
        outfile << decoder[code.size()];
    }
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

    if (argc != 4) {
        std::cout << "Usage: ./sequential <original_filename> <encoded_filename> <decoded_filename>" << std::endl;
        return 1;
    }

    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];
    std::string decoded_filename = argv[3];
    
    std::vector<int> encoder(NUM_CHARS);
    std::vector<std::string> decoder(NUM_CHARS, "0");
    
    std::string contents = read_file(original_filename);

    build_encoder(contents, encoder);

    encode(contents, encoded_filename, encoder);

    build_decoder(decoder, encoder);

    decode(encoded_filename, decoded_filename, decoder);

    // check if the original file and the reconstructed file are the same
    if (equals(original_filename, decoded_filename))
        std::cout << "The original file and the reconstructed file are the same" << std::endl;
    else
        std::cout << "The original file and the reconstructed file are different" << std::endl;

    return 0;
}
