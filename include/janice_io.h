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

/*!
 * \defgroup janice_io Janice I/O
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
JANICE_EXPORT janice_error janice_load_media(const std::string &filename, janice_media &media);

/*!
 * \brief Frees the memory previously allocated for a #janice_image.
 * \param[in] image #janice_image to free.
 * \remark This function is \ref reentrant.
 * \see janice_allocate_image
 */
JANICE_EXPORT janice_error janice_free_media(janice_media &media);

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
 * - All rows associated with the same \c TEMPLATE_ID occur sequentially.
 * - All rows associated with the same \c TEMPLATE_ID and \c FILE_NAME occur sequentially ordered by \c FRAME.
 * - A cell is empty when no value is available for the specified attribute.
 *
 * \par Examples:
 * - [meds.csv](https://raw.githubusercontent.com/biometrics/janice/master/data/meds.csv)
 * - [Kirchner.csv](https://raw.githubusercontent.com/biometrics/janice/master/data/Kirchner.csv)
 * - [Toledo.csv](https://raw.githubusercontent.com/biometrics/janice/master/data/Toledo.csv)
 */
typedef const char *janice_metadata;

/*!
 * \brief High-level helper function for enrolling templates from a metadata file and writing templates to disk.
 * \param [in] data_path Prefix path to files in metadata.
 * \param [in] metadata #janice_metadata to enroll.
 * \param [in] output_path Directory to save the templates to
 * \param [in] output_file CSV file to hold the filenames, template ids, and subject_ids for the saved templates.
 *                         The format is templateID,subjectID,filename\n
 * \param [in] role The role for the templates
 * \param [in] verbose Print information and warnings during gallery enrollment.
 * \remark This function is \ref thread_unsafe.
 */
JANICE_EXPORT janice_error janice_create_templates_helper(const std::string &data_path, janice_metadata metadata, const std::string &templates_path, const std::string &templates_list_file, const janice_template_role role, bool verbose);

/*!
 * \brief High-level helper function for enrolling a gallery from a metadata file.
 * \param [in] templates_file Text file of templates to enroll into the gallery
 * \param [in] gallery_file File to save the gallery to
 * \param [in] verbose Print information and warnings during gallery enrollment.
 * \remark This function is \ref thread_unsafe.
 */
JANICE_EXPORT janice_error janice_create_gallery_helper(const std::string &templates_list_file, const std::string &gallery_file, bool verbose);

/*!
 * \brief High-level helper function for running verification on two equal sized lists of templates
 * \param [in] templates_list_file_a The first list of templates
 * \param [in] templates_list_file_b The second list of templates
 * \param [in] scores_file The file to write scores to. Scores are written template_id_a,template_id_b,similarity,genuine_match\n
 * \param [in] verbose Print information and warnings during verification.
 * \remark This function is \ref thread_unsafe.
 */
JANICE_EXPORT janice_error janice_verify_helper(const std::string &templates_list_file_a, const std::string &templates_list_file_b, const std::string &scores_file, bool verbose);

/*!
 * \brief High-level helper function for running verification on two equal sized lists of templates
 * \param [in] templates_list_file List of templates to enroll as probes
 * \param [in] gallery_file Path to the gallery to search against
 * \param [in] candidate_list File to write the candidate lists to. Each line in the file has the format probe_template_id,rank,gallery_template_id,similarity,genuine_match\n
 * \param [in] verbose Print information and warnings during search.
 * \remark This function is \ref thread_unsafe.
 */
JANICE_EXPORT janice_error janice_search_helper(const std::string &probes_list_file, const std::string &gallery_list_file, const std::string &gallery_file, int num_requested_returns, const std::string &candidate_list, bool verbose);

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
    struct janice_metric janice_load_media_speed; /*!< \brief ms */
    struct janice_metric janice_free_media_speed; /*!< \brief ms */
    struct janice_metric janice_detection_speed; /*!< \brief ms */
    struct janice_metric janice_create_template_speed; /*!< \brief ms */
    struct janice_metric janice_serialize_template_speed; /*!< \brief ms */
    struct janice_metric janice_deserialize_template_speed; /*!< \brief ms */
    struct janice_metric janice_delete_serialized_template_speed; /*!< \brief ms */
    struct janice_metric janice_delete_template_speed; /*!< \brief ms */
    struct janice_metric janice_verify_speed; /*!< \brief ms */
    struct janice_metric janice_create_gallery_speed; /*!< \brief ms */
    struct janice_metric janice_gallery_insert_speed; /*!< \brief ms */
    struct janice_metric janice_gallery_remove_speed; /*!< \brief ms */
    struct janice_metric janice_serialize_gallery_speed; /*!< \brief ms */
    struct janice_metric janice_deserialize_gallery_speed; /*!< \brief ms */
    struct janice_metric janice_delete_serialized_gallery_speed; /*!< \brief ms */
    struct janice_metric janice_delete_gallery_speed; /*!< \brief ms */
    struct janice_metric janice_search_speed; /*!< \brief ms */

    struct janice_metric janice_gallery_size; /*!< \brief KB */
    struct janice_metric janice_template_size; /*!< \brief KB */
    int                 janice_missing_attributes_count; /*!< \brief Count of
                                                             \ref JANICE_MISSING_ATTRIBUTES */
    int                 janice_failure_to_detect_count; /*!< \brief Count of
                                                            \ref JANICE_FAILURE_TO_DETECT */
    int                 janice_failure_to_enroll_count; /*!< \brief Count of
                                                            \ref JANICE_FAILURE_TO_ENROLL */
    int                 janice_other_errors_count; /*!< \brief Count of \ref janice_error excluding
                                                       \ref JANICE_MISSING_ATTRIBUTES,
                                                       \ref JANICE_FAILURE_TO_ENROLL, and
                                                       \ref JANICE_SUCCESS */
};

/*!
 * \brief Retrieve and reset performance metrics.
 * \remark This function is \ref thread_unsafe.
 */
JANICE_EXPORT struct janice_metrics janice_get_metrics();

/*!
 * \brief Print metrics to stdout.
 * \note Will only print metrics with count > 0 occurrences.
 * \remark This function is \ref thread_unsafe.
 */
JANICE_EXPORT void janice_print_metrics(struct janice_metrics metrics);

/*! @}*/

#endif /* JANICE_IO_H */
