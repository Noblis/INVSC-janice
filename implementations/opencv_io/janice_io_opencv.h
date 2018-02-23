#ifndef JANICE_IO_OPENCV_H
#define JANICE_IO_OPENCV_H

#include <janice_io.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Create a JaniceMediaIterator from a file on disk
 * \param filename A null-terminated path to an image or video or disk. The file must be readable.
 * \param it A pointer to an unallocated JaniceMediaIterator. The iterator is allocated by
 *        this function
 * \returns JANICE_SUCCESS if the iterator is created successfully. Otherwise returns
 *      error code.
 */
JANICE_EXPORT JaniceError janice_io_opencv_create_media_iterator(const char* filename,
                                                                 JaniceMediaIterator* it);

/*!
 * \brief Create a sparse iterator over a selection of video frames.
 *
 * This creates a single media iterator over a collection of image files on disk. Typically,
 * the images would have been extracted from a single video, for example as key frames.
 * \param filenames An array of null-terminated filenames. Each file must be readable.
 * \param frames An array of frame numbers associated with the given images. The images
 *        will be stored in the iterator in increasing frame order.
 * \param num_files The number of elements in *filenames* and *frames*.
 * \param it A pointer to an unallocated JaniceMediaIterator. The iterator is allocated by
 *        this function.
 * \returns JANICE_SUCCESS if the iterator is created successfully. Otherwise returns
 *          an error code.
 */
JANICE_EXPORT JaniceError janice_io_opencv_create_sparse_media_iterator(const char** filenames,
                                                                        size_t num_files,
                                                                        JaniceMediaIterator* it);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_IO_OPENCV_H

