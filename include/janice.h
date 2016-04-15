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
#define JANICE_VERSION_MINOR 2
#define JANICE_VERSION_PATCH 0

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
    JANICE_INVALID_MEDIA      , /*!< Could not decode media file */
    JANICE_MISSING_TEMPLATE_ID, /*!< Expected a missing template ID */
    JANICE_MISSING_FILE_NAME  , /*!< Expected a missing file name */
    JANICE_NULL_ATTRIBUTES    , /*!< Null #janice_attributes */
    JANICE_MISSING_ATTRIBUTES , /*!< Not all required attributes were
                                    provided */
    JANICE_FAILURE_TO_DETECT  , /*!< Could not localize a face within the
                                    provided image */
    JANICE_FAILURE_TO_ENROLL  , /*!< Could not construct a template from the
                                    provided image and attributes */
    JANICE_FAILURE_TO_SERIALIZE, /*!< Could not serialize a template or
                                     gallery */
    JANICE_FAILURE_TO_DESERIALIZE, /*!< Could not deserialize a template
                                       or gallery */
    JANICE_NOT_IMPLEMENTED    , /*!< Optional functions may return this value in
                                    lieu of a meaninful implementation */
    JANICE_NUM_ERRORS         /*!< Idiom to iterate over all errors */
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
 * \brief Common representation for still images and videos.
 *
 * Pixels are stored in _row-major_ order.
 * In other words, pixel layout with respect to decreasing memory spatial
 * locality is \a channel, \a column, \a row \a frames.
 * Thus pixel intensity can be retrieved as follows:
 *
\code
janice_data get_intensity(janice_media media, size_t channel, size_t column,
                                              size_t row, size_t frame)
{
    const size_t columnStep = (image.color_space == JANICE_BGR24 ? 3 : 1);
    const size_t rowStep    = image.width * columnStep;
    const size_t index      = row * rowStep + column * columnStep + channel;
    return image.data[frame][index];
}
\endcode
 *
 * Coordinate (0, 0) corresponds to the top-left corner of the image.
 * Coordinate (width-1, height-1) corresponds to the bottom-right corner of the image.
 */
typedef struct janice_media
{
    std::vector<janice_data*> data; /*! < \brief A collection of image data of size N,
                                                where is the number of frames in a video
                                                or 1 in the case of a still image. */
    size_t width;     /*!< \brief Column count in pixels. */
    size_t height;    /*!< \brief Row count in pixels. */
    janice_color_space color_space; /*!< \brief Arrangement of #data. */
} janice_media;

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
    double face_x; /*!< \brief Horizontal offset to top-left corner of face
                               (pixels) \see \ref face. */
    double face_y; /*!< \brief Vertical offset to top-left corner of face
                               (pixels) \see \ref face. */
    double face_width; /*!< \brief Face horizontal size (pixels)
                                   \see \ref face. */
    double face_height; /*!< \brief Face vertical size (pixels)
                                    \see \ref face. */
    double right_eye_x; /*!< \brief Face landmark (pixels)
                                    \see \ref right_eye. */
    double right_eye_y; /*!< \brief Face landmark (pixels)
                                    \see \ref right_eye. */
    double left_eye_x; /*!< \brief Face landmark (pixels) \see \ref left_eye. */
    double left_eye_y; /*!< \brief Face landmark (pixels) \see \ref left_eye. */
    double nose_base_x; /*!< \brief Face landmark (pixels)
                                    \see \ref nose_base. */
    double nose_base_y; /*!< \brief Face landmark (pixels)
                                    \see \ref nose_base. */
    double face_yaw; /*!< \brief Face yaw estimation (degrees). */
    bool forehead_visible; /*!< \brief Visibility of forehead
                                  \see forehead_visible. */
    bool eyes_visible; /*!< \brief Visibility of eyes
                                     \see \ref eyes_visible. */
    bool nose_mouth_visible; /*!< \brief Visibility of nose and mouth
                                    \see nouse_mouth_visible. */
    bool indoor; /*!< \brief Image was captured indoors \see \ref indoor. */

    double frame_number; /*!< \brief Frame number or -1 for images. */
} janice_attributes;

/*!
 * \brief A list of janice_attributes representing a single identity in a
 *        \ref janice_media instance.
 */
