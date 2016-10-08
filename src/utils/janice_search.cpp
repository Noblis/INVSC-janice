#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>

#include <janice.h>
#include <janice_io.h>

using namespace std;

const char *get_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void printUsage()
{
    printf("Usage: janice_evaluate_search sdk_path temp_path probes_list_file gallery_list_file gallery_file num_requested_returns candidate_list_file [-algorithm <algorithm>] [-verbose]\n");
}

int main(int argc, char *argv[])
{
    int requiredArgs = 8;

    if ((argc < requiredArgs) || (argc > 11)) {
        printUsage();
        return 1;
    }

    std::string algorithm = "";
    bool verbose = false;

    for (int i = 0; i < argc - requiredArgs; i++) {
        if (strcmp(argv[requiredArgs+i],"-algorithm") == 0)
            algorithm = argv[requiredArgs+(++i)];
        else if (strcmp(argv[requiredArgs+i],"-verbose") == 0)
            verbose = true;
        else {
            fprintf(stderr, "Unrecognized flag: %s\n", argv[requiredArgs+i]);
            return 1;
        }
    }

    JANICE_ASSERT(janice_initialize(argv[1], argv[2], algorithm, 0))
    JANICE_ASSERT(janice_search_helper(argv[3], argv[4], argv[5], atoi(argv[6]), argv[7], verbose))
    JANICE_ASSERT(janice_finalize())

    return EXIT_SUCCESS;
}
