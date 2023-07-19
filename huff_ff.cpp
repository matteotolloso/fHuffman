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


    char * filepath;
    int nworkers;
    int CHUNKSIZE;

    if (argc != 4) {
        std::cout << "Usage: ./huff_ff <filepath> <nworkers> <chunksize>" << std::endl;
        filepath = "dataset/test.txt";
        nworkers = 1;
        CHUNKSIZE = 1;
    }
    else{
        filepath = argv[1];
        nworkers = atoi(argv[2]);
        CHUNKSIZE= atoi(argv[3]);
    }

    int ** counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);

    char * mapped_file;
    mmap_file(filepath, &mapped_file);

    int dataSize = strlen(mapped_file);
    std::cout << "data size: " << dataSize << std::endl;


    // PARALLEL FOR REDUCE INITIALIZATION

    // TODO check the parallel for pipe reduce
    ff::ParallelForReduce<std::map<char, int>> pfr(nworkers, false, false);


    // PARALLEL FOR (MAP) CHARACTER COUNT

    auto map_counts_function = [&](const long start_index, const long stop_index, int thid){
        
        utimer utimer("map function thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");

        if (counts[thid] == nullptr){
            counts[thid] = new int[CODE_POINTS]();
        }

        for (long i = start_index; i < stop_index; i++){
            counts[thid][ (int) mapped_file[i]]++;
        }
    };

    
    // TODO check the chunksize
    CHUNKSIZE = dataSize / nworkers;
    std::cout << "chunksize: " << CHUNKSIZE << std::endl;
    //pfr.parallel_for_static(0,arraySize,1,CHUNKSIZE, [&](const long j) { A[j]=j*3.14; B[j]=2.1*j;});
    pfr.parallel_for_idx(0, dataSize, 1 ,CHUNKSIZE, map_counts_function, nworkers);
    

    


    // PARALLEL REDUCE


    auto parallel_reduce_function = [&](const long start_index, const long stop_index, std::map<char, int>& reduce_counts, const int thid) {

        utimer utimer("parallel reduce function thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");
        
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
        utimer utimer("sequential reduce function");
        
        for (auto key_value: elem){
            v[key_value.first] += key_value.second;
        }
        
    };


    pfr.parallel_reduce_idx(
        global_counts, std::map<char, int>(),
        0, CODE_POINTS, 1, CHUNKSIZE,
        parallel_reduce_function, 
        sequential_reduce_function,
        nworkers
    );

    // print the counts
    for (int i = 0; i < CODE_POINTS; i++){
        if (global_counts[i] != 0){
            std::cout << (char) i << " " << global_counts[i] << std::endl;
        }
    }
    

    // BUILD THE HUFFMAN TREE

    std::vector<string> encoder(CODE_POINTS);
    MinHeapNode* decoder;
    
    huffman_codes(global_counts, encoder, decoder);

    // print the codes
    for (auto i = 0; i < encoder.size(); i++){
        if (encoder[i] != ""){
            std::cout << (char) i << " " << encoder[i] << std::endl;
        }
    }


    // ENCODE THE FILE

    auto map_encode_function = [&](const long start_index, const long stop_index, int thid){
        
        utimer utimer("map encode function thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");

        for (long i = start_index; i < stop_index; i++){
            mapped_file[i] = encoder[(int) mapped_file[i]][0];
        }
    };

    pfr.parallel_for_idx(0, dataSize, 1 ,CHUNKSIZE, map_encode_function, nworkers);


    

    
    return 0;
}