typedef struct janice_track
{
    std::vector<janice_attributes> track;

    double detection_confidence; /*!< \brief A higher value indicates greater
                                             detection confidence. */
    double gender; /*!< \brief Gender of subject of interest, 1 for male, 0 for
                        female. */
    double age; /*!< \brief Approximate age of subject (years) \see \ref age. */
    double skin_tone; /*!< \brief Skin tone of subject \see \ref skin_tone. */

    double frame_rate; /*!< \brief Frames per second, or 0 for images. */
} janice_track;

/*!
 * \brief An association between a piece of media and metadata.
 *
 * All metadata in an association can be assumed to belong to a
 * single subject.
 */
typedef struct janice_association
{
    janice_media media;
    std::vector<janice_track> metadata;
} janice_association;

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
JANICE_EXPORT janice_error janice_initialize(const std::string &sdk_path,
                                             const std::string &temp_path,
                                             const std::string &algorithm,
                                             const int gpu_dev);

/*!
 * \brief Detect objects in a #janice_media.
 *
 * Each object is represented by a #janice_track. Detected objects
 * can then be used to create #janice_assocation and passed to
 * \ref janice_create_template.
 *
 * \note The number of attributes in a track can never exceed the
 * number of frames in a janice_media instance. Still images should
 * return tracks of length 1.
 *
 * \section face_size Minimum Face Size
 * A minimum size of faces to be detected may be specified by the
 * caller of \ref janice_detect. This size will be given as a pixel
 * value and corresponds to the width of the face.
 *
 * \section detection_guarantees Detection Guarantees
 * The returned tracks will be ordered by decreasing
 * janice_track::detection_confidence.
 *
 * Each of the tracks will have values for at least janice_track::detection_confidence.
 * Each of the janice_attributes within a track will have values for
 *  - janice_attributes::face_x
 *  - janice_attributes::face_y
 *  - janice_attributes::face_width
 *  - janice_attributes::face_height
 *  - janice_attributes::frame_number
 *
 * Any attribute of the track or a janice_attributes within the track
 * without a value will be set to \c NaN.
 *
 * \param[in] media Media to detect objects in.
 * \param[in] min_face_size The minimum width of detected faces that should be returned. The value is in pixels.
 * \param[out] tracks Empty vector to be filled with detected objects.
 * \remark This function is \ref thread_safe.
 */
JANICE_EXPORT janice_error janice_detect(const janice_media &media,
                                         const size_t min_face_size,
                                         std::vector<janice_track> &tracks);

/*!
 * \brief Contains the recognition information for an object.
 */
typedef struct janice_template_type *janice_template;

typedef enum janice_template_role {
    ENROLLMENT_11 = 0,
    VERIFICATION_11 = 1,
    ENROLLMENT_1N = 2,
    IDENTIFICATION = 3,
    CLUSTERING = 4
} janice_template_role;

/*!
 * \brief Build a template from a list of janice_associations
 *
 * All media necessary to build a complete template will be passed in at
 * one time and the constructed template is expected to be suitable for
 * verification and search.
 *
 * \param[in] associations A vector of associations between a piece of media
 *                         and relevant metadata. All of the associations provided
 *                         are guaranteed to be of a single subject
 * \param[in] role An enumeration describing the intended function for the created template.
 *                 Implementors are not required to have different types of templates for any/all
 *                 of the roles specified but can if they choose.
 * \param[out] template_ The template to contain the subject's recognition information.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_create_template(const std::vector<janice_association> &associations,
                                                  const janice_template_role role,
                                                  janice_template &template_);

/*!
 * \brief Build a list of templates from a single piece of janice_media
 *
 * There is no guarantee on the number of individual subjects appearing in the media
 * and implementors should create as many templates as necessary to represent all of
 * the people they find.
 *
 * Implementors must return a single janice_track for each template they create. The track
 * must have all of the metadata fields specified in \ref detection_guarantees.
 *
 * \param[in] media An image or a video containing an unknown number of identities
 * \param[in] role An enumeration describing the intended function for the created template.
 *                 Implementors are not required to have different types of templates for any/all
 *                 of the roles specified but can if they choose.
 * \param[out] templates A list of templates containing recognition information for all of the
 *                       identities discovered in the media.
 * \param[out] tracks A list of metadata corresponding to the return templates.
 */
