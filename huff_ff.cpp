#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <iostream>
#include <utimer.cpp>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include<map>

#define CODE_POINTS 128

// global variables

char * mapped_file;
int ** counts;

void mmap_file(char * filepath,  char ** mapped_file){

    // TODO check the errors of the system calls

    utimer utimer("mmap file");

    int fd = open(filepath, O_RDONLY, 0);

    struct stat statbuf;
    fstat(fd, &statbuf);
    int file_len = statbuf.st_size;

    *mapped_file = (char*) mmap(
        NULL, file_len,
        PROT_READ, MAP_PRIVATE,
        fd, 0
    );

}


void map_function(const long start_index, const long stop_index, int thid){
    utimer utimer("map function thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");

    if (counts[thid] == nullptr){
        counts[thid] = new int[CODE_POINTS]();
    }

    for (long i = start_index; i < stop_index; i++){
        counts[thid][ (int) mapped_file[i]]++;
    }
        
}


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

    counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);

    mmap_file(filepath, &mapped_file);

    int dataSize = strlen(mapped_file);
    std::cout << "data size: " << dataSize << std::endl;


    // TODO check the parallel for pipe reduce
        
    //pfr.parallel_for_static(0,arraySize,1,CHUNKSIZE, [&](const long j) { A[j]=j*3.14; B[j]=2.1*j;});
    ff::parallel_for_idx(0, dataSize, 1 ,CHUNKSIZE, map_function, nworkers);



    // **************************


    auto reduce_function = [&](const long start_index, const long stop_index, std::map<char, int>& reduce_counts, const int thid) {

        utimer utimer("reduce function thread " + std::to_string(thid) + " (start_index " + std::to_string(start_index) + " stop_index " + std::to_string(stop_index) + ")");
        
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

    auto gather_function = [](std::map<char, int>& v, const std::map<char, int>& elem) {
        
        for (auto key_value: elem){
            v[key_value.first] += key_value.second;
        }
        
    };


    ff::parallel_reduce_idx(
        global_counts, std::map<char, int>(),
        0, CODE_POINTS, 1, CHUNKSIZE,
        reduce_function, 
        gather_function,
        nworkers
    );
    

    // *************************

    std::vector<std::pair<int, char>> global_counts_vector(CODE_POINTS);

    for(int i = 0; i<CODE_POINTS; i++){
        global_counts_vector[i] = std::make_pair(global_counts[i], i);
    }


    std::sort(
        global_counts_vector.begin(), 
        global_counts_vector.end(), 
        [](const std::pair<int, char>& a, const std::pair<int, char>& b) -> bool {
            return a.first < b.first;
        }
    );

    int * code = new int[CODE_POINTS];

    for (int i = 0; i < CODE_POINTS; i++) {
        code[(int) global_counts_vector[i].second] = i;
    }
    

    
    return 0;
}