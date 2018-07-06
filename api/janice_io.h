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

struct JaniceImage
{
    uint32_t channels;
    uint32_t rows;
    uint32_t cols;

    uint8_t* data;
    bool owner;
};

// ----------------------------------------------------------------------------
// Media Iterator

typedef void* JaniceMediaIteratorState;

struct JANICE_EXPORT JaniceMediaIterator
{
    JaniceError (*is_video)(const JaniceMediaIterator*, bool*);
    JaniceError (*get_frame_rate)(const JaniceMediaIterator*, float*);
    JaniceError (*get_physical_frame_rate)(const JaniceMediaIterator*, float*);

    JaniceError (*next)(const JaniceMediaIterator*, JaniceImage*);
    JaniceError (*seek)(const JaniceMediaIterator*, uint32_t);
    JaniceError (* get)(const JaniceMediaIterator*, JaniceImage*, uint32_t);
    JaniceError (*tell)(const JaniceMediaIterator*, uint32_t*);
    JaniceError (*reset)(const JaniceMediaIterator*);

    JaniceError (*physical_frame)(const JaniceMediaIterator*, uint32_t, uint32_t *);

    JaniceError (*free_image)(JaniceImage*);
    JaniceError (*free)(JaniceMediaIterator*);

    JaniceMediaIteratorState _internal;
};

struct JaniceMediaIterators
{
    JaniceMediaIterator* media;
    uint32_t length;
};

struct JaniceMediaIteratorsGroup
{
    JaniceMediaIterators* group;
    size_t length;
};


#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_IO_H
