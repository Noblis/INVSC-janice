#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

void print_usage()
{
    printf("Usage: janice_verify sdk_path temp_path templates_list_file_a templates_list_file_b scores_file [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 6;
    const int max_args = 12;

    if (argc < min_args || argc > max_args) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    const std::string sdk_path       = argv[1];
    const std::string temp_path      = argv[2];
    const std::string templates_a_path = argv[3];
    const std::string templates_b_path = argv[4];
    const std::string scores         = argv[5];

    std::string algorithm;
    int num_threads;
    int gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu)) {
      exit(EXIT_FAILURE);
    }
    
    JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1));
    JaniceTemplates templates_a;
    memset(&templates_a, '\0', sizeof(templates_a));
    JaniceTemplates templates_b;
    memset(&templates_b, '\0', sizeof(templates_b));
    // TODO: Read template files. Figure out how to handle template IDs--presumably in templates list
    // file, but not in templates themselves, so we have to maintain parallel structures for those.
    JANICE_ASSERT(janice_finalize());
    exit(1);
}
