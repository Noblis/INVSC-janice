#ifndef JANICE_H
#define JANICE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#define JANICE_VERSION_MAJOR 4
#define JANICE_VERSION_MINOR 0
#define JANICE_VERSION_PATCH 0

// ----------------------------------------------------------------------------
// Error Handling

enum JaniceError
{
    JANICE_SUCCESS = 0           , // No error
    JANICE_UNKNOWN_ERROR         , // Catch all error code
    JANICE_OUT_OF_MEMORY         , // Out of memory error
    JANICE_INVALID_SDK_PATH      , // Invalid SDK location
    JANICE_BAD_SDK_CONFIG        , // Invalid SDK configuration
    JANICE_BAD_LICENSE           , // Incorrect license file
    JANICE_MISSING_DATA          , // Missing SDK data
    JANICE_INVALID_GPU           , // The GPU is not functioning
    JANICE_OPEN_ERROR            , // Failed to open a file
    JANICE_READ_ERROR            , // Failed to read from a file
    JANICE_WRITE_ERROR           , // Failed to write to a file
    JANICE_PARSE_ERROR           , // Failed to parse a file
    JANICE_INVALID_MEDIA         , // Failed to decode a media file
    JANICE_DUPLICATE_ID          , // Template id already exists in a gallery
    JANICE_MISSING_ID            , // Template id can't be found
    JANICE_MISSING_FILE_NAME     , // An expected file name is not given
    JANICE_INCORRECT_ROLE        , // Incorrect template role
    JANICE_FAILURE_TO_ENROLL     , // Could not construct a template
    JANICE_FAILURE_TO_SERIALIZE  , // Could not serialize a data structure
    JANICE_FAILURE_TO_DESERIALIZE, // Could not deserialize a data structure
    JANICE_NOT_IMPLEMENTED       , // Optional function return
    JANICE_NUM_ERRORS              // Utility to iterate over all errors
};

// ----------------------------------------------------------------------------
// Initialization

JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                            const char* temp_path,
                                            const char* algorithm,
                                            const int gpu_dev);

// ----------------------------------------------------------------------------
// Utility

JANICE_EXPORT const char* janice_error_to_string(JaniceError error);

JANICE_EXPORT JaniceError janice_sdk_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);

// ----------------------------------------------------------------------------
// Training

JANICE_EXPORT JaniceError janice_train(const char* data_prefix,
                                       const char* data_list);

// ----------------------------------------------------------------------------
// Media I/O

typedef struct JaniceMediaType* JaniceMedia;
typedef const struct JaniceMediaType *JaniceConstMedia;

JANICE_EXPORT JaniceError janice_create_media(const char* filename,
                                              JaniceMedia* media);

JANICE_EXPORT JaniceError janice_free_media(JaniceMedia* media);

// ----------------------------------------------------------------------------
// Detection

struct JaniceRect
{
    uint32_t x, y, width, height;
};

struct JaniceDetectionInstance
{
    JaniceRect rect;
    uint32_t frame;
    double confidence;
};

typedef struct JaniceDetectionType* JaniceDetection;
typedef const struct JaniceDetectionType* JaniceConstDetection;

JANICE_EXPORT JaniceError janice_create_detection(JaniceConstMedia media,
                                                  const JaniceRect rect,
                                                  uint32_t frame,
                                                  JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_detect(JaniceConstMedia media,
                                        uint32_t min_object_size,
                                        JaniceDetection* detections,
                                        uint32_t* num_detections);

JANICE_EXPORT JaniceError janice_detection_get_instances(JaniceConstDetection detection,
                                                         JaniceDetectionInstance** instances,
                                                         uint32_t* num_instances);

JANICE_EXPORT JaniceError janice_serialize_detection(JaniceConstDetection detection,
                                                     unsigned char** data,
                                                     size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_detection(const unsigned char* data,
                                                       size_t len,
                                                       JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                JaniceDetection* detection);

JANICE_EXPORT JaniceError janice_write_detection(JaniceConstDetection detection,
                                                 const char* filename);

JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);

// ----------------------------------------------------------------------------
// Enrollment

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

JANICE_EXPORT JaniceError janice_create_template(JaniceConstDetection* detections,
                                                 uint32_t num_detections,
                                                 JaniceEnrollmentType role,
                                                 JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_serialize_template(JaniceConstTemplate tmpl,
                                                    unsigned char** data,
                                                    size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_template(const unsigned char** data,
                                                      size_t len,
                                                      JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                               JaniceTemplate* tmpl);

JANICE_EXPORT JaniceError janice_write_template(JaniceConstTemplate tmpl,
                                                const char* filename);

JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);

// ----------------------------------------------------------------------------
// Verification

JANICE_EXPORT JaniceError janice_verify(JaniceConstTemplate reference,
                                        JaniceConstTemplate verification,
                                        double* similarity);

// ----------------------------------------------------------------------------
// Gallery

typedef struct JaniceGalleryType* JaniceGallery;
typedef const struct JaniceGalleryType* JaniceConstGallery;

JANICE_EXPORT JaniceError janice_create_gallery(JaniceConstTemplate* tmpls,
                                                const uint32_t* ids,
                                                JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                JaniceConstTemplate tmpl,
                                                uint32_t id);

JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                uint32_t id);

JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);

JANICE_EXPORT JaniceError janice_serialize_gallery(JaniceConstGallery gallery,
                                                   unsigned char** data,
                                                   size_t* len);

JANICE_EXPORT JaniceError janice_deserialize_gallery(const unsigned char** data,
                                                     size_t len,
                                                     JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                              JaniceGallery* gallery);

JANICE_EXPORT JaniceError janice_write_gallery(JaniceConstGallery gallery,
                                               const char* filename);

JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);

// ----------------------------------------------------------------------------
// Search

JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                        JaniceConstGallery gallery,
                                        uint32_t num_requested,
                                        double** similarities,
                                        uint32_t** ids,
                                        uint32_t* num_returned);

// ----------------------------------------------------------------------------
// Cluster

struct JaniceClusterItem
{
    uint32_t cluster_id;
    uint32_t source_id;
    double confidence;
    JaniceDetection detection;
};

JANICE_EXPORT JaniceError janice_cluster_media(JaniceConstMedia* input,
                                               const uint32_t* input_ids,
                                               const uint32_t num_inputs,
                                               const uint32_t hint,
                                               JaniceClusterItem** clusters,
                                               uint32_t* num_clusters);

JANICE_EXPORT JaniceError janice_cluster_templates(JaniceConstTemplate* input,
                                                   const uint32_t* input_ids,
                                                   const uint32_t num_inputs,
                                                   const uint32_t hint,
                                                   JaniceClusterItem** clusters,
                                                   uint32_t* num_clusters);

// ----------------------------------------------------------------------------
// Finalize

JANICE_EXPORT JaniceError janice_finalize();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_H
