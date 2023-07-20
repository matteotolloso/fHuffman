#ifndef UTILS_CPP
#define UTILS_CPP

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utimer.cpp>



void mmap_file(char * filepath,  char ** mapped_file){

    // TODO check the errors of the system calls

    utimer utimer("mmap file read");

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

void mmap_file_write(char * filepath, long file_len, char ** mapped_file ){

    // TODO check the errors of the system calls

    utimer utimer("mmap file write");

    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600); 

    ftruncate(fd, file_len);

    (*mapped_file) = (char*) mmap(0, file_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

void mmap_file_sync(char** mapped_file, long file_len){
    msync(mapped_file, file_len, MS_SYNC);
    munmap(mapped_file, file_len);
}

#endif
