#include <janice_io_memory.h>
#include <janice_io_memory_utils.hpp>

#include <vector>

namespace
{

struct JaniceMediaIteratorStateType
{
    std::vector<JaniceImage> images;
    size_t pos;
};

JaniceError is_video(JaniceMediaIterator* it, bool* video)
{
    *video = true; // Treat this as a video
    return JANICE_SUCCESS;
}

JaniceError get_frame_rate(JaniceMediaIterator*, float*)
{
    return JANICE_INVALID_MEDIA;
}

JaniceError get_physical_frame_rate(JaniceMediaIterator* it, float* frame_rate)
{
    return get_frame_rate(it, frame_rate);
}

JaniceError next(JaniceMediaIterator* it, JaniceImage* image)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (state->pos == state->images.size()) {
        return JANICE_MEDIA_AT_END;
    }

    JaniceError ret = mem_utils::copy_janice_image(state->images[state->pos], *image);
    if (ret == JANICE_SUCCESS) {
        ++state->pos;
    }

    return ret;
}

JaniceError seek(JaniceMediaIterator* it, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (frame >= state->images.size()) {
        return JANICE_BAD_ARGUMENT;
    }

    state->pos = frame;

    return JANICE_SUCCESS;
}

JaniceError get(JaniceMediaIterator* it, JaniceImage* image, uint32_t frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (frame >= state->images.size()) {
        return JANICE_BAD_ARGUMENT;
    }

    return mem_utils::copy_janice_image(state->images[frame], *image);
}

JaniceError tell(JaniceMediaIterator* it, uint32_t* frame)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (state->pos == state->images.size()) {
        return JANICE_MEDIA_AT_END;
    }

    *frame = state->pos;

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
        JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
        for (JaniceImage& image : state->images) {
            free_image(&image);
        }

        delete (JaniceMediaIteratorStateType*) it->_internal;
    }

    return JANICE_SUCCESS;
}

JaniceError reset(JaniceMediaIterator* it)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    state->pos = 0;

    return JANICE_SUCCESS;
}

} // anonymous namespace

// ----------------------------------------------------------------------------
// OpenCV I/O only, create a sparse opencv_io media iterator

JaniceError janice_io_memory_create_sparse_media_iterator(const JaniceImage* images, size_t num_images, JaniceMediaIterator* it)
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
    for (size_t i = 0; i < num_images; ++i) {
        JaniceImage image;
        JaniceError ret = mem_utils::copy_janice_image(images[i], image);
        if (ret != JANICE_SUCCESS) { // Try and clean up
            for (size_t i = 0; i < state->images.size(); ++i) {
                free_image(&state->images[i]);
            }
            delete state;

            return ret;
        }

        state->images.push_back(image);
    }
    state->pos = 0;

    it->_internal = (void*) (state);

    return JANICE_SUCCESS;
}
