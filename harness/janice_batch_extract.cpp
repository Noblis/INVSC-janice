#include "janice_harness.h"

#include <iostream>

int main(int argc, char ** argv) {
    // 1 -- sdk path
    // 2 -- input_file
    // 3 -- outptu_file

    if (argc < 4) {
        std::cerr << "Insufficient arguments, expected usage is: janice_batch_extract sdk_path input_filename output_filename" << std::endl;
    }

    int rc = janice_initialize(argv[1],"","",-1);
    if (rc != JANICE_SUCCESS) {
        std::cerr << "Janice initialization failed!" << std::endl;
        return -1;
    }

    std::cerr << "Performing extration" << std::endl;
    janice_batch_extract_templates(argv[2], argv[3]);
    std::cerr << "Finished extracting files" << std::endl;
    janice_finalize();

    return 0;
}
