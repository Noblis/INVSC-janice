#ifndef JANICE_H
#define JANICE_H

#include <janice_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JANICE_VERSION_MAJOR 7
#define JANICE_VERSION_MINOR 0
#define JANICE_VERSION_PATCH 0

// ----------------------------------------------------------------------------
// The JanICE API is an open source API that provides a common interface
// between computer vision algorithms and agencies and entities that would like
// to use them.
//
// Full documentation for the API is available in PDF form in
// ${janice_source_dir}/docs/JanICE.pdf and online at
// https://noblis.github.io/janice/.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Initialization

JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                            const char* temp_path,
                                            const char* log_path,
                                            const char* algorithm,
                                            const int num_threads,
                                            const int* gpus,
                                            const int num_gpus);

enum JaniceLogLevel
{
    JaniceLogDebug = 0,
    JaniceLogInfo = 1,
    JaniceLogWarning = 2,
    JaniceLogError = 3,
    JaniceLogCritical = 4
};

JANICE_EXPORT JaniceError janice_set_log_level(JaniceLogLevel level);

// ----------------------------------------------------------------------------
// Versioning

JANICE_EXPORT JaniceError janice_api_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);

JANICE_EXPORT JaniceError janice_sdk_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);

// ----------------------------------------------------------------------------
// Configuration

struct JaniceConfigurationItem
{
    char* key;
    char* value;
};

struct JaniceConfiguration
{
    JaniceConfigurationItem* values;
    size_t length;
};

JANICE_EXPORT JaniceError janice_get_current_configuration(JaniceConfiguration* configuration);

JANICE_EXPORT JaniceError janice_free_configuration(JaniceConfiguration* configuration);

// ----------------------------------------------------------------------------
// Context

enum JaniceDetectionPolicy
{
    JaniceDetectAll = 0,
    JaniceDetectLargest = 1,
    JaniceDetectBest = 2
};

enum JaniceEnrollmentType
{
    Janice11Reference = 0,
    Janice11Verification = 1,
    Janice1NProbe = 2,
    Janice1NGallery = 3,
    JaniceCluster = 4
};

struct JaniceContext
{
    // Detection parameters
    JaniceDetectionPolicy policy;
    uint32_t min_object_size;

    // Enrollment parameters
    JaniceEnrollmentType role;

    // Search parameters
    double threshold;
    uint32_t max_returns;

    // Cluster parameters
    double hint;
};

JANICE_EXPORT JaniceError janice_init_default_context(JaniceContext* context);

// ----------------------------------------------------------------------------
// Buffer

JANICE_EXPORT JaniceError janice_free_buffer(uint8_t** buffer);

// ----------------------------------------------------------------------------
// Detection

// Structs
typedef struct JaniceDetectionType* JaniceDetection;

struct JaniceRect
{
    int x;
    int y;
    int width; 
    int height;
};

struct JaniceTrack
{
    JaniceRect* rects;
    float* confidences;
    uint32_t* frames;
    size_t length;
};

struct JaniceDetections
{
    JaniceDetection* detections;
    size_t length;
};

struct JaniceDetectionsGroup
{
    JaniceDetections* group;
    size_t length;
};

/*!
 * A callback function for immediate detection output. The callback is not
 * expected to be thread-safe.
 *
 * \param image The image or frame the detection occured in
 * \param rect The location of the detection
 * \param frame The frame number of the detection
 * \param track_id An optional track identifier. The implementation may set
 *        this to -1 to indicate no tracking is done. If the identifier is
 *        not -1, it is a unique id for a person being tracked through the media.
 *        If subsequent calls to the function have the same track id they are
 *        sightings of the same person. It is up to the caller to leverage this
 *        information, e.g. by creating a SparseMediaIterator over a track before
 *        enrollment.
 * \param user_data User defined data structure
 */
typedef JaniceError (*JaniceDetectionCallback)(const JaniceImage*, const JaniceRect*, uint32_t, int, void*);

