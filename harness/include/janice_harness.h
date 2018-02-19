#ifndef JANICE_HARNESS_H
#define JANICE_HARNESS_H

#include <janice.h>
#include <janice_io.h>
#include <janice_io_opencv.h>

#include <fast-cpp-csv-parser/csv.h>

#include <string>
#include <vector>
#include <thread>
#include <cstring>
#include <unordered_map>

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

struct GPUReader
{
    void operator()(const std::string& name, const std::string& value, std::vector<int>& gpus)
    {
        size_t pos = 0;
        while (pos + 1 < value.size()) {
            size_t comma_location = 0;
            gpus.push_back(std::stoi(std::string(value, pos), &comma_location));
            pos += comma_location + 1;
        }
    }
};

#endif // JANICE_HARNESS_H
