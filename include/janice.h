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

#include <vector>
#include <string>

/*!
 * \mainpage
 * \section overview Overview
 *
 * *libjanice* is a *C++* API that makes integration easier between face recognition
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
#define JANICE_VERSION_MINOR 2
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
enum JaniceError
{
    JANICE_SUCCESS         = 0, /*!< No error */
    JANICE_UNKNOWN_ERROR      , /*!< Catch-all error code */
    JANICE_OUT_OF_MEMORY      , /*!< Memorry allocation failed */
    JANICE_INVALID_SDK_PATH   , /*!< Incorrect location provided to
                                    #janice_initialize */
    JANICE_BAD_SDK_CONFIG     , /*!< Incorrect SDK configuration for
                                     attempted operation */
    JANICE_BAD_LICENSE        , /*!< Bad license file */
    JANICE_MISSING_DATA       , /*!< Missing SDK data */
    JANICE_INVALID_GPU        , /*!< GPU is invalid or not working */
    JANICE_OPEN_ERROR         , /*!< Failed to open a file */
    JANICE_READ_ERROR         , /*!< Failed to read from a file */
    JANICE_WRITE_ERROR        , /*!< Failed to write to a file */
    JANICE_PARSE_ERROR        , /*!< Failed to parse file */
    JANICE_INVALID_MEDIA      , /*!< Could not decode media file */
    JANICE_MISSING_TEMPLATE_ID, /*!< Expected a missing template ID */
    JANICE_MISSING_FILE_NAME  , /*!< Expected a missing file name */
    JANICE_INCORRECT_ROLE     , /*!< Incorrect template role for the function */
    JANICE_FAILURE_TO_ENROLL  , /*!< Could not construct a template from the
                                     provided image and attributes */
    JANICE_FAILURE_TO_SERIALIZE, /*!< Could not serialize a template or
                                      gallery */
    JANICE_FAILURE_TO_DESERIALIZE, /*!< Could not deserialize a template
                                        or gallery */
    JANICE_NOT_SET            , /*!< Accessor functions called on fields that do
                                     not have data set, but could have data in the
                                     future if additional processing was completed
                                     may return this value. */
    JANICE_NOT_IMPLEMENTED    , /*!< Optional functions may return this value in
                                     lieu of a meaninful implementation */
    JANICE_NUM_ERRORS         /*!< Idiom to iterate over all errors */
};

/*!
 * \brief Data buffer type.
 */
typedef uint8_t JaniceData;

/*!
 * \brief Supported image formats.
 */
enum JaniceColorSpace
{
    JANICE_GRAY8, /*!< \brief 1 channel grayscale, 8-bit depth. */
    JANICE_BGR24  /*!< \brief 3 channel color (BGR order), 8-bit depth. */
};

/*!
 * \brief Common representation for images and videos.
 *
 * Pixels are stored in _row-major_ order.
 * In other words, pixel layout with respect to decreasing memory spatial
 * locality is \a channel, \a column, \a row \a frames.
 *
 * Coordinate (0, 0) corresponds to the top-left corner of the image.
 * Coordinate (width-1, height-1) corresponds to the bottom-right corner of a frame in the media.
 *
 * Data within an image or a video frame is contiguous. Data between frames is not guaranteed to
 * be contiguous.
 */
struct JaniceMedia
{
    std::vector<JaniceData *> data; /*! < \brief A collection of image data of size N,
                                                 where is the number of frames in a video
                                                 or 1 in the case of a still image. */
    uint32_t width;     /*!< \brief Column count in pixels. */
    uint32_t height;    /*!< \brief Row count in pixels. */
    double frame_rate;  /*!< \brief If the media is an image this should be 0. */

    JaniceColorSpace color_space; /*!< \brief Arrangement of #data. */
};

/*!
 * \brief A rectangle
 */
struct JaniceRect
{
    uint32_t x; /*!< X coordinate of the rect */
    uint32_t y; /*!< Y coordinate of the rect */
    uint32_t width; /*!< Width of the rect */
    uint32_t height; /*!< Height of the rect */
};

/*!
 * \brief A point
 */
struct JanicePoint
{
    uint32_t x; /*!< X coordinate of the point */
    uint32_t y; /*!< Y coordinate of the point */
};

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
 * \param[in] gpu_dev The GPU device number to be used by all subsequent
 * 			          implementation function calls
 * \remark This function is \ref thread_unsafe and should only be called once.
 * \see janice_finalize
 */
JANICE_EXPORT JaniceError janice_initialize(const std::string &sdk_path,
                                            const std::string &temp_path,
                                            const std::string &algorithm,
                                            const int gpu_dev);

