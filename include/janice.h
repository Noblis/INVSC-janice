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

#ifndef JANICE_H
#define JANICE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \mainpage
 * \section overview Overview
 *
 * *libjanice* is a *C* API that makes integration easier between face recognition
 * algorithms and agencies and entities that would like to use them. The API consists
 * of two header files:
 *
 * Header       | Documentation   | Required               | Description
 * ------------ | --------------- | ---------------------- | -----------
 * janice.h     | \ref janice     | **Yes**                | \copybrief janice
 * janice_io.h  | \ref janice_io  | No (Provided)          | \copybrief janice_io
 *
 * - [<b>Source Code</b>](https://github.com/Noblis/janice) [github.com]
 *
 * \subsection about About
 * Facial recognition has emerged as a key technology for government agencies to
 * efficiently triage and analyze large data streams. A large ecosystem of
 * facial recognition algorithms already exists from a variety of sources including
 * commercial vendors, government programs and academia. However, integrating
 * this important technology into existing technology stacks is a difficult and
 * expensive endeavor. The JanICE API aims to address this problem by functioning
 * as a compatibility layer between users and the algorithms. Users can write their
 * applications on "top" of the API while algorithm providers will implement their
 * algorithms "beneath" the API. This means that users can write their applications
 * independent of any single FR algorithm and gives them the freedom to select the
 * algorithm or algorithms that best serve their specific use case without worrying
 * about integration. Algorithm providers will be able to serve their algorithms
 * across teams and agencies without having to integrate with the different tools
 * and services of each specific team.
 *
 * \subsection license License
 * The API is provided under a [BSD-like license](LICENSE.txt) and is
 * *free for academic and commercial use*.
 */

#if defined JANICE_LIBRARY
#  if defined _WIN32 || defined __CYGWIN__
#    define JANICE_EXPORT __declspec(dllexport)
#  else
#    define JANICE_EXPORT __attribute__((visibility("default")))
#  endif
#else
#  if defined _WIN32 || defined __CYGWIN__
#    define JANICE_EXPORT __declspec(dllimport)
#  else
#    define JANICE_EXPORT
#  endif
#endif

#define JANICE_VERSION_MAJOR 2
#define JANICE_VERSION_MINOR 1
#define JANICE_VERSION_PATCH 1

/*!
 * \defgroup janice JanICE
 * \brief Mandatory interface for the JanICE API.
 *
 * \section Overview
 * A JanICE application begins with a call to \ref janice_initialize.
 * New templates are constructed with \ref janice_allocate_template and provided
 * image data with \ref janice_detect followed by \ref janice_augment.
 * Templates are finalized prior to comparison with \ref janice_finalize_template,
 * and freed after finalization with \ref janice_free_template. Templates can be
 * stored on and loaded from disk using the \ref janice_write_template and \ref
 * janice_read_template respectively.
 *
 * Finalized templates can be used for verification with \ref janice_verify, or
 * search with \ref janice_search.
 * Galleries are managed with \ref janice_allocate_gallery, \ref janice_enroll,
 * \ref janice_remove_template and \ref janice_free_gallery.
 *
 * A JanICE application ends with a call to \ref janice_finalize.
 *
 * \section thread_safety Thread Safety
 * All functions are marked one of:
 * - \anchor thread_safe \par thread-safe
 *   Can be called simultaneously from multiple threads, even when the
 *   invocations use shared data.
 * - \anchor reentrant \par reentrant
 *   Can be called simultaneously from multiple threads, but only if each
 *   invocation uses its own data.
 * - \anchor thread_unsafe \par thread-unsafe
 *   Can not be called simultaneously from multiple threads.
 *
 * \section implementer_notes Implementer Notes
 * Define \c JANICE_LIBRARY during compilation to export JanICE symbols.
 *
 * \addtogroup janice
 * @{
 */

/*!
 * \brief Return type for functions that indicate an error status.
 *
 * All error values are positive integers, with the exception of #JANICE_SUCCESS
 * = 0 which indicates no error.
 */
typedef enum janice_error
{
    JANICE_SUCCESS         = 0, /*!< No error */
    JANICE_UNKNOWN_ERROR      , /*!< Catch-all error code */
    JANICE_OUT_OF_MEMORY      , /*!< Memorry allocation failed */
    JANICE_INVALID_SDK_PATH   , /*!< Incorrect location provided to
                                     #janice_initialize */
    JANICE_OPEN_ERROR         , /*!< Failed to open a file */
    JANICE_READ_ERROR         , /*!< Failed to read from a file */
    JANICE_WRITE_ERROR        , /*!< Failed to write to a file */
    JANICE_PARSE_ERROR        , /*!< Failed to parse file */
    JANICE_INVALID_IMAGE      , /*!< Could not decode image file */
    JANICE_INVALID_VIDEO      , /*!< Could not decode video file */
    JANICE_MISSING_TEMPLATE_ID, /*!< Expected a missing template ID */
    JANICE_MISSING_FILE_NAME  , /*!< Expected a missing file name */
    JANICE_NULL_ATTRIBUTES    , /*!< Null #janice_attributes */
    JANICE_MISSING_ATTRIBUTES , /*!< Not all required attributes were
                                    provided */
    JANICE_FAILURE_TO_DETECT  , /*!< Could not localize a face within the
                                    provided image */
    JANICE_FAILURE_TO_ENROLL  , /*!< Could not construct a template from the
                                    provided image and attributes */
    JANICE_NOT_IMPLEMENTED    , /*!< Optional functions may return this value in
                                    lieu of a meaninful implementation */
    JANICE_NUM_ERRORS           /*!< Idiom to iterate over all errors */
} janice_error;

/*!
 * \brief Data buffer type.
 */
typedef uint8_t janice_data;

/*!
 * \brief Supported image formats.
 */
typedef enum janice_color_space
{
    JANICE_GRAY8, /*!< \brief 1 channel grayscale, 8-bit depth. */
    JANICE_BGR24  /*!< \brief 3 channel color (BGR order), 8-bit depth. */
} janice_color_space;

/*!
 * \brief Common representation for still images and video frames.
 *
 * Pixels are stored in _row-major_ order.
 * In other words, pixel layout with respect to decreasing memory spatial
 * locality is \a channel, \a column, \a row.
 * Thus pixel intensity can be retrieved as follows:
 *
\code
janice_data get_intensity(janice_image image, size_t channel, size_t column,
                                                                     size_t row)
{
    const size_t columnStep = (image.image_format == JANICE_COLOR ? 3 : 1);
    const size_t index = row*image.step + column*columnStep + channel;
    return image.data[index];
}
\endcode
 *
 * Coordinate (0, 0) corresponds to the top-left corner of the image.
 * Coordinate (width-1, height-1) corresponds to the bottom-right corner of the image.
 */
typedef struct janice_image
{
    janice_data *data; /*!< \brief Data buffer. */
    size_t width;     /*!< \brief Column count in pixels. */
    size_t height;    /*!< \brief Row count in pixels. */
    size_t step;      /*!< \brief Bytes per row, including padding. */
    janice_color_space color_space; /*!< \brief Arrangement of #data. */
} janice_image;

/*!
 * \brief Attributes for a particular object in an image.
 *
 * Attributes associated with an object.
 * Values of \c NaN ([isnan](http://www.cplusplus.com/reference/cmath/isnan/))
 * indicate that the attribute value is unknown.
 *
 * \section Below is a list of attributes common to general facial recognition
 * algorithms. Implementors can choose to use none, some or all, as they see fit.
 * The only exception is the face location attributes which are required. These
 * are enumerated in \ref janice_detect. The attributes list is subject to change
 * until at least Version 2.3 of this API. If there is a particular field you
 * would like to add please open a pull request on the
 * [<b>Github page</b>](https://github.com/Noblis/janice/pulls).
 */
typedef struct janice_attributes
{
    double detection_confidence; /*!< \brief A higher value indicates greater
                                             detection confidence. */
    double face_x; /*!< \brief Horizontal offset to top-left corner of face
                               (pixels) */
    double face_y; /*!< \brief Vertical offset to top-left corner of face
                               (pixels) */
    double face_width; /*!< \brief Face horizontal size (pixels) */
    double face_height; /*!< \brief Face vertical size (pixels) */
    double right_eye_x; /*!< \brief Face landmark (pixels) */
    double right_eye_y; /*!< \brief Face landmark (pixels) */
    double left_eye_x; /*!< \brief Face landmark (pixels) */
    double left_eye_y; /*!< \brief Face landmark (pixels) */
    double nose_base_x; /*!< \brief Face landmark (pixels) */
    double nose_base_y; /*!< \brief Face landmark (pixels) */
    double face_yaw; /*!< \brief Face yaw estimation (degrees). */
    double gender; /*!< \brief Gender of subject of interest, 1 for male, 0 for
                        female. */
    double age; /*!< \brief Approximate age of subject (years) */
    double skin_tone; /*!< \brief Skin tone of subject */
    double frame_rate; /*!< \brief Frames per second, or 0 for images. */
} janice_attributes;

/*!
 * \brief Call once at the start of the application, before making any other
 * calls to the API.
 *
 * \param[in] sdk_path Path to the \em read-only directory containing the
 *                     janice-compliant SDK as provided by the implementer.
 * \param[in] temp_path Path to an existing empty \em read-write directory for
 *                      use as temporary file storage by the implementation.
 *                      This path is guaranteed until \ref janice_finalize.
 * \param[in] algorithm An empty string indicating the default algorithm, or an
 *                      implementation-defined string indicating an alternative
 *                      configuration.
 * \param[in] gpu_index An integer index indicating to the implementer what GPU
 *                      should be used. The index will be 0 or greater if a GPU
 *                      is available, and negative otherwise. Implementers who
 *                      do not require a GPU can ignore this value.
 * \remark This function is \ref thread_unsafe and should only be called once.
 * \see janice_finalize
 */
JANICE_EXPORT janice_error janice_initialize(const char *sdk_path,
                                             const char *temp_path,
                                             const char *algorithm,
                                             const int gpu_index);

/*!
 * \brief Call once at the end of the application, after making all other calls
 * to the API.
 * \remark This function is \ref thread_unsafe and should only be called once.
 * \see janice_initialize
 */
JANICE_EXPORT janice_error janice_finalize();

/*!
 * \brief Detect objects in a #janice_image.
 *
 * Each object is represented by a #janice_attributes. In the case that the
 * number of detected objects is greater than \p num_requested, the
 * implementation may choose which detections to exclude, potentially returning
 * early before scanning the entire image. Detected objects can then be used in
 * \ref janice_augment.
 *
 * \section detection_guarantees Detection Guarantees
 * The first \p num_actual elements of \p attributes_array will be populated by
 * decreasing janice_attributes::detection_confidence.
 *
 * Each of the \p num_actual detections will have values for at least the
 * following attributes:
 *  - janice_attributes::detection_confidence
 *  - janice_attributes::face_x
 *  - janice_attributes::face_y
 *  - janice_attributes::face_width
 *  - janice_attributes::face_height
 *
 * Any attribute of the \p num_actual detections without a value will be set to
 * \c NaN.
 *
 * \param[in] image Image to detect objects in.
 * \param[out] attributes_array Pre-allocated array of uninitialized
 *                              #janice_attributes. Expected to be at least
 *                              \p num_requested * \c sizeof(#janice_attributes)
 *                              bytes long.
 * \param[in] num_requested Length of \p attributes_array.
 * \param[out] num_actual The number of detections made by the system. If
 *                        \p num_actual <= \p num_requested, then \p num_actual
 *                        is the length of \p attributes_array populated with
 *                        detected objects by the implementation. Otherwise,
 *                        \p num_actual > \p num_requested, then
 *                        \p attributes_array is fully populated and there were
 *                        additional detections that weren't returned.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT janice_error janice_detect(const janice_image image,
                                         janice_attributes *attributes_array,
                                         const size_t num_requested,
                                         size_t *num_actual);

/*!
 * \brief Contains the recognition information for an object.
 *
 * Create a new template with \ref janice_allocate_template.
 */
typedef struct janice_template_type *janice_template;

/*!
 * \brief Allocate memory for an empty template.
 *
 * Memory is managed by the implementation and guaranteed until
 * \ref janice_free_template.
 *
 * Add images to the template with \ref janice_augment.
 *
 * \code
 * janice_template template_;
 * janice_error error = janice_allocate_template(&template_);
 * assert(!error);
 * \endcode
 *
 * \param[in] template_ An uninitialized template.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_allocate_template(janice_template *template_);

/*!
 * \brief The maximum number of images that can be
 *        enrolled to a single template.
 *
 * If there is no limit on the number of images
 * that can be enrolled to a template, this function
 * should return -1.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT size_t janice_max_images_per_template();

/*!
 * \brief Add an image to the template.
 *
 * The \p attributes should be provided from a prior call to \ref janice_detect.
 *
 * This function may write to \p attributes, reflecting additional information
 * gained during augmentation.
 *
 * Augmented templates should be passed to \ref janice_finalize_template when
 * no more imagery needs to be added.
 *
 * \param[in] image The image containing the detected object to be recognized.
 * \param[in,out] attributes Location and metadata associated with a single
 *                          detected object to recognize.
 * \param[in,out] template_ The template to contain the object's recognition
 *                          information.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_augment(const janice_image image,
                                          janice_attributes *attributes,
                                          janice_template template_);

/*!
 * \brief Create a finalized template representation for \ref janice_verify,
 *        \ref janice_write_gallery or \ref janice_search.
 *
 * After this function is called no more images will be added or removed
 * from the template.
 * \param[in, out] template_ The template to finalize.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_finalize_template(janice_template template_);

/*!
 * \brief Templates are represented in persistent storage as files on disk.
 *
 * A \ref janice_template_path is the path to the template folder.
 * Templates are written by \ref janice_write_template and read with
 * \ref janice_read_template. Templates are only written after a call
 * to \ref janice_finalize_template.
 */
typedef const char *janice_template_path;

/*!
 * \brief Write a template to disk.
 *
 * Templates can be read using \ref janice_read_template. Templates are
 * only written after being finalized with \ref janice_finalize_template
 * \param[in] template_ The template to write to disk
 * \param[in] template_path The location of a file on disk to write the
 *                          template to.
 * \remark This function is \ref reentrant
 * \see janice_read_template
 */
JANICE_EXPORT janice_error janice_write_template(const janice_template template_,
                                                 janice_template_path template_path);

/*!
 * \brief Read a template from disk.
 *
 * Templates are written using \ref janice_write_template. Templates that
 * are loaded have already been finalized. The template is already allocated
 * using \ref janice_allocate_template.
 * \param[in] template_ An initialized template
 * \param[in] template_path The location of a file on disk to load the
 *                          template from.
 * \remark This function is \ref reentrant
 * \see janice_write_template
 */
JANICE_EXPORT janice_error janice_read_template(janice_template template_,
                                                janice_template_path template_path);

/*!
 * \brief Free memory for a template previously allocated by
 * \ref janice_allocate_template.
 *
 * Call this function on a template after it is no longer needed.
 * \param[in] template_ The template to deallocate.
 * \remark This function is \ref reentrant.
 * \see janice_allocate_template
 */
 JANICE_EXPORT janice_error janice_free_template(janice_template template_);

/*!
 * \brief Return a similarity score for two finalized templates.
 *
 * Higher scores indicate greater similarity.
 *
 * The returned \p similarity score is \em symmetric. In other words, swapping
 * the order of \p a and \p b will not change \p similarity.
 *
 * \param[in] a The first template to compare.
 * \param[in] b The second template to compare.
 * \param[out] similarity Higher values indicate greater similarity.
 * \remark This function is \ref thread_safe.
 * \see janice_search
 */
JANICE_EXPORT janice_error janice_verify(const janice_template a,
                                         const janice_template b,
                                         float *similarity);

/*!
 * \brief Unique identifier for a \ref janice_template.
 *
 * Associate a template with a unique identifier during
 * \ref janice_enroll.
 * Retrieve the unique identifier from \ref janice_search and \ref janice_cluster.
 */
typedef size_t janice_template_id;

/*!
 * \brief A collection of \ref janice_template%s.
 *
 * Initialize with \ref janice_allocate_gallery and free with
 * \ref janice_free_gallery.
 * Used to perform searches with \ref janice_search and clustering with
 * \ref janice_cluster.
 * Galleries can be stored on and loaded from disk with \ref
 * janice_write_gallery and \ref janice_read_gallery respectively.
 */
typedef struct janice_gallery_type *janice_gallery;

/*!
 * \brief Allocate memory for an empty gallery.
 *
 * Memory is managed by the implementation and guaranteed until
 * \ref janice_free_gallery.
 *
 * Add templates to the gallery with \ref janice_enroll.
 * Remove templates from the gallery with \ref janice_remove_template.
 *
 * \code
 * janice_gallery gallery;
 * janice_error error = janice_allocate_gallery(&gallery);
 * assert(!error);
 * \endcode
 *
 * \param[in] gallery An uninitialized gallery.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_allocate_gallery(janice_gallery *gallery);

/*!
 * \brief Add a template to the gallery.
 *
 * The \p template_ will be finalized before being added to the gallery.
 *
 * \param[in] template_ The finalized template with recognition information
 * \param[out] template_id The unique id for the template. This will be assigned
 *                         by the gallery during this call.
 * \param[in,out] gallery The gallery to add the template to
 * \remark This function is \ref reentrant.
 * \see janice_remove_template
 */
JANICE_EXPORT janice_error janice_enroll(const janice_template template_,
                                         janice_template_id *template_id,
                                         janice_gallery gallery);

/*!
 * \brief Remove a template from a gallery
 *
 * The template to remove is identified by its unique \p template_id
 * \param[in] template_id The unique id of the template to be removed
 * \param[in,out] gallery The gallery to remove the template from
 * \remark This function is \ref reentrant
 * \see janice_enroll
 */
JANICE_EXPORT janice_error janice_remove_template(const janice_template_id template_id,
                                                  janice_gallery gallery);
/*!
 * \brief Galleries are represented in persistent storage as folders on disk.
 *
 * A \ref janice_gallery_path is the path to the gallery folder.
 * Galleries are written with \ref janice_write_gallery and read with
 * \ref janice_read_gallery.
 */
typedef const char *janice_gallery_path;

/*!
 * \brief Write a gallery to a directory on disk.
 *
 * Access the constructed gallery with \ref janice_read_gallery.
 *
 * \param[in] gallery Initialized gallery
 * \param[in] gallery_path Path to an empty read-write folder to store the
 *                         gallery.
 * \remark This function is \ref reentrant.
 * \see janice_read_gallery
 */
JANICE_EXPORT janice_error janice_write_gallery(const janice_gallery gallery,
                                                janice_gallery_path gallery_path);

/*!
 * \brief Read a gallery from a directory on disk.
 *
 * Read a gallery constructed with \ref janice_write_gallery. The gallery
 * has been inititialized with \ref janice_allocate_gallery.
 *
 * \param[in] gallery Initialized gallery
 * \param[in] gallery_path Path to folder containing stored gallery.
 * \remark This function is \ref reentrant.
 * \see janice_write_gallery
 */
JANICE_EXPORT janice_error janice_read_gallery(janice_gallery gallery,
                                               janice_gallery_path gallery_path);

/*!
 * \brief Free a gallery previously initialized by \ref janice_allocate_gallery.
 *
 * \param[in] gallery The gallery to free.
 * \remark This function is \ref reentrant.
 */
 JANICE_EXPORT janice_error janice_free_gallery(janice_gallery gallery);

/*!
 * \brief Ranked search for a template against a gallery.
 *
 * \p template_ids and \p similarities should be pre-allocated buffers large
 * enough to contain \p requested_returns elements. \p actual_returns will be
 * less than or equal to requested_returns, depending on the contents of the
 * gallery.
 *
 * The returned \p similarities \em may be normalized by the implementation
 * based on the contents of the \p gallery. Therefore, similarity scores
 * returned from searches against different galleries are \em not guaranteed to
 * be comparable.
 *
 * \param[in] probe Probe to search for.
 * \param[in] gallery Gallery to search against.
 * \param[in] num_requested_returns The desired number of returned results.
 * \param[out] template_ids Buffer to contain the \ref janice_template_id of the
 *                          top matching gallery templates.
 * \param[out] similarities Buffer to contain the similarity scores of the top
 *                          matching templates.
 * \param[out] num_actual_returns The number of populated elements in
 *                                template_ids and similarities. This value
 *                                could be zero.
 * \remark This function is \ref thread_safe.
 * \see janice_verify
 */
JANICE_EXPORT janice_error janice_search(const janice_template probe,
                                         const janice_gallery gallery,
                                         const size_t num_requested_returns,
                                         janice_template_id *template_ids,
                                         float *similarities,
                                         size_t *num_actual_returns);

/*!
 * \brief Cluster a gallery into a set of identities.
 *
 * The output of this function is two arrays, \p template_ids and \p cluster_ids
 * of equal length, serving as a mapping between templates and clusters.
 *
 * \section clustering_hint Clustering Hint
 * Clustering is generally considered to be an ill-defined problem, and most
 * algorithms require some help determining the appropriate number of clusters.
 * The \p hint parameter helps influence the number of clusters, though the
 * implementation is free to ignore it.
 * - If \p hint is in the range [-1, 1] then it is a clustering
 * \em aggressiveness, with \c -1 favoring more clusters (fewer templates per
 * cluster), and \c 1 favoring fewer clusters (more templates per cluster).
 * - If \p hint is greater than 1 then it is a clustering \em count, indicating
 * the suggested number of clusters.
 * - The suggested default value for \p hint is \c 0.
 *
 * \section gallery_size Gallery Size
 * The size of the gallery is the number of templates that have been enrolled
 *
 *
 * \param[in] gallery The gallery to cluster.
 * \param[in] hint A hint to the clustering algorithm, see \ref clustering_hint.
 * \param[out] template_ids A pre-allocated array provided by the calling
 *                          application large enough to hold \ref gallery_size
 *                          elements.
 * \param[out] cluster_ids A pre-allocated array provided by the calling
 *                         application large enough to hold \ref gallery_size
 *                         elements.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT janice_error janice_cluster(const janice_gallery gallery,
                                          const double hint,
                                          janice_template_id *template_ids,
                                          int *cluster_ids);
/*! @}*/

#ifdef __cplusplus
}
#endif

#endif /* JANICE_H */
