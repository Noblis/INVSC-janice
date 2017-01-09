## JaniceError {: #JaniceError }
Every function in the JanICE *C* API returns an error code when executed. In the case of successful application <code>JANICE_SUCCESS</code> is returned, otherwise a code indicating the specific issue is returned. The error codes are:

Code | Description
--- | ---
<code>JANICE_SUCCESS</code>                | No error
<code>JANICE_UNKNOWN_ERROR</code>          | Catch all error code
<code>JANICE_OUT_OF_MEMORY</code>          | Out of memory error
<code>JANICE_INVALID_SDK_PATH</code>       | Invalid SDK location
<code>JANICE_BAD_SDK_CONFIG</code>         | Invalid SDK configuration
<code>JANICE_BAD_LICENSE</code>            | Incorrect license file
<code>JANICE_MISSING_DATA</code>           | Missing SDK data
<code>JANICE_INVALID_GPU</code>            | The GPU is not functioning
<code>JANICE_OPEN_ERROR</code>             | Failed to open a file
<code>JANICE_READ_ERROR</code>             | Failed to read from a file
<code>JANICE_WRITE_ERROR</code>            | Failed to write to a file
<code>JANICE_PARSE_ERROR</code>            | Failed to parse a file
<code>JANICE_INVALID_MEDIA</code>          | Failed to decode a media file
<code>JANICE_DUPLICATE_ID</code>           | Template id already exists in a gallery
<code>JANICE_MISSING_ID</code>             | Template id can't be found
<code>JANICE_MISSING_FILE_NAME</code>      | An expected file name is not given
<code>JANICE_INCORRECT_ROLE</code>         | Incorrect template role
<code>JANICE_FAILURE_TO_ENROLL</code>      | Could not construct a template
<code>JANICE_FAILURE_TO_SERIALIZE</code>   | Could not serialize a data structure
<code>JANICE_FAILURE_TO_DESERIALIZE</code> | Could not deserialize a data structure
<code>JANICE_NOT_IMPLEMENTED</code>        | Optional function return
<code>JANICE_NUM_ERRORS</code>             | Utility to iterate over all errors

## JaniceEnrollmentType {: #JaniceEnrollmentType }
Often times, the templates produced by algorithms will require different data for different use cases. The enrollment type indicates what the use case for the created template will be, allowing implementors to specialize their templates if they so desire. The use cases supported by the API are:

Type | Description
---|---
<code>Janice11Reference</code> | The template will be used as a reference template for 1 to 1 verification.
<code>Janice11Verification</code> | The template will be used for verification against a reference template in 1 to 1 verification.
<code>Janice1NProbe</code> | The template will be used as a probe in 1 to N search.
<code>Janice1NGallery</code> | The template will be enrolled into a gallery and searched against for 1 to N search.
<code>JaniceCluster</code> | The template will be used for clustering
