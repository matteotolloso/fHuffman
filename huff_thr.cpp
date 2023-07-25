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

    
    // ********** PREPARING THE THREADS **********

    ThreadSafeQueue<std::optional<std::function<void(int)>>> task_queue;
    std::barrier barrier(nworkers + 1);


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

    std::vector<std::thread> workers;
    for (int i = 0; i < nworkers; i++){
        workers.push_back(std::thread(worker_function, i));
    }
    

    // ********** READ THE FILE **********

    char * mapped_file;
    mmap_file(original_filename, &mapped_file);
    long unsigned dataSize = strlen(mapped_file);

    
    // ********** COUNT THE CHARACTERS **********

    int ** counts = new int*[nworkers]{};
    std::fill(counts, counts+nworkers, nullptr);

    auto parallel_for_counts_function = [&] (int thid){

        // utimer utimer("parallel_for_counts_function, thread " + std::to_string(thid));

        // calculate my start and stop index
        long start_index = thid * (dataSize / nworkers);
        long stop_index = (thid != (nworkers - 1)) ? (thid + 1) * (dataSize / nworkers) : dataSize;

        std::cout << "thread " << thid << " start_index " << start_index << " stop_index " << stop_index << std::endl;

        if (counts[thid] == nullptr){
            counts[thid] = new int[CODE_POINTS]();
        }

        for (long i = start_index; i < stop_index; i++){
            counts[thid][ (int) mapped_file[i]]++;
        }
    };


    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::optional<std::function<void(int)>>{parallel_for_counts_function});
    }
    barrier.arrive_and_wait();

    // print the counts
    for (int i = 0; i < nworkers; i++){
        for (int j = 0; j < CODE_POINTS; j++){
            std::cout << counts[i][j] << " ";
        }
        std::cout << std::endl;
    }


  




    
    // ********** CLOSE THE THREADS **********

    for (int i = 0; i < nworkers; i++){
        task_queue.push(std::nullopt);
    }
    for (int i = 0; i < nworkers; i++){
        workers[i].join();
    }

}

