#ifndef JANICE_ERROR_H
#define JANICE_ERROR_H

// ----------------------------------------------------------------------------
// Export Symbols

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

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Error Handling

  // taa: this hack creates the enum and an array containing the names of the enums.
  // We handle JANICE_SUCCESS and JANICE_NUM_ERRORS somewhat separately.
  
#define JANICE_ERROR_ENUM \
  X(JANICE_UNKNOWN_ERROR, "Unknown error")  /* Catch all error code */   \
  X(JANICE_INTERNAL_ERROR, "Internal SDK error")                         \
  X(JANICE_OUT_OF_MEMORY, "Out of memory error")                         \
  X(JANICE_INVALID_SDK_PATH, "Invalid SDK path")                         \
  X(JANICE_BAD_SDK_CONFIG, "Bad SDK configuration")                      \
  X(JANICE_BAD_LICENSE, "Bad license")                                   \
  X(JANICE_MISSING_DATA, "Missing required data")  /* Missing SDK data */\
  X(JANICE_INVALID_GPU, "Invalid GPU")  /* The GPU is not functioning */ \
  X(JANICE_BAD_ARGUMENT, "An argument to a JanICE function is invalid")  \
  X(JANICE_OPEN_ERROR, "Failed to open a file")                          \
  X(JANICE_READ_ERROR, "Failed to read from a file")                     \
  X(JANICE_WRITE_ERROR, "Failed to write to a file")                     \
  X(JANICE_PARSE_ERROR, "Failed to parse a file")                        \
  X(JANICE_INVALID_MEDIA, "Invalid media file")  /* Failed to decode a media file */  \
  X(JANICE_OUT_OF_BOUNDS_ACCESS, "Out of bounds access into a buffer")   \
  X(JANICE_MEDIA_AT_END, "A media object reached the end of its internal data") \
  X(JANICE_INVALID_ATTRIBUTE_KEY, "An invalid attribute key was provided") \
  X(JANICE_MISSING_ATTRIBUTE, "A value for a valid attribute key is not present") \
  X(JANICE_DUPLICATE_ID, "Duplicate template ID in a gallery")           \
  X(JANICE_MISSING_ID, "Missing template ID in a gallery")               \
  X(JANICE_MISSING_FILE_NAME, "Missing a file name")                     \
  X(JANICE_INCORRECT_ROLE, "Incorrect template role")                    \
  X(JANICE_FAILURE_TO_SERIALIZE, "Unable to serialize an API object")    \
  X(JANICE_FAILURE_TO_DESERIALIZE, "Unable to deserialize an API object") \
  X(JANICE_BATCH_ABORTED_EARLY, "Batch call aborted early due to error") \
  X(JANICE_BATCH_FINISHED_WITH_ERRORS, "Batch call finished, but with errors") \
  X(JANICE_CALLBACK_EXIT_IMMEDIATELY, "A callback requested its parent exit immediately") \
  X(JANICE_NOT_IMPLEMENTED, "Optional function is not implemented")

  

enum JaniceError
{
#  define X(a,b) a,
  JANICE_SUCCESS = 0,
  JANICE_ERROR_ENUM
#  undef X
  JANICE_NUM_ERRORS // Utility to iterate over all errors
};

char const *const janice_error_names[] = {
#  define X(a,b) #a,
  "JANICE_SUCCESS",
  JANICE_ERROR_ENUM
#  undef X
  0
};

char const *const janice_error_strings[] = {
#  define X(a,b) b,
  "Success",
  JANICE_ERROR_ENUM
#  undef X
  "The total number of errors. This shouldn't have been returned..."
};

struct JaniceErrors
{
    JaniceError* errors;
    size_t length;
};

JANICE_EXPORT JaniceError janice_clear_errors(JaniceErrors* errors);

static inline const char* janice_error_to_string(JaniceError error)
{
  if (error < JANICE_SUCCESS || error > JANICE_NUM_ERRORS) {
    return "Unknown error code";
  }
  return janice_error_strings[error];
}


// ----------------------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_ERROR_H
