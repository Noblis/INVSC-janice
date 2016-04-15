#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>

#include <janice.h>
#include <janice_io.h>

using namespace std;

void printUsage()
{
    printf("Usage: janice_verify sdk_path temp_path templates_list_file_a templates_list_file_b scores_file [-algorithm <algorithm>] [-verbose]\n");
}

int main(int argc, char *argv[])
{
    int requiredArgs = 6;

    if ((argc < requiredArgs) || (argc > 9)) {
        printUsage();
        return 1;
    }

    char *algorithm = "";
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
    JANICE_ASSERT(janice_verify_helper(argv[3], argv[4], argv[5], verbose));
    JANICE_ASSERT(janice_finalize())

    return EXIT_SUCCESS;
}
