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

#include "janice.h"

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

/*!
 * \brief File name for a Janice Metadata File
 *
 * A *Janice Metadata File* is a *Comma-Separated Value* (CSV) text file with the following format:
 *
\verbatim
TEMPLATE_ID        , SUBJECT_ID, FILE_NAME, MEDIA_ID, FRAME, <janice_attribute>, <janice_attribute>, ..., <janice_attribute>
<janice_template_id>, <int>     , <string> , <int>   , <int>, <double>         , <double>         , ..., <double>
<janice_template_id>, <int>     , <string> , <int>   , <int>, <double>         , <double>         , ..., <double>
...
<janice_template_id>, <int>     , <string> , <int>   , <int>, <double>         , <double>         , ..., <double>
\endverbatim
 *
 * Where:
 * - [TEMPLATE_ID](\ref janice_template_id) is a unique integer identifier indicating rows that belong to the same template.
 * - \c SUBJECT_ID is a unique integer identifier used to establish ground truth match/non-match.
 *      For the purpose of experimentation, multiple \c TEMPLATE_ID may have the same \c SUBJECT_ID.
 * - \c FILE_NAME is a path to the image or video file on disk.
 * - \c MEDIA_ID is a unique integer identifier indicating rows that belong to the same piece of media (image or video clip).
 * - \c FRAME is the video frame number and -1 (or empty string) for still images.
 * - \a \<janice_attribute\> adheres to \ref janice_enum.
 * - All rows associated with the same \c TEMPLATE_ID occur sequentially.
 * - All rows associated with the same \c TEMPLATE_ID and \c FILE_NAME occur sequentially ordered by \c FRAME.
 * - A cell is empty when no value is available for the specified #janice_attribute.
 */
typedef const char *janice_metadata;

/*!
 * \brief High-level function for enrolling templates from a metadata file and writing templates to disk.
 * \param [in] data_path Prefix path to files in metadata.
 * \param [in] metadata #janice_metadata to enroll.
 * \param [in] output_prefix Prefix of a directory to save the templates into. Templates will be saved using their templateID
 * \param [in] verbose Print information and warnings during gallery enrollment.
 */
JANICE_EXPORT janice_error janice_create_templates(const char *data_path, janice_metadata metadata, const char *output_prefix, int verbose);

/*!
 * \brief High-level function for enrolling a gallery from a metadata file.
 * \param [in] data_path Prefix path to files in metadata.
 * \param [in] metadata #janice_metadata to enroll.
 * \param [in] gallery File to save the templates to.
 * \param [in] verbose Print information and warnings during gallery enrollment.
 */
JANICE_EXPORT janice_error janice_create_gallery(const char *data_path, janice_metadata metadata, janice_gallery gallery, int verbose);

/*!
 * \brief A statistic.
 * \see janice_metrics
 */
struct janice_metric
{
    size_t count;  /*!< \brief Number of samples. */
    double mean;   /*!< \brief Sample average. */
    double stddev; /*!< \brief Sample standard deviation. */
};

/*!
 * \brief All statistics.
 * \see janice_get_metrics
 */
struct janice_metrics
{
    struct janice_metric janice_initialize_template_speed; /*!< \brief ms */
    struct janice_metric janice_detection_speed; /*!< \brief ms */
    struct janice_metric janice_augment_speed; /*!< \brief ms */
    struct janice_metric janice_finalize_template_speed; /*!< \brief ms */
    struct janice_metric janice_read_image_speed; /*!< \brief ms */
    struct janice_metric janice_free_image_speed; /*!< \brief ms */
    struct janice_metric janice_verify_speed; /*!< \brief ms */
    struct janice_metric janice_search_speed; /*!< \brief ms */
    struct janice_metric janice_gallery_size_speed; /*!< \brief ms */
    struct janice_metric janice_finalize_gallery_speed; /*!< \brief ms */
    struct janice_metric janice_template_size; /*!< \brief KB */
    int          janice_missing_attributes_count; /*!< \brief Count of \ref JANICE_MISSING_ATTRIBUTES */
    int          janice_failure_to_detect_count; /*! \brief Count of \ref JANICE_FAILURE_TO_DETECT */
    int          janice_failure_to_enroll_count; /*!< \brief Count of \ref JANICE_FAILURE_TO_ENROLL */
    int          janice_other_errors_count; /*!< \brief Count of \ref janice_error excluding \ref JANICE_MISSING_ATTRIBUTES, \ref JANICE_FAILURE_TO_ENROLL, and \ref JANICE_SUCCESS */
};

/*! \brief Retrieve and reset performance metrics. */
JANICE_EXPORT struct janice_metrics janice_get_metrics();

/*!
 * \brief Print metrics to stdout.
 * \note Will only print metrics with count > 0 occurrences.
 */
JANICE_EXPORT void janice_print_metrics(struct janice_metrics metrics);

/*! @}*/

/*!
 * \page janice_enum Enum Naming Convention
 * #janice_attribute, #janice_color_space, #janice_error and enum values follow a
 * \c CAPITAL_CASE naming convention. Functions #janice_attribute_to_string and
 * #janice_error_to_string return a string for the corresponding enum by
 * removing the leading \c janice_:
 * \code
 * janice_attribute_to_string(janice_RIGHT_EYE_X); // returns "RIGHT_EYE_X"
 * \endcode
 * Functions #janice_attribute_from_string and #janice_error_from_string provide
 * the opposite functionality:
 * \code
 * janice_attribute_from_string("RIGHT_EYE_X"); // returns janice_RIGHT_EYE_X
 * \endcode
 * \note #janice_attribute_from_string is used to decode #janice_metadata
 * files, so attribute column names should follow this naming convention.
 */

#ifdef __cplusplus
}
#endif

#endif /* IARPA_janice_IO_H */
