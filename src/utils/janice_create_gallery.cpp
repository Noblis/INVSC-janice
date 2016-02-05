#include <stdlib.h>
#include <string.h>
#include <fstream>

#include "iarpa_janus.h"
#include "iarpa_janus_io.h"

const char *get_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void printUsage()
{
    printf("Usage: janus_create_gallery sdk_path temp_path templates_directory template_list_file flat_gallery_file [-algorithm <algorithm>] [-tuning_data <csvfile>] [-tuning_data_path <image dir>]\n");
}

int main(int argc, char *argv[])
{
    int requiredArgs = 6;

    if ((argc < requiredArgs) || (argc > 9)) {
        printUsage();
        return 1;
    }

    const char *ext1 = get_ext(argv[4]);
    if (strcmp(ext1, "csv") != 0) {
        printf("template_list_file must be \".csv\" format.\n");
        return 1;
    } 
    
    char *algorithm = NULL;
    char *tuning_csvfile = NULL;
    char *img_path = NULL;

    for (int i=0; i<argc-requiredArgs; i++)
        if (strcmp(argv[requiredArgs+i],"-algorithm") == 0)
            algorithm = argv[requiredArgs+(++i)];
	else if (strcmp(argv[requiredArgs+i], "-tuning_data") == 0)
            tuning_csvfile = argv[requiredArgs+(++i)];
	else if (strcmp(argv[requiredArgs+i], "-tuning_data_path") == 0)
            img_path = argv[requiredArgs+(++i)];
        else {
            fprintf(stderr, "Unrecognized flag: %s\n", argv[requiredArgs+i]);
            return 1;
        }

    JANUS_ASSERT(janus_initialize(argv[1], argv[2], algorithm, 0))
    JANUS_ASSERT(janus_set_tuning_data(img_path, tuning_csvfile))
    
    janus_gallery gallery;
    JANUS_ASSERT(janus_allocate_gallery(&gallery))

    int size;
    JANUS_ASSERT(janus_create_gallery(argv[3], argv[4], gallery, &size))

    janus_flat_gallery flat_gallery = new janus_data[size*janus_max_template_size()];
    size_t bytes;
    
    JANUS_ASSERT(janus_flatten_gallery(gallery, flat_gallery, &bytes))
    JANUS_ASSERT(janus_write_flat_gallery(argv[5], flat_gallery))
    JANUS_ASSERT(janus_finalize())
    return EXIT_SUCCESS;
}
