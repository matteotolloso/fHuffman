#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#include <iostream>
#include <utimer.cpp>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

    if (CHUNKSIZE == -1){
        CHUNKSIZE = dataSize / nworkers;
    }

    double sum = 0;

    ff::ParallelForReduce<double> pfr(nworkers, false );
        
    //pfr.parallel_for_static(0,arraySize,1,CHUNKSIZE, [&](const long j) { A[j]=j*3.14; B[j]=2.1*j;});
    pfr.parallel_for_idx(0, dataSize, 1 ,CHUNKSIZE, map_function, nworkers);





    // **************************
    
    auto Fsum = [](double& v, const double& elem) { v += elem; std::cout<<"v="<<v<<std::endl; };
    

    /*
    pfr.parallel_reduce(
        sum, 0.0, 
        0, arraySize,1, 2,
        [&](const long i, double& sum) {sum += A[i]; std::cout<<"sum="<<sum<<std::endl;}, 
        Fsum
    );
    

    */
    return 0;
}