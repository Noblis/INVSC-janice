#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include <janice_io.h>

using namespace std;
using namespace cv;

JaniceError janice_load_media(const string &filename, JaniceMedia &media)
{
    Mat img = imread(filename);
    if (!img.data) { // Couldn't load as an image maybe it's a video
        VideoCapture video(filename);
        if (!video.isOpened()) {
            fprintf(stderr, "Fatal - JanICE failed to read: %s\n", filename.c_str());
            return JANICE_INVALID_MEDIA;
        }

        Mat frame;
        if (!video.read(frame))
            return JANICE_INVALID_MEDIA;

        media.width = frame.cols;
        media.height = frame.rows;
        media.color_space = frame.channels() == 3 ? JANICE_BGR24 : JANICE_GRAY8;
        media.frame_rate = video.get(CV_CAP_PROP_FPS);

        do {
            JaniceData *data = new JaniceData[media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1)];
            memcpy(data, frame.data, media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1));
            media.data.push_back(data);
        } while (video.read(frame));

        return JANICE_SUCCESS;
    }

    media.width = img.cols;
    media.height = img.rows;
    media.color_space = (img.channels() == 3 ? JANICE_BGR24 : JANICE_GRAY8);
    media.frame_rate = 0;

    JaniceData *data = new JaniceData[media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1)];
    memcpy(data, img.data, media.width * media.height * (media.color_space == JANICE_BGR24 ? 3 : 1));
    media.data.push_back(data);

    return JANICE_SUCCESS;
}

JaniceError janice_free_media(JaniceMedia &media)
{
    for (size_t i = 0; i < media.data.size(); i++)
        delete media.data[i];
    return JANICE_SUCCESS;
}
