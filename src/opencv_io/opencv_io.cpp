#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "../../include/janice_io.h"

using namespace cv;

static janice_image janiceFromOpenCV(const Mat &mat)
{
    assert(mat.data && mat.isContinuous());
    janice_image image;
    image.width = mat.cols;
    image.height = mat.rows;
    image.color_space = (mat.channels() == 3 ? JANICE_BGR24 : JANICE_GRAY8);
    image.data = (janice_data*)malloc(image.width * image.height * (image.color_space == JANICE_BGR24 ? 3 : 1));
    memcpy(image.data, mat.data, image.width * image.height * (image.color_space == JANICE_BGR24 ? 3 : 1));
    return image;
}

janice_error janice_read_image(const char *file_name, janice_image *image)
{
    const Mat mat = imread(file_name);
    if (!mat.data) {
        fprintf(stderr, "Fatal - Janus failed to read: %s\n", file_name);
        return JANICE_INVALID_IMAGE;
    }
    *image = janiceFromOpenCV(mat);
    return JANICE_SUCCESS;
}

void janice_free_image(janice_image image)
{
    free(image.data);
}

janice_error janice_open_video(const char *file_name, janice_video *video)
{
    *video = reinterpret_cast<janice_video>(new VideoCapture(file_name));
    return JANICE_SUCCESS;
}

janice_error janice_read_frame(janice_video video, janice_image *image)
{
    Mat mat;
    reinterpret_cast<VideoCapture*>(video)->read(mat);
    if (!mat.data)
        return JANICE_INVALID_VIDEO;
    *image = janiceFromOpenCV(mat);
    return JANICE_SUCCESS;
}

void janice_close_video(janice_video video)
{
    delete reinterpret_cast<VideoCapture*>(video);
}