JANICE_EXPORT janice_error janice_create_template(const janice_media &media,
                                                  const janice_template_role role,
                                                  std::vector<janice_template> &templates,
                                                  std::vector<janice_track> &tracks);

/*!
 * \brief Serialize a template to a byte array
 *
 * The array could be stored on disk, sent to a database, or
 * whatever else the implementor decides.
 *
 * \param[in] template_ The template to serialize
 * \param[out] data Unallocated byte array to store the serialized template
 * \param[out] template_bytes Length of the serialized template
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT janice_error janice_serialize_template(const janice_template &template_,
                                                     janice_data *&data,
                                                     size_t &template_bytes);

/*!
 * \brief Convert a byte array to a janice_template
 *
 * The byte array was initially created with #janice_serialize_template
 *
 * \param[in] data Byte array of serialized template data
 * \param[in] template_bytes Size of \p data
 * \param[out] template_ Unallocated template to hold deserialized template
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT janice_error janice_deserialize_template(const janice_data *data,
                                                       const size_t template_bytes,
                                                       janice_template &template_);

/*!
 * \brief Delete a serialized template
 *
 * Call this function on a serialized template after it is no longer needed.
 *
 * \param[in,out] template_ The serialized template to delete.
 * \param[in] template_bytes Size of \p data
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_delete_serialized_template(janice_data *&template_,
                                                             const size_t template_bytes);

/*!
 * \brief Delete a template
 *
 * Call this function on a template after it is no longer needed.
 *
 * \param[in,out] template_ The template to delete.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_delete_template(janice_template &template_);

/*!
 * \brief Return a similarity score for two templates.
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
JANICE_EXPORT janice_error janice_verify(const janice_template &a,
                                         const janice_template &b,
                                         double &similarity);

/*!
 * \brief Unique identifier for a \ref janice_template.
 *
 * Associate a template with a unique identifier during
 * \ref janice_create_gallery.
 * Retrieve the unique identifier from \ref janice_search and \ref janice_cluster.
 */
typedef size_t janice_template_id;

/*!
 * \brief A collection of templates for search
 */
typedef struct janice_gallery_type *janice_gallery;

/*!
 * \brief Create a gallery from a list of templates.
 *
 * The created gallery should be suitable for search.
 *
 * \param[in] templates List of templates to construct the gallery
 * \param[in] ids list of unique ids to associate with the templates in the gallery
 * \param[out] gallery The created gallery
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT janice_error janice_create_gallery(const std::vector<janice_template> &templates,
                                                 const std::vector<janice_template_id> &ids,
                                                 janice_gallery &gallery);

/*!
 * \brief Serialize a gallery to a byte array
 *
 * The array could be stored on disk, sent to a database, or
 * whatever else the implementor decides.
 *
 * \param[in] gallery The gallery to serialize
 * \param[out] data Unallocated byte array to store the serialized gallery
 * \param[out] gallery_bytes Length of the serialized gallery
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT janice_error janice_serialize_gallery(const janice_gallery &gallery,
                                                    janice_data *&data,
                                                    size_t &gallery_bytes);

/*!
 * \brief Convert a byte array to a janice_gallery
 *
 * The byte array was initially created with #janice_serialize_gallery
 *
 * \param[in] data Byte array of serialized gallery data
 * \param[in] gallery_bytes Size of \p data
 * \param[out] gallery Unallocated gallery to hold deserialized gallery
 * \remark This function is \ref thread_safe
 */
JANICE_EXPORT janice_error janice_deserialize_gallery(const janice_data *data,
                                                      const size_t gallery_bytes,
                                                      janice_gallery &gallery);

/*!
 * \brief Insert a template into a gallery.
 *
 * \param[in,out] gallery The gallery to insert the template into
 * \param[in] template_ The template to insert
 * \param[in] id Unique id for the new template
 * \remark This function \ref reentrant
 */
JANICE_EXPORT janice_error janice_gallery_insert(janice_gallery &gallery,
                                                 const janice_template &template_,
                                                 const janice_template_id id);

/*!
 * \brief Remove a template from a gallery.
 *
 * \param[in,out] gallery The gallery to remove a template from
 * \param[in] id Unique id for the template to delete
 * \remark This function \ref reentrant
 */
