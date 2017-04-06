#include "janice_harness.h"


#include <iostream>


int main(int argc, char ** argv)
{
    // 0 - prgram name 
    // 1 -- sdk path
    // 2 -- input file
    // 3 -- output file
    // 4 -- distance threshold

    if (argc < 5) {
        std::cerr << "Insufficient arguments, expected usage is: janice_eval_cluster sdk_path input_filename output_filename threshold" << std::endl;
        return -1;
    }
    int rc = janice_initialize(argv[1], "", "", -1);
    if (rc != JANICE_SUCCESS) {
        std::cerr << "Initialization failed!" << std::endl;
        return -1;
    }
    
    std::cerr << "Reading pre-enrolled templates" << std::endl;

    std::cerr << "Calling cluster files" << std::endl;
    janice_cluster_files(argv[2], argv[3], atof(argv[4]));
    std::cerr << "Finished cluster files" << std::endl;

    janice_finalize();

    return 0;
}
