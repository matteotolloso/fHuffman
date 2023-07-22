#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <iostream>
#include <utimer.cpp>
#include <fcntl.h>
#include <map>
#include <huffman_codes.cpp>
#include <utils.cpp>
//#include <argparse/argparse.hpp>

#define CODE_POINTS 128


int main(int argc, char * argv[]) { 

    // argparse::ArgumentParser program("huff_ff");

    // program.add_argument("-i")
    //     .help("path of the file to encode")
    //     .default_value("dataset/test.txt")
    //     .required();
    
    // program.add_argument("-o")
    //     .help("path of the encoded file")
    //     .default_value("outputs/test.txt")
    //     .required();

    // program.add_argument("-n")
    //     .help("number of workers")
    //     .default_value(4)
    //     .required();
    

    // try {
    //     program.parse_args(argc, argv);    // Example: ./main --color orange
    // }
    //     catch (const std::runtime_error& err) {
    //     std::cerr << err.what() << std::endl;
    //     std::cerr << program;
    //     std::exit(1);
    // }

    // string input_file_path = program.get<string>("-i");
    // string output_file_path = program.get<string>("-o");
    // int nworkers = program.get<int>("-n");



    // std::string original_filename = "./dataset/bigdata.txt";
    // std::string encoded_filename = "./outputs/test.txt";
    // int nworkers = 2;


    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];
    int nworkers = atoi(argv[3]);
    
    
    int CHUNKSIZE = 0;


    int ** counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);

    char * mapped_file;
    mmap_file(original_filename, &mapped_file);

    long unsigned dataSize = strlen(mapped_file);
    std::cout << "data size: " << dataSize << std::endl;


    // ********** PARALLEL FOR REDUCE INITIALIZATION **********


    ff::ParallelForReduce<std::map<char, int>> pfr(nworkers, false, false);


    auto parallel_for_counts_function = [&](const long start_index, const long stop_index, int thid){
        
        // utimer utimer("map function counts, thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");

        if (counts[thid] == nullptr){
            counts[thid] = new int[CODE_POINTS]();
        }

        for (long i = start_index; i < stop_index; i++){
            counts[thid][ (int) mapped_file[i]]++;
        }
    };
    

    auto parallel_reduce_function = [&](const long start_index, const long stop_index, std::map<char, int>& reduce_counts, const int thid) {

        // utimer utimer("parallel reduce counts, thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");
        
        for (long i = start_index; i < stop_index; i++){
            reduce_counts[i] = 0;
        }

        // iterate fixing first the mapper worker (the array that produced) and then the char chunk
        // for cache efficiency
        for (int j = 0; j < nworkers; j++){  // iterate all the partial computations (mapper threads)
            if (counts[j] != nullptr){  // since the scheduling is dynamic, some threads may not have ever started
                for (long i = start_index; i < stop_index; i++){   // assigned char chunk
                    reduce_counts[i] += counts[j][i]; // only this thread is wrinting on reduce counts becouse is local
                }    
            }
        } 
    };

    std::map<char, int> global_counts;

    auto sequential_reduce_function = [](std::map<char, int>& v, const std::map<char, int>& elem) {
        
        // utimer utimer("sequential reduce counts");
        
        for (auto key_value: elem){
            v[key_value.first] += key_value.second;
        }
        
    };


    {
    utimer utimer("counting characters");

    pfr.parallel_for_idx(0, dataSize, 1 ,CHUNKSIZE, parallel_for_counts_function, nworkers);
    
    pfr.parallel_reduce_idx(
        global_counts, std::map<char, int>(),
        0, CODE_POINTS, 1, CHUNKSIZE,
        parallel_reduce_function, 
        sequential_reduce_function,
        nworkers
    );

    }


    // ********** BUILD THE HUFFMAN TREE **********

    std::vector<string> encoder(CODE_POINTS);
    MinHeapNode* decoder;
    
    {
    utimer utimer("huffman encoder and decoder creation");
    huffman_codes(global_counts, encoder, decoder);
    }


    // ********** ENCODE THE FILE **********

    // TODO add the reserve to avoid the reallocation


    std::vector<std::tuple<long, std::vector<bool>* > *> encoded_chunks(nworkers); // (start index if the original file, encoded string)

    auto parallel_for_encode_function = [&](const long start, const long stop, const int thid) {


        // utimer utimer("map function encode, thread " + std::to_string(thid) + " (start_index " + std::to_string(start) + " stop_index " + std::to_string(stop) + ")");
        
        std::vector<bool> * encoding = new std::vector<bool>;
        
        for(long i = start; i < stop; i++)  {
            for (char bit : encoder[(int)mapped_file[i]]) {
                encoding->push_back(bit == '1');
            }
        }

        std::tuple<long, std::vector<bool>* > * tuple = new std::tuple<long, std::vector<bool>* >(start, encoding);

        encoded_chunks[thid] = tuple;
    };


    {
    utimer utimer("encoding file");
    pfr.parallel_for_idx(0, dataSize, 1, CHUNKSIZE, parallel_for_encode_function, nworkers);
    std::sort(
        encoded_chunks.begin(), 
        encoded_chunks.end(), 
        [](const std::tuple<long, std::vector<bool>* > * a, const std::tuple<long, std::vector<bool>* > * b) -> bool { 
            return (std::get<0>(*a) < std::get<0>(*b)); 
        }
    );
    }


    // ********** WRITE THE ENCODED FILE **********

    long encodedDataSize = 0;
    
    int padding = 0;

    std::vector<bool> encoded_contents;

    {
    utimer utimer("writing encoded file");

    for (auto tuple : encoded_chunks){
        encoded_contents.insert(encoded_contents.end(), std::get<1>(*tuple)->begin(), std::get<1>(*tuple)->end());
    }

    while(encoded_contents.size() % 8 != 0){
        encoded_contents.push_back(false);
        padding++;
    }
    
    std::ofstream encoded(encoded_filename);

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