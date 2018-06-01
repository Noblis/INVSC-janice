#ifndef JANICE_IO_MEMORY_H
#define JANICE_IO_MEMORY_H

#include <janice_io.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Create a JaniceMediaIterator from a memory buffer
 * \param image A pointer to a JaniceImage containing a buffer. The buffer is 
 *        copied into the iterator and can be safely deleted after this call
 * \param it A pointer to an unallocated JaniceMediaIterator. The iterator is allocated by
 *        this function
 * \returns JANICE_SUCCESS if the iterator is created successfully. Otherwise returns
 *      error code.
 */
JANICE_EXPORT JaniceError janice_io_memory_create_media_iterator(const JaniceImage* image,
                                                                 JaniceMediaIterator* it);

/*!
 * \brief Create a sparse iterator over a selection of video frames.
 *
 * This creates a single media iterator over a collection of images. Typically,
 * the images would have been extracted from a single video, for example as key frames.
 * \param images An array of JaniceImage structs. Each struct is copied into the iterator.
 * \param num_images The number of elements in *images*.
 * \param it A pointer to an unallocated JaniceMediaIterator. The iterator is allocated by
 *        this function.
 * \returns JANICE_SUCCESS if the iterator is created successfully. Otherwise returns
 *          an error code.
 */
JANICE_EXPORT JaniceError janice_io_memory_create_sparse_media_iterator(const JaniceImage* images,
                                                                        size_t num_images,
                                                                        JaniceMediaIterator* it);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_IO_MEMORY_H

