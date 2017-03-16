#include <janice_io.h>

#include <opencv2/highgui/highgui.hpp>

// ----------------------------------------------------------------------------
// JaniceImage

JaniceError janice_image_access(JaniceConstImage image,
                                uint32_t channel,
                                uint32_t row,
                                uint32_t col,
                                uint8_t* value)
{
    if (channel >= image->channels ||
        row >= image->rows ||
        col >= image->cols)
        return JANICE_OUT_OF_BOUNDS_ACCESS;

    const uint32_t step = image->cols * image->channels;
    *value = image->data[row * step + col * image->channels + channel];

    return JANICE_SUCCESS;
}

JaniceError janice_free_image(JaniceImage* image)
{
    if ((*image)->owner) { // have to delete the data
        free((*image)->data);
        (*image)->data = nullptr;
    }

    delete *image;
    *image = nullptr;

    return JANICE_SUCCESS;
}

// ----------------------------------------------------------------------------
// JaniceMediaIterator

using namespace cv;

struct JaniceMediaIteratorType
{
    std::string filename;
    VideoCapture video;
};

static inline JaniceError cv_mat_to_janice_image(Mat& m, JaniceImage* _image)
{
    // Allocate a new image
    JaniceImage image = new JaniceImageType();

    // Set up the dimensions
    image->channels = m.channels();
    image->rows = m.rows;
    image->cols = m.cols;

    image->data = (JaniceBuffer) malloc(m.channels() * m.rows * m.cols);
    memcpy(image->data, m.data, m.channels() * m.rows * m.cols);
    image->owner = true;

    *_image = image;

    return JANICE_SUCCESS;
}

JaniceError janice_media_it_next(JaniceMediaIterator it,
                                 JaniceImage* image)
{
    // Check if the media is an image
    if (!it->video.isOpened()) {
        Mat cv_image = imread(it->filename, IMREAD_ANYCOLOR);
        JaniceError ret = cv_mat_to_janice_image(cv_image, image);
        if (ret != JANICE_SUCCESS)
            return ret;
        return JANICE_MEDIA_AT_END;
    }

    // video - a little bit more complicated
    Mat cv_frame;
    if (!it->video.read(cv_frame)) // Something went unexpectedly wrong (maybe a corrupted video?).
        return JANICE_INVALID_MEDIA;

    JaniceError ret = cv_mat_to_janice_image(cv_frame, image);
    if (ret != JANICE_SUCCESS)
        return ret;

    // end of the video. Reset it and return JANICE_MEDIA_AT_END
    if (it->video.get(CV_CAP_PROP_POS_FRAMES) == it->video.get(CV_CAP_PROP_FRAME_COUNT)) {
        it->video.set(CV_CAP_PROP_POS_FRAMES, 0);
        return JANICE_MEDIA_AT_END;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_media_it_seek(JaniceMediaIterator it,
                                 uint32_t frame)
{
    if (!it->video.isOpened()) // image
        return JANICE_INVALID_MEDIA;

    if (frame >= it->video.get(CV_CAP_PROP_FRAME_COUNT)) // invalid index
        return JANICE_OUT_OF_BOUNDS_ACCESS;

    // Set the video to the desired frame
    it->video.set(CV_CAP_PROP_POS_FRAMES, frame);

    return JANICE_SUCCESS;
}

JaniceError janice_media_it_get(JaniceMediaIterator it,
                                JaniceImage* image,
                                uint32_t frame)
{
    // First we seek
    JaniceError ret = janice_media_it_seek(it, frame);
    if (ret != JANICE_SUCCESS)
        return ret;

    // Then we get the frame
    ret = janice_media_it_next(it, image);
    if (ret != JANICE_SUCCESS)
        return ret;

    return JANICE_SUCCESS;
}

JaniceError janice_media_it_tell(JaniceMediaIterator it,
                                 uint32_t* frame)
{
    if (!it->video.isOpened()) // image
        return JANICE_INVALID_MEDIA;

    *frame = (uint32_t) it->video.get(CV_CAP_PROP_POS_FRAMES);
    return JANICE_SUCCESS;
}

JaniceError janice_free_media_iterator(JaniceMediaIterator *it)
{
    delete (*it);
    *it = nullptr;

    return JANICE_SUCCESS;
}

// ----------------------------------------------------------------------------
// JaniceMedia

// Utility function to manually count video frames. Necessary because
// video.get(CV_CAP_PROP_FRAME_COUNT) seems to fail in some cases
static inline uint32_t count_frames(cv::VideoCapture& video)
{
    // Reset the video to the beginning
    video.set(CV_CAP_PROP_POS_FRAMES, 0);

    cv::Mat frame;
    uint32_t frame_count = 0;
    while (video.read(frame))
        ++frame_count;

    // Reset the video again
    video.set(CV_CAP_PROP_POS_FRAMES, 0);

    return frame_count;
}

JaniceError janice_create_media(const char* filename,
                                JaniceMedia* _media)
{
    JaniceMedia media = new JaniceMediaType();

    // Copy over the filename
    media->filename = new char[strlen(filename) + 1];
    strcpy(media->filename, filename);
    media->filename[strlen(filename)] = '\0';

    // Initialize these to the image defaults to avoid an extra condition later
    media->category = Image;
    media->frames   = 1;

    // To get the media dimensions we temporalily load either the image or the
    // the first frame. This also checks if the filename is valid
    Mat img = imread(filename, IMREAD_ANYCOLOR); // We use ANYCOLOR to load either RGB or Grayscale images
    if (!img.data) { // Couldn't load as an image maybe it's a video
        VideoCapture video(filename);
        if (!video.isOpened()) {
            janice_free_media(&media);
            return JANICE_OPEN_ERROR;
        }

        bool got_frame = video.read(img);
        if (!got_frame) {
            janice_free_media(&media);
            return JANICE_INVALID_MEDIA;
        }

        media->category = Video;
        media->frames   = count_frames(video);
    }

    // Set the media dimensions
    media->channels = (uint32_t) img.channels();
    media->rows     = (uint32_t) img.rows;
    media->cols     = (uint32_t) img.cols;

    *_media = media;

    return JANICE_SUCCESS;
}

JaniceError janice_media_get_iterator(JaniceConstMedia media, JaniceMediaIterator *_it)
{
    JaniceMediaIterator it = new JaniceMediaIteratorType();

    it->filename = media->filename;
    if (media->category == Video)
        it->video.open(media->filename);

    *_it = it;

    return JANICE_SUCCESS;
}

JaniceError janice_free_media(JaniceMedia* media)
{
    delete [] (*media)->filename;
    delete (*media);
    *media = nullptr;

    return JANICE_SUCCESS;
}
