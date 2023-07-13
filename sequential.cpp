// sequential implementation of the huffman algorithm

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

#define NUM_CHARS 128
 

// read the input file and count the number of occurrences of each character



void build_encoder(std::string filename, std::vector<std::string>& code) {

    std::vector<int> count(NUM_CHARS, 0);
    
    std::ifstream infile(filename);
    
    // count the number of occurrences of each character
    char ch;
    while(infile.get(ch)){        
        count[(int)ch]++; 
    }
    infile.close();
    
    // build an array of tuples (count, character)
    std::vector<std::pair<int, char>> v(NUM_CHARS);
    
    for (int i = 0; i < NUM_CHARS; i++) {
        v[i] = std::make_pair(count[i], (char) i) ;
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
    code[(char)v[0].second] = "0";
    for (int i = 1; i < NUM_CHARS; i++) {
        code[(char)v[i].second] = std::string(i, '1') + '0';
    }
    
}

void build_decoder(std::vector<std::string>& decoder, std::vector<std::string>& encoder) {
    
    std::vector<std::pair<int, char>> v(NUM_CHARS);

    // for each character, store the length of its code minus 1 (to be used as index in the decoder)
    for (int i = 0; i < NUM_CHARS; i++) {
        v[i] = std::make_pair(encoder[i].size() - 1, (char) i) ;
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

void encode(std::string ascii_file, std::string encoded_file, std::vector<std::string>& encoder) {
    
    std::ifstream infile(ascii_file);  

    std::ofstream outfile(encoded_file);

    char ch;
    while(infile.get(ch)){
        outfile << encoder[(int)ch];
    }
}

void decode(std::string encoded_file, std::string reconstructed_file, std::vector<std::string>& decoder) {
    
    std::ifstream infile(encoded_file);  

    std::ofstream outfile(reconstructed_file);

    // read the file separating by the ones
    std::string code;
    while(std::getline(infile, code, '0')){
        outfile << decoder[code.size()];
    }
}


int main(){
    
    std::vector<std::string> encoder(NUM_CHARS, "0");
    std::vector<std::string> decoder(NUM_CHARS, "0");

    build_encoder("dataset/test.txt", encoder);

    encode("dataset/test.txt", "outputs/test_encoded.txt", encoder);

    build_decoder(decoder, encoder);

    decode("outputs/test_encoded.txt", "outputs/test_decoded.txt", decoder);
    

    return 0;
}
