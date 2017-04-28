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
// Buffer

typedef uint8_t* JaniceBuffer;

JANICE_EXPORT JaniceError janice_free_buffer(JaniceBuffer* buffer);

// ----------------------------------------------------------------------------
// Image

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

inline uint8_t janice_image_access(JaniceConstImage image, uint32_t channel, uint32_t row, uint32_t col)
{
    return image->data[(row * image->cols * image->channels) + (col * image->channels) + channel];
}

// ----------------------------------------------------------------------------
// Media Iterator

typedef void* JaniceMediaIteratorState;

struct JANICE_EXPORT JaniceMediaIteratorType
{
    JaniceError (*next)(JaniceMediaIteratorType*, JaniceImage*);
    JaniceError (*seek)(JaniceMediaIteratorType*, uint32_t);
    JaniceError (* get)(JaniceMediaIteratorType*, JaniceImage*, uint32_t);
    JaniceError (*tell)(JaniceMediaIteratorType*, uint32_t*);

    JaniceError (*free_image)(JaniceImage*);
    JaniceError (*free)(JaniceMediaIteratorType**);

    JaniceMediaIteratorState _internal;
};

typedef struct JaniceMediaIteratorType* JaniceMediaIterator;
typedef JaniceMediaIterator* JaniceMediaIterators;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_IO_H
