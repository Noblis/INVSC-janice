#include <janice_io_opencv.h>

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

struct JaniceMediaIteratorStateType
{
    std::string filename;
    bool initialized;

    cv::VideoCapture video;
};


// initialize the iterator, for still images, img will be filled in, for
// videos it will not, for videos the video capture (video) will be open
static inline JaniceError initialize_media_iterator(JaniceMediaIterator it, cv::Mat& img)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    state->initialized = true;

    // attempt imread
    img = cv::imread(state->filename, cv::IMREAD_ANYCOLOR); // We use ANYCOLOR to load either RGB or Grayscale images
    if (img.data)
        return JANICE_MEDIA_AT_END;

    // Couldn't load as an image maybe it's a video
    state->video.open(state->filename);
    if (!state->video.isOpened()) // couldn't open as a video either? error out
        return JANICE_OPEN_ERROR;

    return JANICE_SUCCESS;
}

static inline JaniceError cv_mat_to_janice_image(cv::Mat& m, JaniceImage* _image)
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

JaniceError next(JaniceMediaIterator it, JaniceImage* image)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        // are we an image or a video?
        cv::Mat buffer;
        JaniceError rc = initialize_media_iterator(it, buffer);

        // we do an initial read on images, but not videos.
        // if video is not opened, and we returned media_at_end (always
        // do this for stills), convert output and return.
        if (!state->video.isOpened() && rc == JANICE_MEDIA_AT_END) {
            JaniceError conv_rc = cv_mat_to_janice_image(buffer, image);
            if (conv_rc != JANICE_SUCCESS)
                return conv_rc;
            return rc;
        }

        if (rc != JANICE_SUCCESS && rc != JANICE_MEDIA_AT_END)
            return rc; // just return an error code, we have no valid output
    }

    // Check if the media is an image
    if (!state->video.isOpened()) {
        cv::Mat cv_image = cv::imread(state->filename, cv::IMREAD_ANYCOLOR);
        cv_mat_to_janice_image(cv_image, image);
	
        return JANICE_MEDIA_AT_END;
    }

    // video - a little bit more complicated
    cv::Mat cv_frame;
    if (!state->video.read(cv_frame)) // Something went unexpectedly wrong (maybe a corrupted video?).
        return JANICE_INVALID_MEDIA;

    JaniceError ret = cv_mat_to_janice_image(cv_frame, image);
    if (ret != JANICE_SUCCESS)
        return ret;

    // end of the video. Reset it and return JANICE_MEDIA_AT_END
    if (state->video.get(CV_CAP_PROP_POS_FRAMES) == state->video.get(CV_CAP_PROP_FRAME_COUNT)) {
        state->video.set(CV_CAP_PROP_POS_FRAMES, 0);
        return JANICE_MEDIA_AT_END;
    }

    return JANICE_SUCCESS;
}

JaniceError seek(JaniceMediaIterator it, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(it, useless);
    	if (rc != JANICE_SUCCESS && rc != JANICE_MEDIA_AT_END)
	        return rc;
    }
    
    if (!state->video.isOpened()) // image
        return JANICE_INVALID_MEDIA;
    
    if (frame >= state->video.get(CV_CAP_PROP_FRAME_COUNT)) // invalid index
        return JANICE_OUT_OF_BOUNDS_ACCESS;

    // Set the video to the desired frame
    state->video.set(CV_CAP_PROP_POS_FRAMES, frame);

    return JANICE_SUCCESS;
}

JaniceError get(JaniceMediaIterator it, JaniceImage* image, uint32_t frame)
{
    // don't need to check it->initialized, seek will do it.

    // First we seek
    JaniceError ret = seek(it, frame);
    if (ret != JANICE_SUCCESS)
        return ret;

    // Then we get the frame
    ret = next(it, image);
    if (ret != JANICE_SUCCESS)
        return ret;

    return JANICE_SUCCESS;
}

JaniceError tell(JaniceMediaIterator it, uint32_t* frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(it, useless);
    	if (rc != JANICE_SUCCESS && rc != JANICE_MEDIA_AT_END)
	      return rc;
    }
    
    if (!state->video.isOpened()) // image
        return JANICE_INVALID_MEDIA;

    *frame = (uint32_t) state->video.get(CV_CAP_PROP_POS_FRAMES);
    return JANICE_SUCCESS;
}

// ----------------------------------------------------------------------------
// OpenCV I/O

JaniceError janice_io_opencv_create_media_iterator(const char* filename, JaniceMediaIterator* _it)
{
    JaniceMediaIterator it = new JaniceMediaIteratorType();

    it->next = &next;
    it->seek = &seek;
    it->get  = &get;
    it->tell = &tell;

    JaniceMediaIteratorStateType* state = new JaniceMediaIteratorStateType();
    state->initialized = false;
    state->filename = filename;
    
    it->_internal = (void*) (state);

    *_it = it;

    return JANICE_SUCCESS;
}

JaniceError janice_io_opencv_free_media_iterator(JaniceMediaIterator *it)
{
    delete (JaniceMediaIteratorStateType*) (*it)->_internal;
    delete (*it);
    *it = nullptr;

    return JANICE_SUCCESS;
}