/*!
 * \brief Called once before template generation and gallery construction
 *
 * \param[in] path A path to the training images on disk
 * \param[in] csv Comma-delimited metadata file giving data and possible metadata
 * \remark Implementors must handle the case where csv = "" (i.e there is no training data provided)
 */
JANICE_EXPORT JaniceError janice_train(const std::string &path, const std::string &csv = "");

/*!
 * \brief Contains the detection information for an object.
 *        JaniceDetection is an opaque data type to hold
 *        detection information. There are no restrictions
 *        on the structure of the detection but it must be
 *        able to return certain information through the API
 *        accessor functions.
 */
typedef struct JaniceDetectionType *JaniceDetection;

/*!
 * \brief Create a detection from an image and a bounding box.
 * \param[in] media Media that contains a face
 * \param[in] rect Rectangle that shows the location of a face of interest. If
 *                 the media is a video, the location is of the first appearance
 *                 of a person of interest. The implementation is responsible for
 *                 finding additional examples of the person of interest in successive
 *                 frames if it could benefit from additional information.
 * \param[in] frame The index of the frame that the face appears in.
 * \param[out] detection A detection object encoding the media and rectangle
 */
JANICE_EXPORT JaniceError janice_create_detection(const JaniceMedia &media,
                                                  const JaniceRect &rect,
                                                  const uint32_t frame,
                                                  JaniceDetection &detection);

/*!
 * \brief Detect objects in a #JaniceMedia instance.
 *
 * \section face_size Minimum Face Size
 * A minimum size of faces to be detected may be specified by the
 * caller of \ref janice_detect. This size will be given as a pixel
 * value and corresponds to the width of the face.
 *
 * \param[in] media Media to detect objects in.
 * \param[in] min_face_size The minimum width of detected faces that should be returned. The value is in pixels.
 * \param[out] tracks Empty vector to be filled with detected objects.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT JaniceError janice_detect(const JaniceMedia &media,
                                        const uint32_t min_face_size,
                                        std::vector<JaniceDetection> &detections);

/*!
 * \brief Accessor function to get a list of rectangles from a detection object.
 *        If the detection object was collected from a video it should return 1
 *        or more rectangles taken from sequential frames. If the object was
 *        collected from an image it should return a vector of size 1.
 * \param[in] detection The detection object to get the rectangle from
 * \param[out] rects An empty vector of rectangle objects to store the detection rectangles
 * \note This function is required.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT JaniceError janice_get_rects_from_detection(const JaniceDetection &detection,
                                                          std::vector<JaniceRect> &rects);

/*!
 * \brief Accessor function to get the frame offset for a detection object. The
 *        frame offset is the number of frames that pass in a video before the
 *        detection object begins.
 * \param[in] detection The detection object to get the frame offset from
 * \param[out] offset The frame offset. It should be -1 if the detection object
 *             was collected from an image.
 * \note This function is required.
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT JaniceError janice_get_frame_offset_from_detection(const JaniceDetection &detection,
                                                                 int &offset);

/*!
 * \brief Accessor function to get the confidence for a detection object. In
 *        the case of a video and a detection object that spans multiple frames
 *        the confidence should still be a single value.
 * \param[in] detection The detection object to get the frame offset from
 * \param[out] confidence The detection confidence
 * \note This function is required.
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT JaniceError janice_get_confidence_from_detection(const JaniceDetection &detection,
                                                               double &confidence);

/*!
 * \brief Accessor function to get a gender from a detection object.
 * \param[in] detection The detection object to get the gender from.
 * \param[out] gender Should be 1 for male and 0 for female.
 * \note This function is optional and can return JANICE_NOT_IMPLEMENTED.
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT JaniceError janice_get_gender_from_detection(const JaniceDetection &detection,
                                                           int &gender);

/*!
 * \brief Serialize a #JaniceDetection object to a stream
 * \param[in] detection The detection object to serialize
 * \param[in,out] stream The stream to serialize the object to
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT JaniceError janice_serialize_detection(const JaniceDetection &detection,
                                                     std::ostream &stream);

/*!
 * \brief Deserialize a #JaniceDetection object from a stream
 * \param[out] detection An unallocated detection to store the loaded data
 * \param[in,out] stream The stream to deserialize the object from
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT JaniceError janice_deserialize_detection(JaniceDetection &detection,
                                                       std::istream &stream);

/*!
 * \brief Delete a detection object
 * \param detection The object to delete
 * \remark This function is \ref reentrant
 */
JANICE_EXPORT JaniceError janice_delete_detection(JaniceDetection &detection);

/*!
 * \brief Contains the recognition information for an object.
 */
typedef struct JaniceTemplateType *JaniceTemplate;