JANICE_EXPORT janice_error janice_gallery_remove(janice_gallery &gallery,
                                                 const janice_template_id id);

/*!
 * \brief Delete a gallery
 *
 * Call this function on a gallery after it is no longer needed.
 *
 * \param[in,out] gallery The gallery to delete.
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_delete_gallery(janice_gallery &gallery);

/*!
 * \brief Delete a serialized gallery
 *
 * Call this function on a serialized gallery after it is no longer needed.
 *
 * \param[in,out] gallery The serialized gallery to delete.
 * \param[in] gallery_bytes Size of \p data
 * \remark This function is \ref reentrant.
 */
JANICE_EXPORT janice_error janice_delete_serialized_gallery(janice_data *&gallery,
                                                            const size_t gallery_bytes);

/*!
 * \brief Ranked search for a template against a gallery.
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
 * \param[out] template_ids Empty vector to contain the \ref janice_template_id
 *                          of the top matching gallery templates.
 * \param[out] similarities Empty vector to contain the similarity scores of
 *                          the top matching templates.
 * \remark This function is \ref thread_safe.
 * \see janice_verify
 */
JANICE_EXPORT janice_error janice_search(const janice_template &probe,
                                         const janice_gallery &gallery,
                                         const size_t num_requested_returns,
                                         std::vector<janice_template_id> &template_ids,
                                         std::vector<double> &similarities);

/*!
 * \brief Cluster a collection of unlabelled people into distinct identites.
 *
 * Bounding boxes for all subjects of interest in the media will be provided.
 *
 * \section clusters Clusters
 * Clusters are represented as a list of lists of <int, double> pairs. Each
 * pairing consists of the cluster id and the cluster confidence and corresponds
 * to an instance of an identity in a single piece of \ref janice_media.
 *
 * \section clustering_hint Clustering Hint
 * Clustering is generally considered to be an ill-defined problem, and most
 * algorithms require some help determining the appropriate number of clusters.
 * The \p hint parameter helps influence the number of clusters, though the
 * implementation is free to ignore it.
 * The goal of the hint is to provide an order of magnitude estimation for the
 * number of identities that appear in a set of media. As such it will be a
 * multiple of 10 (10, 100, 1000 etc.).
 *
 * \note The implementation of this function is optional, and may return
 *       #JANICE_NOT_IMPLEMENTED.
 *
 * \param[in] associations The collection of media and relevant detection information to cluster
 * \param[in] hint A hint to the clustering algorithm, see \ref clustering_hint.
 * \param[out] clusters A list of lists of cluster pairs, see \ref clusters.
 * \remark This function is \ref thread_safe.
 */
typedef std::pair<int, double> cluster_pair;
JANICE_EXPORT janice_error janice_cluster(const std::vector<janice_association> &associations,
                                          const size_t hint,
                                          std::vector<std::vector<cluster_pair> > &clusters);

/*!
 * \brief Cluster an unlabelled set of media into distinct identities.
 *
 * No bounding box information will be provided and all people must be detected
 * by the implementor. These detections should be return as a list of lists of
 * janice_tracks.
 *
 * \note The implementation of this function is optional, and may return
 *       #JANICE_NOT_IMPLEMENTED
 *
 * \param[in] media The collection of media to cluster.
 * \param[in] hint A hint to the clustering algorithm, see \ref clustering_hint.
 * \param[out] clusters A list of lists of cluster pairs, see \ref clusters.
 * \param[out] tracks A list of lists of tracks that must be the same size as #clusters.
 *                    Each track should provide detection information for the associated
 *                    \ref cluster_pair.
 */
JANICE_EXPORT janice_error janice_cluster(const std::vector<janice_media> &media,
                                          const size_t hint,
                                          std::vector<std::vector<cluster_pair> > &clusters,
                                          std::vector<std::vector<janice_track> > &tracks);
/*!
 * \brief Call once at the end of the application, after making all other calls
 * to the API.
 * \remark This function is \ref thread_unsafe and should only be called once.
 * \see janice_initialize
 */
JANICE_EXPORT janice_error janice_finalize();

/*! @}*/

#endif /* JANICE_H */
