#ifndef JANICE_H
#define JANICE_H

#include <janice_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JANICE_VERSION_MAJOR 5
#define JANICE_VERSION_MINOR 0
#define JANICE_VERSION_PATCH 0

// ----------------------------------------------------------------------------
// Initialization

JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                            const char* temp_path,
                                            const char* algorithm,
                                            const int num_threads,
                                            const int* gpus,
                                            const int num_gpus);

// ----------------------------------------------------------------------------
// Versioning

JANICE_EXPORT JaniceError janice_api_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);

JANICE_EXPORT JaniceError janice_sdk_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);

// ----------------------------------------------------------------------------
// Training

JANICE_EXPORT JaniceError janice_train(const char* data_prefix,
                                       const char* data_list);

// ----------------------------------------------------------------------------
// Detection

// Structs
typedef struct JaniceDetectionType* JaniceDetection;
typedef const struct JaniceDetectionType* JaniceConstDetection;
typedef JaniceDetection* JaniceDetections;
typedef JaniceConstDetection* JaniceConstDetections;

struct JaniceRect
{
    uint32_t x, y, width, height;
};

struct JaniceTrack
{
    JaniceRect* rects;
    float* confidences;
    uint32_t* frames;
    size_t length;
};

typedef char* JaniceAttribute;

