
# Parallel implementation of Huffman Code using native C++ threads and FastFlow library

## How to run

`FastFlow` must be installed on the machine in the default `g++` include path, otherwise you have to specify the path in the `CmakeLists.txt` file with the `include_directories()` command. If you are not on the default machine, you have to run the `mapping_string.sh` script locatend in `FastFlow` library.  

### General test

The scripts `general_test.sh` automatically compiles the code, runs the three different implementations with timing and compares the results. You only have to specify the path of the `jemalloc` library in the `JMALLOC_PATH` variable inside the script.

### Valgrind test

The script `valgrind_test.sh` automatically compiles the code and runs the three different implementations with `valgrind`.
Currently a small memory leak is present in the `FastFlow` implementation due probably to a bug in the library itself.