enum JaniceTemplateRole {
    ENROLLMENT_11 = 0,
    VERIFICATION_11 = 1,
    ENROLLMENT_1N = 2,
    IDENTIFICATION = 3,
    CLUSTERING = 4
};

/*!
 * \brief Build a template from a list of #JaniceDetection objects
 *
 * All of the detections given as input are known to be of a single subject.
 *
 * All media necessary to build a complete template will be passed in at
 * one time and the constructed template is expected to be suitable for
 * verification and search.
 *
 * \param[in,out] detections A vector of detection objects. The vector is mutable so that additional
 *                           metadata created during template creation can be associated with the detection.
 * \param[in] role An enumeration describing the intended function for the created template.
 *                 Implementors are not required to have different types of templates for any/all
 *                 of the roles specified but can if they choose.
 * \param[out] tmpl The template to contain the subject's recognition information.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT JaniceError janice_create_template(std::vector<JaniceDetection> &detections,
                                                 const JaniceTemplateRole role,
                                                 JaniceTemplate &janice_template);

/*!
 * \brief Serialize a template to a stream
 *
 * \param[in] tmpl The template to serialize
 * \param[in, out] stream Output stream to store the template.
 * \remark This function is \ref reentrant
 */
JANICE_EXPORT JaniceError janice_serialize_template(const JaniceTemplate &tmpl,
                                                    std::ostream &stream);

/*!
 * \brief Load a template from a stream
 *
 * \param[out] tmpl An unallocated template to store the loaded data
 * \param[in, out] stream Input stream to deserialize the template from
 * \remark This function is \ref reentrant
 */
JANICE_EXPORT JaniceError janice_deserialize_template(JaniceTemplate &tmpl,
                                                      std::istream &stream);

/*!
 * \brief Delete a template
 *
 * Call this function on a template after it is no longer needed.
 *
 * \param[in,out] tmpl The template to delete.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT JaniceError janice_delete_template(JaniceTemplate &tmpl);

/*!
 * \brief Return a similarity score for two templates.
 *
 * Higher scores indicate greater similarity.
 *
 * The returned \p similarity score can be \em asymmetric.
 *
 * \param[in] reference The reference template to compare against.
 *                      This template was enrolled with the ENROLLMENT_11
 *                      template role.
 * \param[in] verification The verification template to compare with the
 *                         reference. This template was enrolled with the
 *                         VERIFICATION_11 template role.
 * \param[out] similarity Higher values indicate greater similarity.
 * \remark This function is \ref thread_safe.
 * \see janice_search
 */
JANICE_EXPORT JaniceError janice_verify(const JaniceTemplate &reference,
                                        const JaniceTemplate &verification,
                                        double &similarity);

/*!
 * \brief Unique identifier for a \ref JaniceTemplate.
 *
 * Associate a template with a unique identifier during
 * \ref janice_create_gallery.
 * Retrieve the unique identifier from \ref janice_search and \ref janice_cluster.
 */
typedef uint32_t JaniceTemplateId;

/*!
 * \brief A collection of templates for search
 */
typedef struct JaniceGalleryType *JaniceGallery;

/*!
 * \brief Create a gallery from a list of templates.
 *
 * All templates passed into the gallery should have been
 * created with the IDENTIFICATION role.
 *
 * The created gallery does not need to be suitable for search. \ref janice_prepare_gallery
 * will be called on this gallery before it used in any search.
 *
 * \param[in] tmpls List of templates to construct the gallery. This vector should be the
 *                  same size as the #ids vector.
 * \param[in] ids List of unique ids to associate with the templates in the gallery. This
 *                vector should be the same size as the the #tmpls vector.
 * \param[out] gallery An unallocated gallery
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT JaniceError janice_create_gallery(const std::vector<JaniceTemplate> &tmpls,
                                                const std::vector<JaniceTemplateId> &ids,
                                                JaniceGallery &gallery);

/*!
 * \brief Serialize a gallery to a stream
 *
 * \param[in] gallery The gallery to serialize
 * \param[in, out] stream The output stream to store the serialized gallery
 * \remark This function is \ref reentrant
 */
JANICE_EXPORT JaniceError janice_serialize_gallery(const JaniceGallery &gallery,
                                                   std::ostream &stream);

/*!
 * \brief Load a janice_gallery from a stream
 *
 * \param[out] gallery An unallocated gallery to store the loaded data
 * \param[in, out] stream The input stream to load data from
 * \remark This function is \ref reentrant
 */
JANICE_EXPORT JaniceError janice_deserialize_gallery(JaniceGallery &gallery,
                                                     std::istream &stream);

/*!
 * \brief Prepare a gallery to be searched against.
 *
 * \param[in,out] gallery The gallery to prepare.
 * \remark This function is reentrant
 */
