#ifndef JANICE_HARNESS_H
#define JANICE_HARNESS_H

#include "janice.h"
#include "janice_io.h"
#include "janice_io_opencv.h"

#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define JANICE_ASSERT(func)                 \
{                                           \
    JaniceError rc = (func);                \
    if (rc != JANICE_SUCCESS) {             \
        printf("Janice function %s failed!\n"  \
               "    Error: %s\n"            \
               "    Location: %s:%d\n",     \
               STRINGIFY(func),             \
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

inline bool file_exists(const std::string &filename)
{
  return (access(filename.c_str(), F_OK) != -1);
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
 
