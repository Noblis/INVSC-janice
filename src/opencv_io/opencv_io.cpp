#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <janice_io.h>

using namespace std;
using namespace cv;

janice_error janice_load_media(const string &filename, janice_media &media)
{
    Mat img = imread(filename);
    if (!img.data) { // Couldn't load as an image maybe it's a video
        VideoCapture video(filename);
        if (!video.isOpened()) {
            fprintf(stderr, "Fatal - Janice failed to read: %s\n", filename.c_str());
            return JANICE_INVALID_MEDIA;
        }

        Mat frame;
        bool got_frame = video.read(frame);
        if (!got_frame)
            return JANICE_INVALID_MEDIA;

        media.width = frame.cols;
        media.height = frame.rows;
        media.color_space = frame.channels() == 3 ? JANICE_BGR24 : JANICE_GRAY8;

        do {
            janice_data *data = new janice_data[media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1)];
            memcpy(data, frame.data, media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1));
            media.data.push_back(data);
        } while (video.read(frame));
    }

    media.width = img.cols;
    media.height = img.rows;
    media.color_space = (img.channels() == 3 ? JANICE_BGR24 : JANICE_GRAY8);

    janice_data *data = new janice_data[media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1)];
    memcpy(data, img.data, media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1));
    media.data.push_back(data);

    return JANICE_SUCCESS;
}

janice_error janice_free_media(janice_media &media)
{
    for (size_t i = 0; i < media.data.size(); i++)
        delete media.data[i];
    return JANICE_SUCCESS;
}
