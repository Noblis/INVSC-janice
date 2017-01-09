#ifndef JANICE_CPP_HPP
#define JANICE_CPP_HPP

#include <janice.h>

#include <string>
#include <vector>
#include <stdexcept>

#define JANICE_C_CHECK(func)               \
{                                          \
    JaniceError result = (func);           \
    if (result != JANICE_SUCCESS)          \
        throw std::runtime_error(          \
            janice_error_to_string(result) \
        );                                 \
}

namespace janice
{

// ----------------------------------------------------------------------------
// Initialization

JANICE_EXPORT void janice_cpp_initialize(const std::string& sdk_path,
                                         const std::string& temp_path,
                                         const std::string& algorithm,
                                         const int gpu_dev)
{
    JANICE_C_CHECK(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), gpu_dev))
}

// ----------------------------------------------------------------------------
// Training

JANICE_EXPORT void janice_cpp_train(const std::string& data_prefix,
                                    const std::string& data_list)
{
    JANICE_C_CHECK(janice_train(data_prefix.c_str(), data_list.c_str()))
}

// ----------------------------------------------------------------------------
// Media I/O

struct JaniceCppMedia
{
    JaniceCppMedia() :
        c_ptr(nullptr) {}

    JaniceCppMedia(const std::string& filename)
    {
        JANICE_C_CHECK(janice_create_media(filename, &c_ptr))
    }

    JaniceCppMedia(JaniceMedia media) :
        c_ptr(media) {}

    // disable the copy and move constructors (to make pointer management easier
    JaniceCppMedia(const JaniceCppMedia& other) = delete;
    JaniceCppMedia(JaniceCppMedia&& other) = delete;

    ~JaniceCppMedia()
    {
        if (c_ptr)
            JANICE_C_CHECK(janice_free_media(&c_ptr))
    }

    JaniceMedia c_ptr;
};



} // namespace janice

#endif // JANICE_CPP_HPP
