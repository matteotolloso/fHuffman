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

  
    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];
    int nworkers = atoi(argv[3]);

    // std::string original_filename = "./dataset/test.txt";
    // std::string encoded_filename = "./outputs/huff_ff.txt";
    // int nworkers = 2;
    
    
    int CHUNKSIZE = 0; // static scheduling


    // ********** READ THE FILE **********
    char * mapped_file;
    mmap_file(original_filename, &mapped_file);
    long unsigned dataSize = strlen(mapped_file);


// ********** COUNT THE CHARACTERS **********

    int ** counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);


    ff::ParallelForReduce<std::map<char, int>> pfr(nworkers, false, false);


    auto parallel_for_counts_function = [&](const long start_index, const long stop_index, int thid){
        
        utimer utimer("parallel_for_counts_function, thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");

        if (counts[thid] == nullptr){
            counts[thid] = new int[CODE_POINTS]();
        }

        for (long i = start_index; i < stop_index; i++){
            counts[thid][ (int) mapped_file[i]]++;
        }
    };
    

    auto parallel_reduce_function = [&](const long start_index, const long stop_index, std::map<char, int>& reduce_counts, const int thid) {

        utimer utimer("parallel reduce counts, thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");
        
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

    // TODO avoid the sort assigning each thread a chunk of the file
    std::vector<std::tuple<long, std::deque<bool>* > *> encoded_chunks(nworkers); // (start index if the original file, encoded string)

    auto parallel_for_encode_function = [&](const long start, const long stop, const int thid) {


        utimer utimer("parallel_for_encode_function, thread " + std::to_string(thid) + " (start_index " + std::to_string(start) + " stop_index " + std::to_string(stop) + ")");
        
        std::deque<bool> * encoding = new std::deque<bool>;
        
        for(long i = start; i < stop; i++)  {
            for (char bit : encoder[(int)mapped_file[i]]) {
                encoding->push_back(bit == '1');
            }
        }

        std::tuple<long, std::deque<bool>* > * tuple = new std::tuple<long, std::deque<bool>* >(start, encoding);

        encoded_chunks[thid] = tuple;
    };


    {
    utimer utimer("encoding file");
    pfr.parallel_for_idx(0, dataSize, 1, CHUNKSIZE, parallel_for_encode_function, nworkers);
    std::sort(
        encoded_chunks.begin(), 
        encoded_chunks.end(), 
        [](const std::tuple<long, std::deque<bool>* > * a, const std::tuple<long, std::deque<bool>* > * b) -> bool { 
            return (std::get<0>(*a) < std::get<0>(*b)); 
        }
    );
    }

    // ********** BALANCING ENCODING **********

    int padding = 0;

    {
    utimer utimer("balancing encoding");

    for (long unsigned i = 0; i < encoded_chunks.size() - 1; i++){
        while(std::get<1>(*encoded_chunks[i])->size() % 8 != 0){
            bool element = (bool)std::get<1>(*(encoded_chunks[i+1]))->front();
            std::get<1>(*encoded_chunks[i])->push_back(element);
            std::get<1>(*(encoded_chunks[i+1]))->pop_front();
        }
        // update the start index of the next chunk with the byte size it has to start from
        std::get<0>(*encoded_chunks[i+1]) = std::get<0>(*encoded_chunks[i]) + (std::get<1>(*encoded_chunks[i])->size() / 8);
    }

    // pad the last chunk
    while(std::get<1>(*encoded_chunks[encoded_chunks.size()-1])->size() % 8 != 0){
        std::get<1>(*encoded_chunks[encoded_chunks.size()-1])->push_back(false);
        padding++;
    }

    }
    // ********** COMPRESSING AND WRITING **********

    long encoded_compressed_size = 0;
    char * mapped_output_file;

    for (auto ch : encoded_chunks){
        encoded_compressed_size += (std::get<1>(*ch)->size() / 8);
        std::cout << std::get<1>(*ch)->size() << std::endl;
        std::cout << std::get<0>(*ch) << std::endl;
    }

    mmap_file_write(encoded_filename, encoded_compressed_size, mapped_output_file);


    auto parallel_for_compress_function = [&](const long index, const int thid) {

        utimer utimer("parallel_for_compress_function, thread " + std::to_string(thid) + " (index " + std::to_string(index) + ")");
        
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
            cout<< "thread " << thid << " byte " << byte << " start_index " << start_index << endl;
            mapped_output_file[start_index++] = byte;
        }
    };

    {
    utimer utimer("compressing and writing");

    pfr.parallel_for_thid(0, encoded_chunks.size(), 1, -1, parallel_for_compress_function, nworkers);
    mmap_file_sync(mapped_output_file, encoded_compressed_size);

    } 
    

    return 0;
}