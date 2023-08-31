#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <iostream>
#include <utimer.cpp>
#include <fcntl.h>
#include <map>
#include <huffman_codes.cpp>
#include <utils.cpp>


#define CODE_POINTS 128


int main(int argc, char * argv[]) { 

    if(argc != 4){
        std::cout << "Usage: " << argv[0] << " <original_filename> <encoded_filename> <nworkers>" << std::endl;
        return 1;
    }

    utimer tt("total time");
  
    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];
    int nworkers = atoi(argv[3]);

    int CHUNKSIZE = 0; // static scheduling


    // ********** READ AND COUNT **********

    char * mapped_file;
    long long dataSize = mmap_file(original_filename, &mapped_file); 

    int ** counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);


    ff::ParallelForReduce<std::map<char, long long unsigned>> pfr(nworkers);


    auto parallel_for_counts_function = [&](const long long unsigned start_index, const long long unsigned stop_index, int thid){
        
        // utimer utimer("parallel_for_counts_function, thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");

        if (counts[thid] == nullptr){
            counts[thid] = new int[CODE_POINTS]();
        }

        for (long long unsigned i = start_index; i < stop_index; i++){
            counts[thid][ (int) mapped_file[i]]++;
        }
    };
    

    auto parallel_reduce_function = [&](const long long unsigned  start_index, const long long unsigned stop_index, std::map<char, long long unsigned >& reduce_counts, const int thid) {

        // utimer utimer("parallel reduce counts, thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");
        
        for (long long unsigned i = start_index; i < stop_index; i++){
            reduce_counts[i] = 0;
        }

        // iterate fixing first the mapper worker (the array that produced) and then the char chunk
        // for cache efficiency
        for (int j = 0; j < nworkers; j++){  // iterate all the partial computations (mapper threads)
            for (long long unsigned i = start_index; i < stop_index; i++){   // assigned char chunk
                reduce_counts[i] += counts[j][i]; // only this thread is wrinting on reduce counts because is local
            }    
        } 
    };

    std::map<char, long long unsigned> global_counts;

    auto sequential_reduce_function = [](std::map<char, long long unsigned>& v, const std::map<char, long long unsigned>& elem) {
        
        // utimer utimer("sequential reduce counts");
        
        for (auto key_value: elem){
            v[key_value.first] += key_value.second;
        }
        
    };


    {
    utimer utimer("read and count");

    pfr.parallel_for_idx(0, dataSize, 1 ,CHUNKSIZE, parallel_for_counts_function, nworkers);
    
    pfr.parallel_reduce_idx(
        global_counts, std::map<char, long long unsigned>(),
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

    std::vector<std::tuple<long, std::deque<bool>* > *> encoded_chunks(nworkers); // (start index if the original file, encoded string)

    auto parallel_for_encode_function = [&](const long start, const long stop, const int thid) {

        // utimer utimer("parallel_for_encode_function, thread " + std::to_string(thid) + " (start_index " + std::to_string(start) + " stop_index " + std::to_string(stop) + ")");
        
        std::deque<bool> * encoding = new std::deque<bool>;
        
        for(long i = start; i < stop; i++)  {
            for (char bit : encoder[(int)mapped_file[i]]) {
                encoding->push_back(bit == '1');
            }
        }

        std::tuple<long, std::deque<bool>* > * tuple = new std::tuple<long, std::deque<bool>* >(0, encoding);

        encoded_chunks[thid] = tuple;
    };


    {
    utimer utimer("encoding file");
    pfr.parallel_for_idx(0, dataSize, 1, CHUNKSIZE, parallel_for_encode_function, nworkers);
    }

    // ********** BALANCING ENCODING **********

    int padding = 0;
    long encoded_compressed_size = 0;

    {
    utimer utimer("balancing encoding");

    for (long unsigned i = 0; i < encoded_chunks.size() - 1; i++){
        while(std::get<1>(*encoded_chunks[i])->size() % 8 != 0){
            bool element = (bool)std::get<1>(*(encoded_chunks[i+1]))->front();
            std::get<1>(*encoded_chunks[i])->push_back(element);
            std::get<1>(*(encoded_chunks[i+1]))->pop_front();
        }
        // update the start index of the next chunk with the byte size it has to start from
        // so they can by written in parallel
        std::get<0>(*encoded_chunks[i+1]) = std::get<0>(*encoded_chunks[i]) + (std::get<1>(*encoded_chunks[i])->size() / 8);
    }

    // pad the last chunk
    while(std::get<1>(*encoded_chunks[encoded_chunks.size()-1])->size() % 8 != 0){
        std::get<1>(*encoded_chunks[encoded_chunks.size()-1])->push_back(false);
        padding++;
    }

    encoded_compressed_size = std::get<0>(*encoded_chunks[encoded_chunks.size()-1]) + (std::get<1>(*encoded_chunks[encoded_chunks.size()-1])->size() / 8);

    }
    
    // ********** COMPRESSING AND WRITING **********

    char * mapped_output_file;

    mmap_file_write(encoded_filename, encoded_compressed_size, mapped_output_file);


    auto parallel_for_compress_function = [&](const long index, const int thid) {

        // utimer utimer("parallel_for_compress_function, thread " + std::to_string(thid));
        
        std::deque<bool>  encoding = *std::get<1>(*encoded_chunks[index]);
        long start_index = std::get<0>(*encoded_chunks[index]);
        
        for (long unsigned i = 0; i < encoding.size(); i += 8) {
            char byte = 0;
            for (int j = 0; j < 8; j++) {
                byte = byte << 1;
                if (encoding[i + j]) {
                    byte = byte | 1;
                }
            }
            mapped_output_file[start_index++] = byte;
        }
    };

    {
    utimer utimer("compressing and writing");

    pfr.parallel_for_thid(0, encoded_chunks.size(), 1, -1, parallel_for_compress_function, nworkers);
    mmap_file_sync(mapped_output_file, encoded_compressed_size);

    } 


    // ********** CLEAN UP **********

    clean_up(decoder);

    for (int i=0; i<nworkers; i++){
        delete[] counts[i];
        delete std::get<1>(*encoded_chunks[i]);
        delete encoded_chunks[i];
    }

    delete[] counts;
  


    
    

    return 0;
}