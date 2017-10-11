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

#ifdef __cplusplus
extern "C" {
#endif

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
    JANICE_BAD_ARGUMENT          , // An argument to a JanICE function is invalid 
    JANICE_OPEN_ERROR            , // Failed to open a file
    JANICE_READ_ERROR            , // Failed to read from a file
    JANICE_WRITE_ERROR           , // Failed to write to a file
    JANICE_PARSE_ERROR           , // Failed to parse a file
    JANICE_INVALID_MEDIA         , // Failed to decode a media file
    JANICE_OUT_OF_BOUNDS_ACCESS  , // Out of bounds access into a buffer
    JANICE_MEDIA_AT_END          , // Media object is at the end of its data
    JANICE_INVALID_ATTRIBUTE_KEY , // An invalid attribute key was provided
    JANICE_MISSING_ATTRIBUTE     , // A value for a valid attribute key is not present
    JANICE_DUPLICATE_ID          , // Template id already exists in a gallery
    JANICE_MISSING_ID            , // Template id can't be found
    JANICE_MISSING_FILE_NAME     , // An expected file name is not given
    JANICE_INCORRECT_ROLE        , // Incorrect template role
    JANICE_FAILURE_TO_SERIALIZE  , // Could not serialize a data structure
    JANICE_FAILURE_TO_DESERIALIZE, // Could not deserialize a data structure
    JANICE_NOT_IMPLEMENTED       , // Optional function return
    JANICE_NUM_ERRORS              // Utility to iterate over all errors
};

static inline const char* janice_error_to_string(JaniceError error)
{
    if      (error == JANICE_SUCCESS)                return "Success";
    else if (error == JANICE_UNKNOWN_ERROR)          return "Unknown error";
    else if (error == JANICE_UNKNOWN_ERROR)          return "Out of memory error";
    else if (error == JANICE_INVALID_SDK_PATH)       return "Invalid SDK path";
    else if (error == JANICE_BAD_SDK_CONFIG)         return "Bad SDK configuration";
    else if (error == JANICE_BAD_LICENSE)            return "Bad license";
    else if (error == JANICE_MISSING_DATA)           return "Missing required data";
    else if (error == JANICE_INVALID_GPU)            return "Invalid GPU";
    else if (error == JANICE_BAD_ARGUMENT)           return "An argument to a JanICE function is invalid";
    else if (error == JANICE_OPEN_ERROR)             return "Failed to open a file";
    else if (error == JANICE_READ_ERROR)             return "Failed to read from a file";
    else if (error == JANICE_WRITE_ERROR)            return "Failed to write to a file";
    else if (error == JANICE_PARSE_ERROR)            return "Failed to parse a file";
    else if (error == JANICE_INVALID_MEDIA)          return "Invalid media file";
    else if (error == JANICE_OUT_OF_BOUNDS_ACCESS)   return "Out of bounds access into a buffer";
    else if (error == JANICE_MEDIA_AT_END)           return "A media object has reached the end of its internal data";
    else if (error == JANICE_INVALID_ATTRIBUTE_KEY)  return "An invalid attribute key has been provided";
    else if (error == JANICE_MISSING_ATTRIBUTE)      return "A valid attribute key was provided but there is no value";
    else if (error == JANICE_DUPLICATE_ID)           return "Duplicate template ID in a gallery";
    else if (error == JANICE_MISSING_ID)             return "Missing template ID in a gallery";
    else if (error == JANICE_MISSING_FILE_NAME)      return "Missing a file name";
    else if (error == JANICE_INCORRECT_ROLE)         return "Incorrect template role";
    else if (error == JANICE_FAILURE_TO_SERIALIZE)   return "Unable to serialize an API object";
    else if (error == JANICE_FAILURE_TO_DESERIALIZE) return "Unable to deserialize an API object";
    else if (error == JANICE_NOT_IMPLEMENTED)        return "Optional function is not implemented";
    else if (error == JANICE_NUM_ERRORS)             return "The total number of errors. This shouldn't have been returned...";

    return "Unknown error code";
}

// ----------------------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif // JANICE_ERROR_H
