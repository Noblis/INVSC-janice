#include <janice_io.h>

#include <opencv2/highgui.hpp>

#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
JANICE_EXPORT JaniceError janice_io_opencv_create_sparse_media_iterator(const char** filenames,
                                                                        size_t num_files,
                                                                        JaniceMediaIterator* it);

#ifdef __cplusplus
} // extern "C"
#endif



namespace
{

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

// ----------------------------------------------------------------------------
// JaniceMediaIterator

struct JaniceMediaIteratorStateType
{
    std::vector<std::string> filenames;
    size_t pos;
};

JaniceError is_video(JaniceMediaIterator it, bool* video)
{
    *video = true; // Treat this as a video
    return JANICE_SUCCESS;
}

JaniceError get_frame_rate(JaniceMediaIterator, float*)
{
    return JANICE_INVALID_MEDIA;
}

JaniceError get_physical_frame_rate(JaniceMediaIterator it, float* frame_rate)
{
    return get_frame_rate(it, frame_rate);
}

JaniceError next(JaniceMediaIterator it, JaniceImage* image)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (state->pos == state->filenames.size())
        return JANICE_MEDIA_AT_END;

    try {
        cv::Mat cv_img = cv::imread(state->filenames[state->pos], cv::IMREAD_ANYCOLOR | cv::IMREAD_IGNORE_ORIENTATION);
        cv_mat_to_janice_image(cv_img, image);
    } catch (...) {
        return JANICE_UNKNOWN_ERROR;
    }

    ++state->pos;

    return JANICE_SUCCESS;
}

JaniceError seek(JaniceMediaIterator it, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (frame >= state->filenames.size())
        return JANICE_BAD_ARGUMENT;

    state->pos = frame;

    return JANICE_SUCCESS;
}

JaniceError get(JaniceMediaIterator it, JaniceImage* image, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (frame >= state->filenames.size())
        return JANICE_BAD_ARGUMENT;

    try {
        cv::Mat cv_img = cv::imread(state->filenames[frame], cv::IMREAD_ANYCOLOR | cv::IMREAD_IGNORE_ORIENTATION);
        cv_mat_to_janice_image(cv_img, image);
    } catch (...) {
        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError tell(JaniceMediaIterator it, uint32_t* frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (state->pos == state->filenames.size())
        return JANICE_MEDIA_AT_END;

    *frame = state->pos;

    return JANICE_SUCCESS;
}

// Map a logical frame number (as from tell) to a physical frame number, allowing
// for downsampling, clipping, etc. on videos. Here, we just return the physical frame.
JaniceError physical_frame(JaniceMediaIterator it, uint32_t logical, uint32_t *physical)
{
  if (physical == nullptr) {
    return JANICE_BAD_ARGUMENT;
  }
  *physical = logical;
  return JANICE_SUCCESS;
}

JaniceError free_image(JaniceImage* image)
{
    if (image && (*image)->owner)
        free((*image)->data);
    delete (*image);

    return JANICE_SUCCESS;
}

JaniceError free_iterator(JaniceMediaIterator* it)
{
    if (it && (*it)->_internal) {
        delete (JaniceMediaIteratorStateType*) (*it)->_internal;
        delete (*it);
        *it = nullptr;
    }

    return JANICE_SUCCESS;
}

JaniceError reset(JaniceMediaIterator it)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    state->pos = 0;

    return JANICE_SUCCESS;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// OpenCV I/O only, create a sparse opencv_io media iterator

JaniceError janice_io_opencv_create_sparse_media_iterator(const char** filenames, size_t num_files, JaniceMediaIterator *_it)
{
    JaniceMediaIterator it = new JaniceMediaIteratorType();

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
    for (size_t i = 0; i < num_files; ++i) {
        state->filenames.push_back(std::string(filenames[i]));
    }
    state->pos = 0;

    it->_internal = (void*) (state);

    *_it = it;

    return JANICE_SUCCESS;
}
