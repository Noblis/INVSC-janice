/*******************************************************************************
 * Copyright (c) 2013 Noblis, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 ******************************************************************************/

#ifndef JANICE_IO_H
#define JANICE_IO_H

#include <janice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \defgroup janice_io JanICE I/O
 * \brief Media decoding and evaluation harness.
 * \addtogroup janice_io
 *  @{
 */

/*!
 * \brief The \c JANICE_ASSERT macro provides a simple unrecoverable error
 *        handling mechanism.
 * \see JANICE_CHECK
 */
#define JANICE_ASSERT(EXPRESSION)                                     \
{                                                                    \
    const janice_error error = (EXPRESSION);                          \
    if (error != JANICE_SUCCESS) {                                    \
        fprintf(stderr, "Janice error: %s\n\tFile: %s\n\tLine: %d\n", \
                janice_error_to_string(error),                        \
                __FILE__,                                            \
                __LINE__);                                           \
        abort();                                                     \
    }                                                                \
}                                                                    \

/*!
 * \brief The \c JANICE_CHECK macro provides a simple recoverable error handling
 *        mechanism.
 * \see JANICE_ASSERT
 */
#define JANICE_CHECK(EXPRESSION)             \
{                                           \
    const janice_error error = (EXPRESSION); \
    if (error != JANICE_SUCCESS)             \
        return error;                       \
}                                           \

/*!
 * \brief #janice_error to string.
 * \param[in] error Error code to stringify.
 * \note Memory for the return value is static and should not be freed.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT const char *janice_error_to_string(janice_error error);

/*!
 * \brief #janice_error from string.
 * \param[in] error String to decode.
 * \remark This function is \ref thread_safe.
 * \see janice_enum
 */
JANICE_EXPORT janice_error janice_error_from_string(const char *error);

/*!
 * \brief Read an image from disk.
 * \param[in] file_name Path to the image file.
 * \param[out] image Address to store the decoded image.
 * \remark This function is \ref reentrant.
 * \see janice_free_image
 */
JANICE_EXPORT janice_error janice_read_image(const char *file_name, janice_image *image);

/*!
 * \brief Frees the memory previously allocated for a #janice_image.
 * \param[in] image #janice_image to free.
 * \remark This function is \ref reentrant.
 * \see janice_allocate_image
 */
JANICE_EXPORT void janice_free_image(janice_image image);

/*!
 * \brief Handle to a private video decoding type.
 */
typedef struct janice_video_type *janice_video;

/*!
 * \brief Returns a video ready for reading.
 * \param[in] file_name Path to image file.
 * \param[out] video Address to store the allocated video.
 * \remark This function is \ref reentrant.
 * \see janice_read_frame janice_close_video
 */
JANICE_EXPORT janice_error janice_open_video(const char *file_name, janice_video *video);

/*!
 * \brief Returns the current frame and advances the video to the next frame.
 * \param[in] video Video to decode.
 * \param[out] image Address to store the allocated image.
 * \remark This function is \ref reentrant.
 * \see janice_open_video janice_free_image
 */
JANICE_EXPORT janice_error janice_read_frame(janice_video video, janice_image *image);

/*!
 * \brief Closes a video previously opened by \ref janice_open_video.
 *
 * Call this function to deallocate the memory allocated to decode the video
 * after the desired frames have been read.
 * \param[in] video The video to close.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT void janice_close_video(janice_video video);

#ifdef __cplusplus
}
#endif

#endif /* IARPA_JANUS_IO_H */
