#include <janice_io_opencv.h>

#include <opencv2/highgui/highgui.hpp>

// ----------------------------------------------------------------------------
// JaniceMediaIterator

struct JaniceMediaIteratorStateType
{
    std::string filename;

    // was this object already initialized
    bool initialized;
    // are we currently at the end of this object
    // behavior is: return success on last frame, return media_at_end on
    // subsequent reads, don't cycle.
    bool at_end;

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

    // if this succeeds, we are confident the file is just an image and can
    // return immediately.
    if (img.data) {
        // still images are considered @ the end after the first read.
        state->at_end = true;
        return JANICE_SUCCESS;
    }

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

    image->data = (uint8_t*) malloc(m.channels() * m.rows * m.cols);
    memcpy(image->data, m.data, m.channels() * m.rows * m.cols);
    image->owner = true;

    *_image = image;

    return JANICE_SUCCESS;
}

JaniceError next(JaniceMediaIterator it, JaniceImage* image)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    // if we are currently at the end, just return.
    if (state->at_end)
        return JANICE_MEDIA_AT_END;

    if (!state->initialized) {
        // are we an image or a video?
        cv::Mat buffer;
        JaniceError rc = initialize_media_iterator(it, buffer);

        // we do an initial read on images, but not videos.
        // if this is a still (video not opened by init), just convert the
        // image we read to the output type, and return.
        if (!state->video.isOpened()) {
            JaniceError conv_rc = cv_mat_to_janice_image(buffer, image);
            return conv_rc;
        }

        if (rc != JANICE_SUCCESS)
            return rc; // got an error code on init, just return.
    }

    // Check if the media is an image, if so just read it
    // this can happen if: we do the initial read, then seek 0 on the still
    // which unsets at_end. 
    if (!state->video.isOpened()) {
        cv::Mat cv_image = cv::imread(state->filename, cv::IMREAD_ANYCOLOR);
        cv_mat_to_janice_image(cv_image, image);
        state->at_end = true;
	
        return JANICE_SUCCESS;
    }

    // video - a little bit more complicated
    cv::Mat cv_frame;
    // try to read a frame, could error out
    if (!state->video.read(cv_frame)) 
        return JANICE_INVALID_MEDIA;

    // convert the frame we got to the output type.
    JaniceError ret = cv_mat_to_janice_image(cv_frame, image);
    // could fail the conversion...
    if (ret != JANICE_SUCCESS)
        return ret;

    // If we're at the last frame of the video, mark at_end so that 
    // subsequent reads end early with JANICE_MEDIA_AT_END
    if (state->video.get(CV_CAP_PROP_POS_FRAMES) == state->video.get(CV_CAP_PROP_FRAME_COUNT)) {
        state->at_end = true;
    }

    return JANICE_SUCCESS;
}

// seek to the specified frame number
JaniceError seek(JaniceMediaIterator it, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(it, useless);
    	if (rc != JANICE_SUCCESS)
            return rc;
    }
    
    if (!state->video.isOpened()) {
        // for stills, we still allow seek(0) as an idiom for reset
        if (frame != 0)
            return JANICE_OUT_OF_BOUNDS_ACCESS;
        else {
            state->at_end = false;
            return JANICE_SUCCESS;
        }
    }
    
    if (frame >= state->video.get(CV_CAP_PROP_FRAME_COUNT)) // invalid index
        return JANICE_OUT_OF_BOUNDS_ACCESS;

    // Set the video to the desired frame
    state->video.set(CV_CAP_PROP_POS_FRAMES, frame);
    state->at_end = false;

    return JANICE_SUCCESS;
}

// get the specified frame (aka seek + next)
JaniceError get(JaniceMediaIterator it, JaniceImage* image, uint32_t frame)
{
    // don't need to check it->initialized, seek will do it.

    // First we seek
    JaniceError ret = seek(it, frame);
    if (ret != JANICE_SUCCESS)
        return ret;

    // Then we get the frame
    return next(it, image);
}

// say what frame we are currently on.
JaniceError tell(JaniceMediaIterator it, uint32_t* frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(it, useless);
    	if (rc != JANICE_SUCCESS)
	      return rc;
    }
    
    if (!state->video.isOpened()) // image, getting fnum is not supported
        return JANICE_INVALID_MEDIA;

    *frame = (uint32_t) state->video.get(CV_CAP_PROP_POS_FRAMES);
    return JANICE_SUCCESS;
}

JaniceError free_image(JaniceImage* image)
{
    if ((*image)->owner)
        free((*image)->data);
    delete (*image);

    return JANICE_SUCCESS;
}

JaniceError free_iterator(JaniceMediaIterator* it)
{
    delete (JaniceMediaIteratorStateType*) (*it)->_internal;
    delete (*it);
    *it = nullptr;

    return JANICE_SUCCESS;
}

// ----------------------------------------------------------------------------
// OpenCV I/O only, create an opencv_io media iterator 

JaniceError janice_io_opencv_create_media_iterator(const char* filename, JaniceMediaIterator* _it)
{
    JaniceMediaIterator it = new JaniceMediaIteratorType();

    it->next = &next;
    it->seek = &seek;
    it->get  = &get;
    it->tell = &tell;

    it->free_image = &free_image;
    it->free       = &free_iterator;

    JaniceMediaIteratorStateType* state = new JaniceMediaIteratorStateType();
    state->initialized = false;
    state->filename = filename;
    state->at_end = false;

    it->_internal = (void*) (state);

    *_it = it;

    return JANICE_SUCCESS;
}
