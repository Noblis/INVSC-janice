#ifndef JANICE_HARNESS_H
#define JANICE_HARNESS_H

#include "janice.h"
#include "janice_io.h"
#include "janice_io_opencv.h"

#include <string>
#include <vector>
#include <thread>
#include <cstring>

#define JANICE_ASSERT(func)                 \
{                                           \
    JaniceError rc = (func);                \
    if (rc != JANICE_SUCCESS) {             \
        printf("Janice function failed!\n"  \
               "    Error: %s\n"            \
               "    Location: %s:%d\n",     \
               janice_error_to_string(rc),  \
               __FILE__, __LINE__);         \
        exit(EXIT_FAILURE);                 \
    }                                       \
}

inline std::string get_ext(const std::string& filename) 
{
    size_t idx = filename.rfind('.');
    if (idx != std::string::npos)
        return filename.substr(idx + 1);
    return "";
}

inline bool parse_optional_args(int argc, char** argv, int min_args, int /*max_args*/, std::string& algorithm, int& num_threads, int& gpu)
{
    algorithm = "";
    num_threads = std::thread::hardware_concurrency();
    gpu = -1;

    int i;
    for (i = 0; i < (argc - min_args); ++i) {
      if (strcmp(argv[min_args + i], "-algorithm") == 0) {
        algorithm   = argv[min_args + (++i)];
      }
      else if (strcmp(argv[min_args + i], "-threads") == 0) {
        num_threads = atoi(argv[min_args + (++i)]);
      }
      else if (strcmp(argv[min_args + i], "-gpu") == 0) {
        gpu = atoi(argv[min_args + (++i)]);
      }
      else {
        printf("Unrecognized flag: %s\n", argv[min_args + i]);
        return false;
      }
    }

    return true;
}

#endif
 