// Functions
JANICE_EXPORT JaniceError janice_create_detection_from_rect(const JaniceMediaIterator* media,
                                                            const JaniceRect* rect,
                                                            const uint32_t frame,
                                                            JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_create_detection_from_track(const JaniceMediaIterator* media,
                                                             const JaniceTrack* track,
                                                             JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_detect(const JaniceMediaIterator* media,
                                        const JaniceContext* context,
                                        JaniceDetections* detections);

JANICE_EXPORT JaniceError janice_detect_batch(const JaniceMediaIterators* media,
                                              const JaniceContext* context,
                                              JaniceDetectionsGroup* detections);

JANICE_EXPORT JaniceError janice_detect_with_callback(const JaniceMediaIterator* media,
                                                      const JaniceContext* context,
                                                      JaniceDetectionCallback callback,
                                                      void* user_data);

JANICE_EXPORT JaniceError janice_detection_get_track(const JaniceDetection detection,
                                                     JaniceTrack* track);

JANICE_EXPORT JaniceError janice_detection_get_attribute(const JaniceDetection detection,
                                                         const char* key,
                                                         char** value);

// I/O
JANICE_EXPORT JaniceError janice_serialize_detection(const JaniceDetection detection,
                                                     uint8_t** data,
                                                     size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_detection(const uint8_t* data,
                                                       const size_t len,
                                                       JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_write_detection(const JaniceDetection detection,
                                                 const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_clear_detections(JaniceDetections* detections);

JANICE_EXPORT JaniceError janice_clear_detections_group(JaniceDetectionsGroup* group);

JANICE_EXPORT JaniceError janice_clear_track(JaniceTrack* track);

JANICE_EXPORT JaniceError janice_free_attribute(char** value);

// ----------------------------------------------------------------------------
// Training

JANICE_EXPORT JaniceError janice_fine_tune(const JaniceMediaIterators* media,
                                           const JaniceDetectionsGroup* detections,
                                           int** labels,
                                           const char* output_prefix);

// ----------------------------------------------------------------------------
// Enrollment

// Structs
enum JaniceFeatureVectorType
{
    JaniceInt8 = 0,
    JaniceInt16 = 1,
    JaniceInt32 = 2,
    JaniceInt64 = 3,

    JaniceUInt8 = 4,
    JaniceUInt16 = 5,
    JaniceUInt32 = 6,
    JaniceUInt64 = 7,

    JaniceFloat = 8,
    JaniceDouble = 9
};

typedef struct JaniceTemplateType* JaniceTemplate;

struct JaniceTemplates
{
    JaniceTemplate* tmpls;
    size_t length;
};

struct JaniceTemplatesGroup
{
    JaniceTemplates* group;
    size_t length;
};

struct JaniceTemplateIds
{
    uint64_t* ids;
    size_t length;
};

struct JaniceTemplateIdsGroup
{
    JaniceTemplateIds* group;
    size_t length;
};

// Functions
JANICE_EXPORT JaniceError janice_enroll_from_media(const JaniceMediaIterator* media,
                                                   const JaniceContext* context,
                                                   JaniceTemplates* tmpls,
                                                   JaniceDetections* detections);

JANICE_EXPORT JaniceError janice_enroll_from_media_batch(const JaniceMediaIterators* media,
                                                         const JaniceContext* context,
                                                         JaniceTemplatesGroup* tmpls,
                                                         JaniceDetectionsGroup* detections);

JANICE_EXPORT JaniceError janice_enroll_from_detections(const JaniceMediaIterators* media,
                                                        const JaniceDetections* detections,
                                                        const JaniceContext* context,
                                                        JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_enroll_from_detections_batch(const JaniceMediaIteratorsGroup* media,
                                                              const JaniceDetectionsGroup* detections,
                                                              const JaniceTemplateIds* ids,
                                                              const JaniceContext* context,
                                                              JaniceTemplates* tmpls);

JANICE_EXPORT JaniceError janice_template_is_fte(const JaniceTemplate tmpl,
                                                 int* fte);

JANICE_EXPORT JaniceError janice_template_get_attribute(const JaniceTemplate tmpl,
                                                        const char* key,
                                                        char** value);

JANICE_EXPORT JaniceError janice_template_get_feature_vector(const JaniceTemplate tmpl,
                                                             JaniceFeatureVectorType* feature_vector_type,
                                                             void** feature_vector,
                                                             size_t* length);

// I/O
JANICE_EXPORT JaniceError janice_serialize_template(const JaniceTemplate tmpl,
                                                    uint8_t** data,
                                                    size_t* length);

JANICE_EXPORT JaniceError janice_deserialize_template(const uint8_t* data,
                                                      const size_t length,
                                                      JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                               JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_write_template(const JaniceTemplate tmpl,
                                                const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_clear_templates(JaniceTemplates* tmpls);

JANICE_EXPORT JaniceError janice_clear_templates_group(JaniceTemplatesGroup* group);

// ----------------------------------------------------------------------------
// Verification

// Structs
struct JaniceSimilarities
{
    double* similarities;
    size_t length;
};

struct JaniceSimilaritiesGroup
{
    JaniceSimilarities* group;
    size_t length;
};

// Functions
JANICE_EXPORT JaniceError janice_verify(const JaniceTemplate reference,
                                        const JaniceTemplate verification,
                                        double* similarity);

JANICE_EXPORT JaniceError janice_verify_batch(const JaniceTemplates* references,
                                              const JaniceTemplates* verifications,
                                              JaniceSimilarities* similarities);

// Cleanup
JANICE_EXPORT JaniceError janice_clear_similarities(JaniceSimilarities* similarities);

JANICE_EXPORT JaniceError janice_clear_similarities_group(JaniceSimilaritiesGroup* group);

// ----------------------------------------------------------------------------
// Gallery

// Structs
typedef struct JaniceGalleryType* JaniceGallery;

// Functions
JANICE_EXPORT JaniceError janice_create_gallery(const JaniceTemplates* tmpls,
                                                const JaniceTemplateIds* ids,
                                                JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_gallery_reserve(JaniceGallery gallery,
                                                 const size_t n);

JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                const JaniceTemplate* tmpl,
                                                const uint64_t id);

JANICE_EXPORT JaniceError janice_gallery_insert_batch(JaniceGallery gallery,
                                                      const JaniceTemplates* tmpls,
                                                      const JaniceTemplateIds* ids);

JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                const uint64_t id);

JANICE_EXPORT JaniceError janice_gallery_remove_batch(JaniceGallery gallery,
                                                      const JaniceTemplateIds* ids);

// This function prepares a gallery for search after it has been modified.
// Please see
// https://noblis.github.io/janice/api/gallery.html#janice-gallery-prepare
// for more information.
JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);

// I/O
JANICE_EXPORT JaniceError janice_serialize_gallery(const JaniceGallery gallery,
                                                   uint8_t** data,
                                                   size_t* length);

JANICE_EXPORT JaniceError janice_deserialize_gallery(const uint8_t* data,
                                                     const size_t length,
                                                     JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                              JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_write_gallery(const JaniceGallery gallery,
                                               const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_clear_template_ids(JaniceTemplateIds* ids);

JANICE_EXPORT JaniceError janice_clear_template_ids_group(JaniceTemplateIdsGroup* group);

// ----------------------------------------------------------------------------
// Search

JANICE_EXPORT JaniceError janice_search(const JaniceTemplate probe,
                                        const JaniceGallery gallery,
                                        const JaniceContext* context,
                                        JaniceSimilarities* similarities,
                                        JaniceTemplateIds* ids);

JANICE_EXPORT JaniceError janice_search_batch(const JaniceTemplates* probes,
                                              const JaniceGallery gallery,
                                              const JaniceContext* context,
                                              JaniceSimilaritiesGroup* similarities,
                                              JaniceTemplateIdsGroup* ids);

// ----------------------------------------------------------------------------
// Cluster

// Structs
struct JaniceClusterIds
{
    uint64_t* ids;
    size_t length;
};

struct JaniceClusterIdsGroup
{
    JaniceClusterIds* group;
    size_t length;
};

struct JaniceClusterConfidences
{
    double* confidences;
    size_t length;
};

struct JaniceClusterConfidencesGroup
{
    JaniceClusterConfidences* group;
    size_t length;
};

// Functions
JANICE_EXPORT JaniceError janice_cluster_media(const JaniceMediaIterators* media,
                                               const JaniceContext* context,
                                               JaniceClusterIdsGroup* cluster_ids,
                                               JaniceClusterConfidencesGroup* cluster_confidences,
                                               JaniceDetectionsGroup* detections);

JANICE_EXPORT JaniceError janice_cluster_templates(const JaniceTemplates* tmpls,
                                                   const JaniceContext* context,
                                                   JaniceClusterIds* cluster_ids,
                                                   JaniceClusterConfidences* cluster_confidences);

// Cleanup
JANICE_EXPORT JaniceError janice_clear_cluster_ids(JaniceClusterIds* ids);

JANICE_EXPORT JaniceError janice_clear_cluster_ids_group(JaniceClusterIdsGroup* group);

JANICE_EXPORT JaniceError janice_clear_cluster_confidences(JaniceClusterConfidences* confidences);

JANICE_EXPORT JaniceError janice_clear_cluster_confidences_group(JaniceClusterConfidencesGroup* group);

// ----------------------------------------------------------------------------
// Finalize

JANICE_EXPORT JaniceError janice_finalize();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_H
