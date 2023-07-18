#ifndef UTILS_CPP
#define UTILS_CPP

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utimer.cpp>

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

#endif