// Functions
JANICE_EXPORT JaniceError janice_create_detection_from_rect(JaniceMediaIterator media,
                                                            const JaniceRect rect,
                                                            uint32_t frame,
                                                            JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_create_detection_from_track(JaniceMediaIterator media,
                                                             const JaniceTrack track,
                                                             JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_detect(JaniceMediaIterator media,
                                        uint32_t min_object_size,
                                        JaniceDetections* detections,
                                        uint32_t* num_detections);

JANICE_EXPORT JaniceError janice_detect_batch(JaniceMediaIterators media,
                                              JaniceMediaIds media_ids,
                                              uint32_t min_object_size,
                                              JaniceDetections* detections,
                                              JaniceMediaIds* detection_ids,
                                              uint32_t* num_detections);

JANICE_EXPORT JaniceError janice_detection_get_track(JaniceConstDetection detection,
                                                     JaniceTrack* track);

JANICE_EXPORT JaniceError janice_detection_get_attribute(JaniceConstDetection detection,
                                                         JaniceAttribute key,
                                                         JaniceAttribute* value);

// I/O
JANICE_EXPORT JaniceError janice_serialize_detection(JaniceConstDetection detection,
                                                     JaniceBuffer* data,
                                                     size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_detection(const JaniceBuffer data,
                                                       size_t len,
                                                       JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_write_detection(JaniceConstDetection detection,
                                                 const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_free_detections(JaniceDetections* detections, 
                                                 uint32_t num_detections);

JANICE_EXPORT JaniceError janice_free_track(JaniceTrack* track);

JANICE_EXPORT JaniceError janice_free_attribute_value(JaniceAttribute* value);

// ----------------------------------------------------------------------------
// Enrollment

// Structs
enum JaniceEnrollmentType
{
    Janice11Reference = 0,
    Janice11Verification = 1,
    Janice1NProbe = 2,
    Janice1NGallery = 3,
    JaniceCluster = 4
};

typedef struct JaniceTemplateType* JaniceTemplate;
typedef const struct JaniceTemplateType* JaniceConstTemplate;
typedef JaniceTemplate* JaniceTemplates;
typedef JaniceConstTemplate* JaniceConstTemplates;

typedef size_t JaniceTemplateId;
typedef JaniceTemplateId* JaniceTemplateIds;

// Functions
JANICE_EXPORT JaniceError janice_enroll_from_media(JaniceMediaIterator media,
                                                   uint32_t min_object_size,
                                                   JaniceEnrollmentType role,
                                                   JaniceTemplates* tmpls,
                                                   JaniceTracks* tracks,
                                                   uint32_t* num_tmpls);

JANICE_EXPORT JaniceError janice_enroll_from_media_batch(JaniceMediaIterators* medias,
                                                         JaniceMediaIds media_ids,
                                                         uint32_t min_object_size,
                                                         JaniceEnrollmentType role,
                                                         JaniceTemplates* tmpls,
                                                         JaniceTracks* tracks,
                                                         JaniceMediaIds* tmpl_ids,
                                                         uint32_t* num_tmpls);

JANICE_EXPORT JaniceError janice_enroll_from_detections(JaniceConstDetections detections,
                                                        uint32_t num_detections,
                                                        JaniceEnrollmentType role,
                                                        JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_enroll_from_detections_batch(JaniceConstDetections detections,
                                                              JaniceTemplateIds ids,
                                                              uint32_t num_detections,
                                                              JaniceEnrollmentType role,
                                                              JaniceTemplates* tmpls,
                                                              uint32_t* num_tmpls);

JANICE_EXPORT JaniceError janice_template_get_attribute(JaniceConstTemplate tmpl,
                                                        JaniceAttribute attribute,
                                                        JaniceAttribute* value);

// I/O
JANICE_EXPORT JaniceError janice_serialize_template(JaniceConstTemplate tmpl,
                                                    JaniceBuffer* data,
                                                    size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_template(const JaniceBuffer data,
                                                      size_t len,
                                                      JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                               JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_write_template(JaniceConstTemplate tmpl,
                                                const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);

// ----------------------------------------------------------------------------
// Verification

typedef double JaniceSimilarity;
typedef JaniceSimilarity* JaniceSimilarities;

JANICE_EXPORT JaniceError janice_verify(JaniceConstTemplate reference,
                                        JaniceConstTemplate verification,
                                        JaniceSimilarity* similarity);

JANICE_EXPORT JaniceError janice_verify_batch(JaniceConstTemplates references,
                                              JaniceConstTemplates verifications,
                                              JaniceSimilarities* similarities);

// ----------------------------------------------------------------------------
// Gallery

// Structs
typedef struct JaniceGalleryType* JaniceGallery;
typedef const struct JaniceGalleryType* JaniceConstGallery;

// Functions
JANICE_EXPORT JaniceError janice_create_gallery(JaniceConstTemplates tmpls,
                                                const JaniceTemplateIds ids,
                                                uint32_t num_tmpls,
                                                JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_gallery_reserve(size_t n);

JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                JaniceConstTemplate tmpl,
                                                JaniceTemplateId id);

JANICE_EXPORT JaniceError janice_gallery_insert_batch(JaniceGallery gallery,
                                                      JaniceConstTemplates tmpls,
                                                      JaniceTemplateIds ids);

JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                JaniceTemplateId id);

JANICE_EXPORT JaniceError janice_gallery_remove_batch(JaniceGallery gallery,
                                                      JaniceTemplateIds ids);

JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);

// I/O
JANICE_EXPORT JaniceError janice_serialize_gallery(JaniceConstGallery gallery,
                                                   JaniceBuffer* data,
                                                   size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_gallery(const JaniceBuffer data,
                                                     size_t len,
                                                     JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                              JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_write_gallery(JaniceConstGallery gallery,
                                               const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);

// ----------------------------------------------------------------------------
// Search

struct JaniceSearchResult
{
    JaniceSimilarities similarities;
    JaniceTemplateIds ids;
    uint32_t num_returned;
};

typedef JaniceSearchResult* JaniceSearchResults;

JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                        JaniceConstGallery gallery,
                                        uint32_t num_requested,
                                        JaniceSearchResult* results);

JANICE_EXPORT JaniceError janice_search_batch(JaniceConstTemplates probes,
                                              uint32_t num_probes,
                                              JaniceConstGallery gallery,
                                              uint32_t num_requested,
                                              JaniceSearchResults* results,
                                              uint32_t num_results);

// Cleanup
JANICE_EXPORT JaniceError janice_free_search_result(JaniceSearchResult* result)

JANICE_EXPORT JaniceError janice_free_search_results(JaniceSearchResults* results);

// ----------------------------------------------------------------------------
// Cluster

// Structs
typedef uint32_t JaniceClusterId;

struct JaniceMediaClusterItem
{
    JaniceClusterId cluster_id;
    JaniceMediaId media_id;
    double confidence;

    JaniceTrack track;
};

typedef struct JaniceMediaClusterItem* JaniceMediaClusterItems;

struct JaniceTemplateClusterItem
{
    JaniceClusterId cluster_id;
    JaniceTemplateId tmpl_id;
    double confidence;
};

typedef struct JaniceTemplateClusterItem* JaniceTemplateClusterItems;

// Functions
JANICE_EXPORT JaniceError janice_cluster_media(JaniceMediaIterators input,
                                               const JaniceMediaIds input_ids,
                                               uint32_t num_inputs,
                                               uint32_t hint,
                                               JaniceMediaClusterItems* clusters,
                                               uint32_t* num_clusters);

JANICE_EXPORT JaniceError janice_cluster_templates(JaniceConstTemplates input,
                                                   const JaniceTemplateIds input_ids,
                                                   uint32_t num_inputs,
                                                   uint32_t hint,
                                                   JaniceTemplateClusterItems* clusters,
                                                   uint32_t* num_clusters);

// Cleanup
JANICE_EXPORT JaniceError janice_free_media_cluster_items(JaniceMediaClusterItems* clusters);

JANICE_EXPORT JaniceError janice_free_template_cluster_items(JaniceTemplateClusterItems* clusters);

// ----------------------------------------------------------------------------
// Finalize

JANICE_EXPORT JaniceError janice_finalize();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_H
