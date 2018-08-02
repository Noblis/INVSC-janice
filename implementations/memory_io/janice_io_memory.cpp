#include <janice_io_memory.h>
#include <janice_io_memory_utils.hpp>

namespace
{

struct JaniceMediaIteratorStateType
{
    JaniceImage image;
    bool at_end;
};


JaniceError is_video(JaniceMediaIterator*, bool* video)
{
    *video = false;
    return JANICE_SUCCESS;
}

JaniceError get_frame_rate(JaniceMediaIterator*, float*)
{
    return JANICE_INVALID_MEDIA;
}

JaniceError get_physical_frame_rate(JaniceMediaIterator*, float*)
{
    return JANICE_INVALID_MEDIA;
}

JaniceError next(JaniceMediaIterator* it, JaniceImage* image)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;

    if (state->at_end) {
        return JANICE_MEDIA_AT_END;
    }

    JaniceError ret = mem_utils::copy_janice_image(state->image, *image);
    if (ret == JANICE_SUCCESS) { // Don't mark finished unless the copy succeeded
        state->at_end = true;
    }

    return ret;
}

// seek to the specified frame number
JaniceError seek(JaniceMediaIterator* it, uint32_t frame)
{
    if (frame != 0) {
        return JANICE_INVALID_MEDIA;
    }

    return it->reset(it);
}

// get the specified frame. This is a stateless operation so it resets the
// frame position after the get.
JaniceError get(JaniceMediaIterator* it, JaniceImage* image, uint32_t frame)
{
    if (frame != 0) {
        return JANICE_INVALID_MEDIA;
    }

    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    return mem_utils::copy_janice_image(state->image, *image);
}

// say what frame we are currently on.
JaniceError tell(JaniceMediaIterator*, uint32_t*)
{
    return JANICE_INVALID_MEDIA;
}

// Map a logical frame number (as from tell) to a physical frame number, allowing
// for downsampling, clipping, etc. on videos. Here, we just return the physical frame.
JaniceError physical_frame(JaniceMediaIterator*, uint32_t logical, uint32_t *physical)
{
    if (physical == NULL) {
        return JANICE_BAD_ARGUMENT;
    }

    *physical = logical;
    return JANICE_SUCCESS;
}

JaniceError free_image(JaniceImage* image)
{
    if (image && image->owner) {
        free(image->data);
        image->data = nullptr;
    }

    return JANICE_SUCCESS;
}

JaniceError free_iterator(JaniceMediaIterator* it)
{
    if (it && it->_internal) {
        JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
        free_image(&state->image);
    }

    return JANICE_SUCCESS;
}

JaniceError reset(JaniceMediaIterator* it)
{
    JaniceMediaIteratorStateType* state = (JaniceMediaIteratorStateType*) it->_internal;
    state->at_end = false;
}

} // anoymous namespace

// ----------------------------------------------------------------------------
// OpenCV I/O only, create an opencv_io media iterator 

JaniceError janice_io_memory_create_media_iterator(const JaniceImage* image, JaniceMediaIterator* it)
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
    JaniceError ret = mem_utils::copy_janice_image(*image, state->image);
    if (ret != JANICE_SUCCESS) {
        return ret;
    }

    it->_internal = (void*) (state);

    return JANICE_SUCCESS;
}
