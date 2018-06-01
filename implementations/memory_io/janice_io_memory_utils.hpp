#ifndef JANICE_IO_MEMORY_UTILS_HPP
#define JANICE_IO_MEMORY_UTILS_HPP

#include <janice_io.h>

#include <cstring>

namespace mem_utils
{

inline JaniceError copy_janice_image(const JaniceImage& src, JaniceImage& dst)
{
    // Set up the dimensions
    dst.channels = src.channels;
    dst.rows     = src.rows;
    dst.cols     = src.cols;

    dst.data = (uint8_t*) malloc(src.channels * src.rows * src.cols);
    memcpy(dst.data, src.data, src.channels * src.rows * src.cols);
    dst.owner = true;

    return JANICE_SUCCESS;
}

} // namespace mem_utils

#endif // JANICE_IO_MEMORY_UTILS_HPP
