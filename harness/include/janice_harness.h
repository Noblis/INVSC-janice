#ifndef JANICE_HARNESS_H
#define JANICE_HARNESS_H

#include <janice.h>
#include <janice_io.h>
#include <janice_io_opencv.h>

#include <fast-cpp-csv-parser/csv.h>

#include <string>
#include <vector>
#include <set>
#include <thread>
#include <cstring>
#include <unordered_map>
#include <type_traits>
#include <sstream>

// nonfatal errors as provided on command line
static inline void janice_harness_register_nonfatal_errors(const std::vector<std::string>& error_list, std::set<JaniceError>& ignored_errors) {
    for (auto &error_str : error_list) {
        JaniceError error;
        if      (error_str == "JANICE_SUCCESS")                    error = JANICE_SUCCESS;
        else if (error_str == "JANICE_UNKNOWN_ERROR")              error = JANICE_UNKNOWN_ERROR;
        else if (error_str == "JANICE_INTERNAL_ERROR")             error = JANICE_INTERNAL_ERROR;
        else if (error_str == "JANICE_OUT_OF_MEMORY")              error = JANICE_OUT_OF_MEMORY;
        else if (error_str == "JANICE_INVALID_SDK_PATH")           error = JANICE_INVALID_SDK_PATH;
        else if (error_str == "JANICE_BAD_SDK_CONFIG")             error = JANICE_BAD_SDK_CONFIG;
        else if (error_str == "JANICE_BAD_LICENSE")                error = JANICE_BAD_LICENSE;
        else if (error_str == "JANICE_MISSING_DATA")               error = JANICE_MISSING_DATA;
        else if (error_str == "JANICE_INVALID_GPU")                error = JANICE_INVALID_GPU;
        else if (error_str == "JANICE_BAD_ARGUMENT")               error = JANICE_BAD_ARGUMENT;
        else if (error_str == "JANICE_OPEN_ERROR")                 error = JANICE_OPEN_ERROR;
        else if (error_str == "JANICE_READ_ERROR")                 error = JANICE_READ_ERROR;
        else if (error_str == "JANICE_WRITE_ERROR")                error = JANICE_WRITE_ERROR;
        else if (error_str == "JANICE_PARSE_ERROR")                error = JANICE_PARSE_ERROR;
        else if (error_str == "JANICE_INVALID_MEDIA")              error = JANICE_INVALID_MEDIA;
        else if (error_str == "JANICE_OUT_OF_BOUNDS_ACCESS")       error = JANICE_OUT_OF_BOUNDS_ACCESS;
        else if (error_str == "JANICE_MEDIA_AT_END")               error = JANICE_MEDIA_AT_END;
        else if (error_str == "JANICE_INVALID_ATTRIBUTE_KEY")      error = JANICE_INVALID_ATTRIBUTE_KEY;
        else if (error_str == "JANICE_MISSING_ATTRIBUTE")          error = JANICE_MISSING_ATTRIBUTE;
        else if (error_str == "JANICE_DUPLICATE_ID")               error = JANICE_DUPLICATE_ID;
        else if (error_str == "JANICE_MISSING_ID")                 error = JANICE_MISSING_ID;
        else if (error_str == "JANICE_MISSING_FILE_NAME")          error = JANICE_MISSING_FILE_NAME;
        else if (error_str == "JANICE_INCORRECT_ROLE")             error = JANICE_INCORRECT_ROLE;
        else if (error_str == "JANICE_FAILURE_TO_SERIALIZE")       error = JANICE_FAILURE_TO_SERIALIZE;
        else if (error_str == "JANICE_FAILURE_TO_DESERIALIZE")     error = JANICE_FAILURE_TO_DESERIALIZE;
        else if (error_str == "JANICE_BATCH_ABORTED_EARLY")        error = JANICE_BATCH_ABORTED_EARLY;
        else if (error_str == "JANICE_BATCH_FINISHED_WITH_ERRORS") error = JANICE_BATCH_FINISHED_WITH_ERRORS;
        else if (error_str == "JANICE_CALLBACK_EXIT_IMMEDIATELY")  error = JANICE_CALLBACK_EXIT_IMMEDIATELY;
        else if (error_str == "JANICE_NOT_IMPLEMENTED")            error = JANICE_NOT_IMPLEMENTED;
        else {
            throw std::runtime_error("Unknown error string: " + error_str);
        }

        ignored_errors.insert(error);
    }
}

#define JANICE_ASSERT(func, ignored_errors)             \
{                                                       \
    JaniceError rc = (func);                            \
    if (rc != JANICE_SUCCESS) {                         \
        fprintf(stderr, "Janice function failed!\n"     \
                        "    Error: %s\n"               \
                        "    Location: %s:%d\n",        \
                        janice_error_to_string(rc),     \
                        __FILE__, __LINE__);            \
        if (ignored_errors.find(rc) != \
            ignored_errors.end()) {    \
            exit(EXIT_FAILURE);                         \
        }                                               \
    }                                                   \
}

template <typename R, typename S>
using janus_enable_if = typename std::enable_if<R::value, S>::type;

template <typename T>
struct ListReader
{
    // support int lists
    template <typename U = T>
    U _get_item(const std::string& value, janus_enable_if<std::is_integral<U>, U>* = nullptr)
    {
        return std::stoi(value);
    }

    // support floating point lists
    template <typename U = T>
    U _get_item(const std::string& value, janus_enable_if<std::is_floating_point<U>, U>* = nullptr)
    {
        return (U) (std::stold(value));
    }

    // support string lists
    template <typename U = T>
    U _get_item(const std::string& value, janus_enable_if<std::is_same<U, std::string>, U>* = nullptr)
    {
        return std::string(value);
    }

    void operator()(const std::string& name, const std::string& value, std::vector<T>& out_list)
    {
        std::stringstream ss(value);
        std::string token;
       
        while (std::getline(ss, token, ',')) {
            out_list.push_back(_get_item(token));
        }
    }
};

#endif // JANICE_HARNESS_H
