#ifndef JANICE_IO_H
#define JANICE_IO_H

#include <janice_error.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Image

typedef uint8_t* JaniceBuffer;

JANICE_EXPORT JaniceError janice_free_buffer(JaniceBuffer* buffer);

struct JaniceImageType
{
    uint32_t channels;
    uint32_t rows;
    uint32_t cols;

    JaniceBuffer data;
    bool owner;
};

typedef struct JaniceImageType* JaniceImage;
typedef const struct JaniceImageType* JaniceConstImage;

JANICE_EXPORT JaniceError janice_create_image(uint32_t channels,
                                              uint32_t rows,
                                              uint32_t cols,
                                              JaniceImage* image);

JANICE_EXPORT JaniceError janice_create_image_from_buffer(uint32_t channels,
                                                          uint32_t rows,
                                                          uint32_t cols,
                                                          const JaniceBuffer data,
                                                          JaniceImage* image);

JANICE_EXPORT JaniceError janice_image_access(JaniceConstImage image,
                                              uint32_t channel,
                                              uint32_t row,
                                              uint32_t col,
                                              uint8_t* value);

JANICE_EXPORT JaniceError janice_free_image(JaniceImage* image);

// ----------------------------------------------------------------------------
// Media Iterator

typedef struct JaniceMediaIteratorType* JaniceMediaIterator;

JANICE_EXPORT JaniceError janice_media_it_next(JaniceMediaIterator it,
                                               JaniceImage* image);

JANICE_EXPORT JaniceError janice_media_it_seek(JaniceMediaIterator it,
                                               uint32_t frame);

JANICE_EXPORT JaniceError janice_media_it_get(JaniceMediaIterator it,
                                              JaniceImage* image,
                                              uint32_t frame);

JANICE_EXPORT JaniceError janice_media_it_tell(JaniceMediaIterator it,
                                               uint32_t* frame);

JANICE_EXPORT JaniceError janice_free_media_iterator(JaniceMediaIterator* it);

// ----------------------------------------------------------------------------
// Media

struct JaniceMediaType
{
    char* filename;

    uint32_t channels;
    uint32_t rows;
    uint32_t cols;
    uint32_t frames;
};

typedef struct JaniceMediaType* JaniceMedia;
typedef const struct JaniceMediaType *JaniceConstMedia;
typedef JaniceMedia* JaniceMedias;
typedef JaniceConstMedia* JaniceConstMedias;

JANICE_EXPORT JaniceError janice_create_media(const char* filename,
                                              JaniceMedia* media);

JANICE_EXPORT JaniceError janice_media_get_iterator(JaniceConstMedia media,
                                                    JaniceMediaIterator* it);

JANICE_EXPORT JaniceError janice_free_media(JaniceMedia* media);

// ----------------------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_IO_H
