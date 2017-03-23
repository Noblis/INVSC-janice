#ifndef JANICE_FILE_IO_H
#define JANICE_FILE_IO_H

#include <janice_io.h>
#ifdef __cplusplus
extern "C" {
#endif

JANICE_EXPORT JaniceError janice_file_get_iterator(const char * _filename, JaniceMediaIterator *_it);

#ifdef __cplusplus
} // extern "C"
#endif


#endif


