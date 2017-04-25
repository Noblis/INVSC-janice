#ifndef JANICE_IO_OPENCV_H
#define JANICE_IO_OPENCV_H

#include <janice_io.h>

#ifdef __cplusplus
extern "C" {
#endif

JANICE_EXPORT JaniceError janice_io_opencv_create_media_iterator(const char* filename,
                                                                 JaniceMediaIterator* it);

JANICE_EXPORT JaniceError janice_io_opencv_free_media_iterator(JaniceMediaIterator* it);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_IO_OPENCV_H