JANICE_EXPORT JaniceError janice_prepare_gallery(JaniceGallery &gallery);

/*!
 * \brief Insert a template into a gallery. After insertion the gallery does
 * not need to be suitable for search.
 *
 * \param[in,out] gallery The gallery to insert the template into
 * \param[in] tmpl The template to insert
 * \param[in] id Unique id for the new template
 * \remark This function \ref reentrant
 */
JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery &gallery,
                                                const JaniceTemplate &tmpl,
                                                const JaniceTemplateId id);

/*!
 * \brief Remove a template from a gallery. After removal the gallery does
 * not need to be suitable for search.
 *
 * \param[in,out] gallery The gallery to remove a template from
 * \param[in] id Unique id for the template to delete
 * \remark This function \ref reentrant
 */
JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery &gallery,
                                                const JaniceTemplateId id);

/*!
 * \brief Delete a gallery
 *
 * Call this function on a gallery after it is no longer needed.
 *
 * \param[in,out] gallery The gallery to delete.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT JaniceError janice_delete_gallery(JaniceGallery &gallery);

/*!
 * \brief Ranked search for a template against a gallery.
 *
 * Prior to be passed to search the gallery must be prepared with \ref janice_prepare_gallery.
 *
 * The probe must have be created with the ENROLLMENT_1N role.
 *
 * \p template_ids and \p similarities are empty vectors to hold the return
 * scores. The number of returns should be less than or equal to \p num_requested_returns,
 * depending on the contents of the gallery.
 *
 * The returned \p similarities \em may be normalized by the implementation
 * based on the contents of the \p gallery. Therefore, similarity scores
 * returned from searches against different galleries are \em not guaranteed to
 * be comparable.
 *
 * \param[in] probe Probe to search for.
 * \param[in] gallery Gallery to search against.
 * \param[in] num_requested_returns The desired number of returned results.
 * \param[out] ids Empty vector to contain the \ref JaniceTemplateId
 *                 of the top matching gallery templates.
 * \param[out] similarities Empty vector to contain the similarity scores of
 *                          the top matching templates.
 * \remark This function is \ref thread_safe.
 * \see janice_verify
 */
JANICE_EXPORT JaniceError janice_search(const JaniceTemplate &probe,
                                        const JaniceGallery &gallery,
                                        const size_t num_requested_returns,
                                        std::vector<JaniceTemplateId> &ids,
                                        std::vector<double> &similarities);

/*!
 * \brief A struct to hold data relevant to a specific cluster.
 */
struct JaniceCluster
{
    uint32_t cluster_id; /*!< Cluster identifier */
    std::vector<JaniceTemplateId> ids; /*!< Set of ids belonging in this cluster */
    double purity_confidence; /*!< A confidence that the cluster only contains a single subject. */
};

/*!
 * \brief Cluster a collection of templates into unique identities.
 *
 * The templates must have been created with the CLUSTERING role.
 *
 * \section clusters Clusters
 * Clusters should be defined using the #JaniceCluster struct. Clusters
 * are often discussed in terms of their "purity" in this API. In this case,
 * purity refers to the number of different subjects in a cluster. A perfectly
 * pure cluster would have media from only a single subject.
 *
 * \section clustering_hint Clustering Hint
 * Clustering is generally considered to be an ill-defined problem, and most
 * algorithms require some help determining the appropriate number of clusters.
 * The \p hint parameter helps influence the number of clusters, though the
 * implementation is free to ignore it.
 * The goal of the hint is to provide user input for two use cases:
 *
 * If the hint is between 0 - 1 it should be regarded as a purity requirement for the algorithm.
 * A 1 indicates the user wants perfectly pure clusters, even if that means more
 * clusters are returned. A 0 indicates that the user wants very few clusters returned
 * and accepts there may be some errors.
 *
 * If the hint is > 1 it represents an estimated upper bound on the number of subjects
 * in the set.
 *
 * \param[in] tmpls The collection of templates to cluster.
 * \param[in] hint A hint to the clustering algorithm, see \ref clustering_hint.
 * \param[out] clusters A list of JaniceClusters, see \ref clusters.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT JaniceError janice_cluster(const std::vector<JaniceTemplate> &tmpls,
                                         const std::vector<JaniceTemplateId> &ids,
                                         const double hint,
                                         std::vector<JaniceCluster> &clusters);

/*!
 * \brief Call once at the end of the application, after making all other calls
 * to the API.
 * \remark This function is \ref thread_unsafe and should only be called once.
 * \see janice_initialize
 */
JANICE_EXPORT JaniceError janice_finalize();

/*! @}*/

#endif /* JANICE_H */
