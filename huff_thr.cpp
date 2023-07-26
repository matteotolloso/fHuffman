#include <shared_queue.cpp>
#include <utimer.cpp>
#include <functional>
#include <optional>
#include <utils.cpp>
#include <huffman_codes.cpp>
#include <string>
#include <barrier>

#define CODE_POINTS 128

int main(int argc, char * argv[]){

    std::string original_filename = argv[1];
    std::string encoded_filename = argv[2];
    int nworkers = atoi(argv[3]);

    // std::string original_filename = "./dataset/1gb.txt";
    // std::string encoded_filename = "./outputs/test_thr.txt";
    // int nworkers = 2;

    
    // ********** PREPARING THE THREADS **********

    ThreadSafeQueue<std::optional<std::function<void(int)>>> task_queue;
    std::barrier barrier(nworkers + 1);
    std::vector<std::thread> workers;

    auto worker_function = [&] (int thid){
        while (true){
            std::optional<std::function<void(int)>> function = task_queue.pop();
            
            if (function == std::nullopt){
                break;
            }
            
            function.value()(thid);
            barrier.arrive_and_wait();
        }
    };

    for (int i = 0; i < nworkers; i++){
        workers.push_back(std::thread(worker_function, i));
    }
    

    // ********** READ THE FILE **********

    char * mapped_file;
    long long unsigned dataSize = mmap_file(original_filename, &mapped_file); 
    
    // ********** COUNT THE CHARACTERS **********

    int ** counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);
    std::vector<std::map<char, long long unsigned>>reduce_counts(nworkers);
    std::map<char, long long unsigned> global_counts;

    auto parallel_for_counts_function = [&] (int thid){

        // utimer utimer("parallel_for_counts_function, thread " + std::to_string(thid));

        // calculate my start and stop index
        long long unsigned start_index = thid * (dataSize / nworkers);
        long long unsigned stop_index = (thid != (nworkers - 1)) ? (thid + 1) * (dataSize / nworkers) : dataSize;

        if (counts[thid] == nullptr){
            counts[thid] = new int[CODE_POINTS]();
        }

        for (long long unsigned i = start_index; i < stop_index; i++){
            counts[thid][ (int) mapped_file[i]]++;
        }
    };
 
    auto parallel_reduce_function = [&] (int thid){

        // utimer utimer("parallel reduce counts, thread " + std::to_string(thid));

        // calculate my start and stop index
        long long unsigned start_index = thid * (CODE_POINTS / nworkers);
        long long unsigned stop_index = (thid != (nworkers - 1)) ? (thid + 1) * (CODE_POINTS / nworkers) : CODE_POINTS;

        for (long long unsigned i = start_index; i < stop_index; i++){
            reduce_counts[thid][i] = 0;
        }

        for (int i = 0; i < nworkers; i++){
            for (long long unsigned j = start_index; j < stop_index; j++){
                reduce_counts[thid][j] += counts[i][j];
            }
        }
    };


    {
    utimer utimer("counting characters");

    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::optional<std::function<void(int)>>{parallel_for_counts_function});
    }
    barrier.arrive_and_wait();

    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::optional<std::function<void(int)>>{parallel_reduce_function});
    }
    barrier.arrive_and_wait();

    for (int i = 0; i < nworkers; i++){
        for (auto it = reduce_counts[i].begin(); it != reduce_counts[i].end(); it++){
            global_counts[it->first] = it->second;
        }
    }

    }

    // ********** BUILD THE HUFFMAN TREE **********

    std::vector<string> encoder(CODE_POINTS);
    MinHeapNode* decoder;
    
    {
    utimer utimer("huffman encoder and decoder creation");
    huffman_codes(global_counts, encoder, decoder);
    }


  // ********** ENCODE THE FILE **********

    std::vector<std::tuple<long long unsigned, std::deque<bool>* > *> encoded_chunks(nworkers); // (start index if the encoded file, encoded string)

    auto parallel_for_encode_function = [&](int thid) {

        // calculate my start and stop index
        unsigned long long start_index = thid * (dataSize / nworkers);
        unsigned long long stop_index = (thid != (nworkers - 1)) ? (thid + 1) * (dataSize / nworkers) : dataSize;
        
        std::deque<bool> * encoding = new std::deque<bool>;
        
        for(unsigned long long i = start_index; i < stop_index; i++)  {
            for (char bit : encoder[(int)mapped_file[i]]) {
                encoding->push_back(bit == '1');
            }
        }

        std::tuple<long long unsigned, std::deque<bool>* > * tuple = new std::tuple<long long unsigned, std::deque<bool>* >(0, encoding);

        encoded_chunks[thid] = tuple;

    };

    {
    utimer utimer("encoding file");

    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::optional<std::function<void(int)>>{parallel_for_encode_function});
    }
    barrier.arrive_and_wait();

    }

    // ********** BALANCING ENCODING **********

    int padding = 0;

    {
    utimer utimer("balancing encoding");

    for (long long unsigned i = 0; i < encoded_chunks.size() - 1; i++){
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

    }


    // ********** COMPRESSING AND WRITING **********

    long long unsigned encoded_compressed_size = 0;
    char * mapped_output_file;

    for (auto ch : encoded_chunks){
        encoded_compressed_size += (std::get<1>(*ch)->size() / 8);
    }

    mmap_file_write(encoded_filename, encoded_compressed_size, mapped_output_file);


    auto parallel_for_compress_function = [&](const int thid) {

        // utimer utimer("parallel_for_compress_function, thread " + std::to_string(thid));
        
        std::deque<bool>  encoding = *std::get<1>(*encoded_chunks[thid]);
        long long unsigned start_index = std::get<0>(*encoded_chunks[thid]);
        
        for (long long unsigned i = 0; i < encoding.size(); i += 8) {
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

    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::optional<std::function<void(int)>>{parallel_for_compress_function});
    }
    barrier.arrive_and_wait();

    mmap_file_sync(mapped_output_file, encoded_compressed_size);

    }


    // ********** CLOSE THE THREADS **********

    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::nullopt);
    }
    for (int i = 0; i < nworkers; i++){
        workers[i].join();
    }

}

