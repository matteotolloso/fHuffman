#ifndef UTILS_CPP
#define UTILS_CPP

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utimer.cpp>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>

void mmap_file(string filepath,  char ** mapped_file){

    utimer utimer("mmap file read");

    

    int fd = open(filepath.c_str(), O_RDONLY, 0);

    if (fd == -1){
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1){
        perror("Error getting the file size");
        exit(EXIT_FAILURE);
    }

    int file_len = statbuf.st_size;

    *mapped_file = (char*) mmap(
        NULL, file_len,
        PROT_READ, MAP_PRIVATE,
        fd, 0
    );

    if (*mapped_file == MAP_FAILED){
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
}

void mmap_file_write(string filepath, long file_len, char ** mapped_file ){

    utimer utimer("mmap file write");

    int fd = open(filepath.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600); 

    if (fd == -1){
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, file_len) == -1){
        perror("Error truncating file");
        exit(EXIT_FAILURE);
    }

    (*mapped_file) = (char*) mmap(0, file_len +1 , PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (*mapped_file == MAP_FAILED){
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
}

void mmap_file_sync(char** mapped_file, long file_len){
    int res = msync(mapped_file, file_len, MS_SYNC);
    if (res == -1){
        perror("Error syncing the file");
        exit(EXIT_FAILURE);
    }
    res = munmap(mapped_file, file_len);
    if (res == -1){
        perror("Error un-mmapping the file");
        exit(EXIT_FAILURE);
    }
}

#endif
