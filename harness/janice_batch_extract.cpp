#include "janice_harness.h"

#include <iostream>
#include <thread>

int main(int argc, char ** argv) {
    // 1 -- sdk path
    // 2 -- input_file
    // 3 -- outptu_file
    // 4 -- optional, nthreads

    if (argc < 4) {
        std::cerr << "Insufficient arguments, expected usage is: janice_batch_extract sdk_path input_filename output_filename (nThreads) " << std::endl;
    }
    int nThreads = std::thread::hardware_concurrency();

    if (argc >= 5)
        nThreads = atoi(argv[4]);

    int rc = janice_initialize(argv[1],"","",nThreads,nullptr,0);
    if (rc != JANICE_SUCCESS) {
        std::cerr << "Janice initialization failed!" << std::endl;
        return -1;
    }

    std::cerr << "Performing extration" << std::endl;
    janice_batch_extract_templates(argv[2], argv[3], nThreads);
    std::cerr << "Finished extracting files" << std::endl;
    janice_finalize();

    return 0;
}
