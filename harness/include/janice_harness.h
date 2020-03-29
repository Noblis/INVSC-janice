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
        bool found = false;
        for (int err = (int)JANICE_SUCCESS; err < (int)JANICE_NUM_ERRORS; ++err) {
          if (error_str == janice_error_strings[err]) {
            ignored_errors.insert((JaniceError)err);
            found = true;
            break;
          }
        }
        if (! found) {
          throw std::runtime_error("Unknown error string: " + error_str);
        }
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
