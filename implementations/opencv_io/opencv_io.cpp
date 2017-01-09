#include <opencv_io.hpp>

#include <string.h>

#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

bool JaniceMediaType::next(Mat& img)
{
    if (!video.isOpened()) { // image
        img = imread(filename, IMREAD_ANYCOLOR);
        return true;
    }

    // video - a little bit more complicated
    bool got_frame = video.read(img);
    if (!got_frame) { // Something went unexpectedly wrong (maybe a corrupted video?). Print a warning, set img to empty and return true.
        fprintf(stderr, "Fatal - Unexpectedly unable to collect next frame from video.");
        img = Mat();
        return true;
    }

    if (video.get(CV_CAP_PROP_POS_FRAMES) == frames) { // end of the video. Reset it and return true
        video.set(CV_CAP_PROP_POS_FRAMES, 0);
        return true;
    }

    return false;
}

bool JaniceMediaType::seek(uint32_t frame)
{
    if (!video.isOpened()) // image
        return false;

    if (frame >= frames) // invalid index
        return false;

    // Set the video to the desired frame
    video.set(CV_CAP_PROP_POS_FRAMES, frame);
    return true;
}

JaniceError janice_create_media(const char* filename, JaniceMedia* media)
{
   media = new JaniceMediaType();
   media->filename = new char[strlen(filename) + 1];
   strcpy(media->filename, filename);
   media->filename[strlen(filename)] = '\0';
   media->frames = 1; // Initialize this to the image default to avoid an extra condition later

   // To get the media dimensions we temporalily load either the image or the
   // the first frame. This also checks if the filename is valid
   Mat img = imread(filename, IMREAD_ANYCOLOR); // We use ANYCOLOR to load either RGB or Grayscale images
   if (!img.data) { // Couldn't load as an image maybe it's a video
       media->video = VideoCapture(filename);
       if (!video.isOpened()) {
           fprintf(stderr, "Fatal - OpenCV I/O failed to read: %s\n", filename);
           delete media->filename; // Delete media memory before returning an error
           delete media;
           return JANICE_INVALID_MEDIA;
       }

       bool got_frame = media->video.read(img);
       if (!got_frame) {
           delete media->filename; // Delete media memory before returning an error
           delete media;
           return JANICE_INVALID_MEDIA;
       }
       media->frames = (uint32_t) media->video.get(CV_CAP_PROP_FRAME_COUNT);

       // Reset the video to the start
       media->video.set(CV_CAP_PROP_POS_FRAMES, 0);
   }

   // Set the media dimensions
   media->channels = (uint32_t) img.channels();
   media->rows     = (uint32_t) img.rows;
   media->cols     = (uint32_t) img.cols;

   return JANICE_SUCCESS;
}

JaniceError janice_free_media(JaniceMedia* media)
{
    delete (*media)->filename;
    delete (*media);
    *media = nullptr;

    return JANICE_SUCCESS;
}
