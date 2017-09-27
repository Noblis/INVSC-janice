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
// Configuration

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

struct JaniceContextType
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

typedef JaniceContextType* JaniceContext;

JANICE_EXPORT JaniceError janice_create_context(JaniceDetectionPolicy policy,
                                                uint32_t min_object_size,
                                                JaniceEnrollmentType role,
                                                double threshold,
                                                uint32_t max_returns,
                                                double hint,
                                                JaniceContext* context);

JANICE_EXPORT JaniceError janice_free_context(JaniceContext* context);

// ----------------------------------------------------------------------------
// Buffer

typedef uint8_t* JaniceBuffer;

JANICE_EXPORT JaniceError janice_free_buffer(JaniceBuffer* buffer);

// ----------------------------------------------------------------------------
// Detection

// Structs
typedef struct JaniceDetectionType* JaniceDetection;

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

struct JaniceTracks
{
    JaniceTrack* tracks;
    size_t length;
};

struct JaniceTracksGroup
{
    JaniceTracks* group;
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
                                        JaniceContext context,
                                        JaniceDetections* detections);

JANICE_EXPORT JaniceError janice_detect_batch(JaniceMediaIterators media,
                                              JaniceContext context,
                                              JaniceDetectionsGroup* detections);

JANICE_EXPORT JaniceError janice_detection_get_track(JaniceDetection detection,
                                                     JaniceTrack* track);

JANICE_EXPORT JaniceError janice_detection_get_attribute(JaniceDetection detection,
                                                         const char* key,
                                                         JaniceAttribute* value);

