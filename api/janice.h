#ifndef JANICE_H
#define JANICE_H

#include <janice_io.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JANICE_VERSION_MAJOR 4
#define JANICE_VERSION_MINOR 0
#define JANICE_VERSION_PATCH 0

// ----------------------------------------------------------------------------
// Initialization

JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                            const char* temp_path,
                                            const char* algorithm,
                                            const int gpu_dev);

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
// Detection Iterator

typedef struct JaniceDetectionIteratorType* JaniceDetectionIterator;

// Functions
JANICE_EXPORT JaniceError janice_detection_it_next(JaniceDetectionIterator it,
                                                   JaniceRect* rect,
                                                   uint32_t* frame,
                                                   float* confidence);

JANICE_EXPORT JaniceError janice_detection_it_reset(JaniceDetectionIterator it);

// Cleanup
JANICE_EXPORT JaniceError janice_free_detection_iterator(JaniceDetectionIterator* it);

// ----------------------------------------------------------------------------
// Detection

// Structs
struct JaniceRect
{
    uint32_t x, y, width, height;
};

typedef struct JaniceDetectionType* JaniceDetection;
typedef const struct JaniceDetectionType* JaniceConstDetection;
typedef JaniceDetection* JaniceDetections;
typedef JaniceConstDetection* JaniceConstDetections;

// Functions
JANICE_EXPORT JaniceError janice_create_detection(JaniceConstMedia media,
                                                  const JaniceRect rect,
                                                  uint32_t frame,
                                                  JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_detect(JaniceConstMedia media,
                                        uint32_t min_object_size,
                                        JaniceDetections* detections,
                                        uint32_t* num_detections);

JANICE_EXPORT JaniceError janice_create_detection_it(JaniceConstDetection detection,
                                                     JaniceDetectionIterator* it);

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

// Functions
JANICE_EXPORT JaniceError janice_create_template(JaniceConstDetections detections,
                                                 uint32_t num_detections,
                                                 JaniceEnrollmentType role,
                                                 JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_template_get_attribute(JaniceConstTemplate tmpl,
                                                        const char* attribute,
                                                        char** value);

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

JANICE_EXPORT JaniceError janice_verify(JaniceConstTemplate reference,
                                        JaniceConstTemplate verification,
                                        JaniceSimilarity similarity);

// ----------------------------------------------------------------------------
// Gallery

// Structs
typedef struct JaniceGalleryType* JaniceGallery;
typedef const struct JaniceGalleryType* JaniceConstGallery;

typedef uint32_t JaniceTemplateId;
typedef JaniceTemplateId* JaniceTemplateIds;

// Functions
JANICE_EXPORT JaniceError janice_create_gallery(JaniceConstTemplates tmpls,
                                                const JaniceTemplateIds ids,
                                                uint32_t num_tmpls,
                                                JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                JaniceConstTemplate tmpl,
                                                JaniceTemplateId id);

JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                JaniceTemplateId id);

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

typedef JaniceSimilarity* JaniceSimilarities;
typedef JaniceTemplateId* JaniceSearchTemplateIds;

JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                        JaniceConstGallery gallery,
                                        uint32_t num_requested,
                                        JaniceSimilarities* similarities,
                                        JaniceSearchTemplateIds* ids,
                                        uint32_t* num_returned);

JANICE_EXPORT JaniceError janice_free_similarities(JaniceSimilarities* similarities);

JANICE_EXPORT JaniceError janice_free_search_ids(JaniceSearchTemplateIds* ids);

// ----------------------------------------------------------------------------
// Cluster

// Structs
typedef uint32_t JaniceClusterId;

typedef uint32_t JaniceMediaId;
typedef JaniceMediaId* JaniceMediaIds;

struct JaniceMediaClusterItem
{
    JaniceClusterId cluster_id;
    JaniceMediaId media_id;
    double confidence;

    JaniceRect rect;
    uint32_t frame;
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
JANICE_EXPORT JaniceError janice_cluster_media(JaniceConstMedias input,
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
