#ifndef OPENCV_IO_HPP
#define OPENCV_IO_HPP

#include <janice.h>

#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#ifdef __cplusplus
extern "C" {
#endif

struct JaniceMediaType
{
    std::string filename;

    uint32_t channels;
    uint32_t rows;
    uint32_t cols;
    uint32_t frames;

    bool next(cv::Mat& img);
    bool seek(uint32_t frame);

private:
    cv::VideoCapture video;
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif // OPENCV_IO_HPP
