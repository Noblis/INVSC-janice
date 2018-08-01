#ifndef JANICE_IO_OPENCV_UTILS_HPP
#define JANICE_IO_OPENCV_UTILS_HPP

#include <janice_io.h>
#include <opencv2/core.hpp>

namespace ocv_utils
{

inline JaniceError cv_mat_to_janice_image(cv::Mat& m, JaniceImage& image)
{
    // Set up the dimensions
    image.channels = m.channels();
    image.rows = m.rows;
    image.cols = m.cols;

    image.data = (uint8_t*) malloc(m.channels() * m.rows * m.cols);
    memcpy(image.data, m.data, m.channels() * m.rows * m.cols);
    image.owner = true;

    return JANICE_SUCCESS;
}

} // namespace ocv_utils

#endif // JANICE_IO_OPENCV_UTILS_HPP