// I/O
JANICE_EXPORT JaniceError janice_serialize_detection(JaniceDetection detection,
                                                     JaniceBuffer* data,
                                                     size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_detection(const JaniceBuffer data,
                                                       size_t len,
                                                       JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_write_detection(JaniceDetection detection,
                                                 const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_clear_detections(JaniceDetections* detections);

JANICE_EXPORT JaniceError janice_clear_detections_group(JaniceDetectionsGroup* group);

JANICE_EXPORT JaniceError janice_clear_track(JaniceTrack* track);

JANICE_EXPORT JaniceError janice_clear_tracks(JaniceTracks* tracks);

JANICE_EXPORT JaniceError janice_clear_tracks_group(JaniceTracksGroup* group);

JANICE_EXPORT JaniceError janice_free_attribute(JaniceAttribute* value);

// ----------------------------------------------------------------------------
// Training

JANICE_EXPORT JaniceError janice_fine_tune(JaniceDetections subjects,
                                           int* labels,
                                           const char* output_prefix);

// ----------------------------------------------------------------------------
// Enrollment

// Structs
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

// Functions
JANICE_EXPORT JaniceError janice_enroll_from_media(JaniceMediaIterator media,
                                                   JaniceContext context,
                                                   JaniceTemplates* tmpls,
                                                   JaniceTracks* tracks);

JANICE_EXPORT JaniceError janice_enroll_from_media_batch(JaniceMediaIterators medias,
                                                         JaniceContext context,
                                                         JaniceTemplatesGroup* tmpls,
                                                         JaniceTracksGroup* tracks);

JANICE_EXPORT JaniceError janice_enroll_from_detections(JaniceDetections detections,
                                                        JaniceContext context,
                                                        JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_enroll_from_detections_batch(JaniceDetectionsGroup detections,
                                                              JaniceContext context,
                                                              JaniceTemplates* tmpls);

JANICE_EXPORT JaniceError janice_template_is_fte(JaniceTemplate tmpl,
                                                 int* fte);

JANICE_EXPORT JaniceError janice_template_get_attribute(JaniceTemplate tmpl,
                                                        const char* key,
                                                        JaniceAttribute* value);

// I/O
JANICE_EXPORT JaniceError janice_serialize_template(JaniceTemplate tmpl,
                                                    JaniceBuffer* data,
                                                    size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_template(const JaniceBuffer data,
                                                      size_t len,
                                                      JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                               JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_write_template(JaniceTemplate tmpl,
                                                const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_clear_templates(JaniceTemplates* tmpls);

JANICE_EXPORT JaniceError janice_clear_templates_group(JaniceTemplatesGroup* group);

// ----------------------------------------------------------------------------
// Verification

// Structs
typedef double JaniceSimilarity;

struct JaniceSimilarities
{
    JaniceSimilarity* similarities;
    size_t length;
};

struct JaniceSimilaritiesGroup
{
    JaniceSimilarities* group;
    size_t length;
};

// Functions
JANICE_EXPORT JaniceError janice_verify(JaniceTemplate reference,
                                        JaniceTemplate verification,
                                        JaniceSimilarity* similarity);

JANICE_EXPORT JaniceError janice_verify_batch(JaniceTemplates references,
                                              JaniceTemplates verifications,
                                              JaniceSimilarities* similarities);

// Cleanup
JANICE_EXPORT JaniceError janice_clear_similarities(JaniceSimilarities* similarities);

JANICE_EXPORT JaniceError janice_clear_similarities_group(JaniceSimilaritiesGroup* group);

// ----------------------------------------------------------------------------
// Gallery

// Structs
typedef struct JaniceGalleryType* JaniceGallery;

typedef size_t JaniceTemplateId;

struct JaniceTemplateIds
{
    JaniceTemplateId* ids;
    size_t length;
};

struct JaniceTemplateIdsGroup
{
    JaniceTemplateIds* group;
    size_t length;
};

// Functions
JANICE_EXPORT JaniceError janice_create_gallery(JaniceTemplates tmpls,
                                                JaniceTemplateIds ids,
                                                JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_gallery_reserve(JaniceGallery gallery,
                                                 size_t n);

JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                JaniceTemplate tmpl,
                                                JaniceTemplateId id);

JANICE_EXPORT JaniceError janice_gallery_insert_batch(JaniceGallery gallery,
                                                      JaniceTemplates tmpls,
                                                      JaniceTemplateIds ids);

JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                JaniceTemplateId id);

JANICE_EXPORT JaniceError janice_gallery_remove_batch(JaniceGallery gallery,
                                                      JaniceTemplateIds ids);

// This function prepares a gallery for search after it has been modified.
// Please see
// https://noblis.github.io/janice/api/gallery.html#janice-gallery-prepare
// for more information.
JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);

// I/O
JANICE_EXPORT JaniceError janice_serialize_gallery(JaniceGallery gallery,
                                                   JaniceBuffer* data,
                                                   size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_gallery(const JaniceBuffer data,
                                                     size_t len,
                                                     JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                              JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_write_gallery(JaniceGallery gallery,
                                               const char* filename);

// Cleanup
JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_clear_template_ids(JaniceTemplateIds* ids);

JANICE_EXPORT JaniceError janice_clear_template_ids_group(JaniceTemplateIdsGroup* group);

// ----------------------------------------------------------------------------
// Search

JANICE_EXPORT JaniceError janice_search(JaniceTemplate probe,
                                        JaniceGallery gallery,
                                        JaniceContext context,
                                        JaniceSimilarities* similarities,
                                        JaniceTemplateIds* ids);

JANICE_EXPORT JaniceError janice_search_batch(JaniceTemplates probes,
                                              JaniceGallery gallery,
                                              JaniceContext context,
                                              JaniceSimilaritiesGroup* similarities,
                                              JaniceTemplateIdsGroup* ids);

// ----------------------------------------------------------------------------
// Cluster

// Structs
typedef size_t JaniceClusterId;

struct JaniceClusterIds
{
    JaniceClusterId* ids;
    size_t length;
};

struct JaniceClusterIdsGroup
{
    JaniceClusterIds* group;
    size_t length;
};

typedef double JaniceClusterConfidence;

struct JaniceClusterConfidences
{
    JaniceClusterConfidence* confidences;
    size_t length;
};

struct JaniceClusterConfidencesGroup
{
    JaniceClusterConfidences* group;
    size_t length;
};

// Functions
JANICE_EXPORT JaniceError janice_cluster_media(JaniceMediaIterators medias,
                                               JaniceContext context,
                                               JaniceClusterIdsGroup* cluster_ids,
                                               JaniceClusterConfidencesGroup* cluster_confidences,
                                               JaniceTracksGroup* tracks);

JANICE_EXPORT JaniceError janice_cluster_templates(JaniceTemplates tmpls,
                                                   JaniceContext context,
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
