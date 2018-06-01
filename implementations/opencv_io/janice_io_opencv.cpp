#include <janice_io_opencv.h>
#include <janice_io_opencv_utils.hpp>

#include <opencv2/highgui/highgui.hpp>

namespace
{

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
static JaniceError initialize_media_iterator(JaniceMediaIterator& it, cv::Mat& img)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it._internal;

    state->initialized = true;

    /*
     * Attempt imread. We use ANYCOLOR to load either RGB or Grayscale images.
     * IGNORE_ORIENTATION will not rotate the image based on EXIF data.
     */
    img = cv::imread(state->filename, cv::IMREAD_ANYCOLOR | cv::IMREAD_IGNORE_ORIENTATION); //

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

JaniceError is_video(JaniceMediaIterator* it, bool* video)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    *video = state->video.isOpened();

    return JANICE_SUCCESS;
}

JaniceError get_frame_rate(JaniceMediaIterator* it, float* frame_rate)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->video.isOpened())
        return JANICE_INVALID_MEDIA;

    *frame_rate = state->video.get(CV_CAP_PROP_FPS);

    return JANICE_SUCCESS;
}

JaniceError get_physical_frame_rate(JaniceMediaIterator* it, float* frame_rate)
{
    return get_frame_rate(it, frame_rate);
}

JaniceError next(JaniceMediaIterator* it, JaniceImage* image)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    // if we are currently at the end, just return.
    if (state->at_end)
        return JANICE_MEDIA_AT_END;

    if (!state->initialized) {
        // are we an image or a video?
        cv::Mat buffer;
        JaniceError rc = initialize_media_iterator(*it, buffer);

        // we do an initial read on images, but not videos.
        // if this is a still (video not opened by init), just convert the
        // image we read to the output type, and return.
        if (!state->video.isOpened()) {
            JaniceError conv_rc = ocv_utils::cv_mat_to_janice_image(buffer, *image);
            return conv_rc;
        }

        if (rc != JANICE_SUCCESS)
            return rc; // got an error code on init, just return.
    }

    // Check if the media is an image, if so just read it
    // this can happen if: we do the initial read, then seek 0 on the still
    // which unsets at_end. 
    if (!state->video.isOpened()) {
        cv::Mat cv_image = cv::imread(state->filename, cv::IMREAD_ANYCOLOR | cv::IMREAD_IGNORE_ORIENTATION);
        ocv_utils::cv_mat_to_janice_image(cv_image, *image);
        state->at_end = true;
	
        return JANICE_SUCCESS;
    }

    // video - a little bit more complicated
    cv::Mat cv_frame;
    // try to read a frame, could error out
    if (!state->video.read(cv_frame)) 
        return JANICE_INVALID_MEDIA;

    // convert the frame we got to the output type.
    JaniceError ret = ocv_utils::cv_mat_to_janice_image(cv_frame, *image);
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
JaniceError seek(JaniceMediaIterator* it, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(*it, useless);
    	if (rc != JANICE_SUCCESS)
            return rc;
    }
    
    // Image - return INVALID_MEDIA
    if (!state->video.isOpened()) {
      // taa: Allow seek to 0 even on images. It used to work, and there's no reason why it shouldn't now.
      if (frame != 0) {
        return JANICE_INVALID_MEDIA;
      }
      state->at_end = false;
      return JANICE_SUCCESS;
    }
    
    if (frame >= state->video.get(CV_CAP_PROP_FRAME_COUNT)) // invalid index
        return JANICE_OUT_OF_BOUNDS_ACCESS;

    // Set the video to the desired frame
    state->video.set(CV_CAP_PROP_POS_FRAMES, frame);
    state->at_end = false;

    return JANICE_SUCCESS;
}

// get the specified frame. This is a stateless operation so it resets the
// frame position after the get.
JaniceError get(JaniceMediaIterator* it, JaniceImage* image, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(*it, useless);
        if (rc != JANICE_SUCCESS)
            return rc;
    }

    uint32_t curr_frame = 0;
    // Image - return INVALID_MEDIA
    if (!state->video.isOpened()) {
      if (frame != 0) {
        return JANICE_INVALID_MEDIA;
      }
    }
    else {
      curr_frame = (uint32_t) state->video.get(CV_CAP_PROP_POS_FRAMES);
    }

    // First we seek
    JaniceError ret = seek(it, frame);
    if (ret != JANICE_SUCCESS) {
        return ret; // This leaves the iterator in an undefined state
    }
    // Then we get the frame
    ret = next(it, image);
    if (ret != JANICE_SUCCESS) {
        return ret; // This leaves the iterator in an undefined state
    }
    // Then we reset the position
    return seek(it, curr_frame);
}

// say what frame we are currently on.
JaniceError tell(JaniceMediaIterator* it, uint32_t* frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (!state->initialized) {
        cv::Mat useless;
        JaniceError rc = initialize_media_iterator(*it, useless);
    	if (rc != JANICE_SUCCESS)
	      return rc;
    }
    
    if (!state->video.isOpened()) // image, getting fnum is not supported
        return JANICE_INVALID_MEDIA;

    *frame = (uint32_t) state->video.get(CV_CAP_PROP_POS_FRAMES);
    return JANICE_SUCCESS;
}

// Map a logical frame number (as from tell) to a physical frame number, allowing
// for downsampling, clipping, etc. on videos. Here, we just return the physical frame.
JaniceError physical_frame(JaniceMediaIterator*, uint32_t logical, uint32_t *physical)
{
    if (physical == nullptr) {
        return JANICE_BAD_ARGUMENT;
    }

    *physical = logical;
    return JANICE_SUCCESS;
}

JaniceError free_image(JaniceImage* image)
{
    if (image && image->owner) {
        free(image->data);
    }

    return JANICE_SUCCESS;
}

JaniceError free_iterator(JaniceMediaIterator* it)
{
    if (it && it->_internal) {
        delete (JaniceMediaIteratorStateType*) it->_internal;
    }

    return JANICE_SUCCESS;
}

JaniceError reset(JaniceMediaIterator* it)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    // If not initialized the next use of the class will initialize so don't need to reset
    if (!state->initialized) {
        return JANICE_SUCCESS;
    }

    if (!state->video.isOpened()) {
        state->at_end = false; // Reload the image next time
        return JANICE_SUCCESS;
    }

    return seek(it, 0); // Reset the video to the first frame
}

} // anoymous namespace

// ----------------------------------------------------------------------------
// OpenCV I/O only, create an opencv_io media iterator 

JaniceError janice_io_opencv_create_media_iterator(const char* filename, JaniceMediaIterator* it)
{
    it->is_video = &is_video;
    it->get_frame_rate =  &get_frame_rate;
    it->get_physical_frame_rate =  &get_physical_frame_rate;

    it->next = &next;
    it->seek = &seek;
    it->get  = &get;
    it->tell = &tell;
    it->physical_frame = &physical_frame;

    it->free_image = &free_image;
    it->free       = &free_iterator;

    it->reset      = &reset;

    JaniceMediaIteratorStateType* state = new JaniceMediaIteratorStateType();
    state->initialized = false;
    state->filename = filename;
    state->at_end = false;

    it->_internal = (void*) (state);

    return JANICE_SUCCESS;
}
