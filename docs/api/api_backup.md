# JanICE C API

Mandatory interface for the JanICE API.

## Overview {: #Overview }
A JanICE application begins with a call to 
[janice_initialize](#JaniceInitialize).

Media (either an image or video) is loaded with 
[janice_create_media](#JaniceCreateMedia). Preprocessing on the images is 
done with [janice_create_detection](#JaniceCreateDetection) or 
[janice_detect](#JaniceDetect). Templates are created from 1 or more 
detections with [janice_create_template](#JaniceCreateTemplate). Templates 
are expected to be suitable for comparison after they are created. 
[janice_verify](#JaniceVerify) computes a 1-1 verification score. A gallery 
of templates can be created for search with 
[janice_create_gallery](#JaniceCreateGallery) and can be searched with 
[janice_search](#JaniceSearch). All JanICE objects can be serialized with 
janice_serialize_\* and deserialized with janice_deserialize_\*.

A JanICE application ends with a call to [janice_finalize](#JaniceFinalize).

## Computer Vision Concepts {: #ComputerVisionConcepts }

### Objects {: #Objects }

This API is built around opaque object pointers that represent high-level
computer vision concepts. To provide the most flexibility to implementors
of this API these objects are not defined in the API and should instead
be defined in the implementation. Access to these object is regulated
through defined accessor functions that implementors are either required
or optional as indicated in their documentation below.

### Media {: #Media }

This API seeks to define a common representation for both images and videos, 
which we call media. Implementors of this API are free to define media however 
they like using the [JaniceMediaType](#JaniceMediaType) structure. However, it is
strongly recommended that the implementation is lazy when loading videos into
memory. It is expected that this API will load multiple videos at once for
processing, for example during a call to 
[janice_cluster_media](#JaniceClusterMedia), which can overload system memory
if all frames are loaded at once. An example implementation has been provided
[here](../implementations/opencv_io/opencv_io.h) that uses the popular OpenCV
library.

### Detection {: #Detection }

In the context of this API, detection is used to refer to the identification of
objects of interest within a [media](#Media) object. Detections are represented
using the [JaniceDetectionType](#JaniceDetectionType) object which implementors
are free to define however they would like. For images, a detection is defined
as a rectangle that bounds an object of interest and an associated confidence
value. For video, a single object can exist in mutliple frames. A rectangle and
confidence are only relevant in a single frame. In this case, we define a
detection as a list of (rectangle, confidence) pairs that track a single object
through a video. It is not required that this list be dense however (i.e.
frames can be skipped). To support this, we extend our representation of a
detection to a (rectangle,confidence,frame) tuple where frame gives the index
of the frame the rectangle was found in.

### Feature Extraction {: #FeatureExtraction }

This API defines feature extraction as the process of turning 1 or more 
[detections](#Detection) that refer to the same object into a single 
representation. This representation is defined in the API using the
[JaniceTemplateType(#JaniceTemplateType) object. In some cases (e.g. face 
recognition) this model of [multiple detections] -> [single representation]
contradicts the current paradigm of [single detection] -> [single 
representation]. Implementors are free to implement whatever paradigm they 
choose internally (i.e. a JanICE template could be a simple list of single
detection templates) provided the [comparison](#Comparison) functions work
appropriately.

### Galleries {: #Galleries }



### Comparison {: #Comparison }

This API defines two possible types of comparisons, 1-1 and 1-many or 1-N.
These are represented by the [janice_verify](#JaniceVerify) and 
[janice_search](#janice_search) functions respectively. 

### Clustering {: #Clustering }

## Software Concepts {: #SoftwareConcepts }

### Error Handling {: #ErrorHandling }

The API handles errors using return codes. Valid return codes are defined 
[here](#JaniceError). In general, it is assumed that new memory is only
allocated if a function returns <code>JANICE_SUCCESS</code>. Therefore,
**implementors are REQUIRED to deallocate any memory allocated during a
function call if that function returns an error.**

### Memory Allocation {: #MemoryAllocation }

The API tries to be agnostic to different memory allocation strategies and 
memory allocators for all API objects. For convienience and brevity however,
functions that return arrays of values (for example 
[janice_search](#JaniceSearch)) must allocate those arrays using the 
[malloc](https://www-s.acm.illinois.edu/webmonkeys/book/c_guide/2.13.html#malloc)
function available in the C standard library. It is the responsibility of the 
user of the API to ensure that those arrays are deleted with corresponding
calls to 
[free](https://www-s.acm.illinois.edu/webmonkeys/book/c_guide/2.13.html#free).

In general, the API often passes unallocated pointers to functions for the
implementor to allocate appropriately. This is indicated if the type of a 
function input is <code>JaniceObject\*\*</code>, or in the case of a utility
<code>typedef</code> <code>JaniceTypedef\*</code>. It is considered a best 
practice for unallocated pointers to be initialized to <code>NULL</code> before
they are passed to a function, but this is not guaranteed. It is the 
responsibility of the users of the API to ensure that pointers do not point to
valid data before they are passed to functions in which they are modified, as
this would cause memory leaks.

### Thread Safety {: #ThreadSafety }
All functions are marked one of:

Type          | Description
------------- | -----------
Thread safe   | Can be called simultaneously from multiple threads, even when the invocations use shared data.
Reentrant     | Can be called simultaneously from multiple threads, but only if each invocation uses its own data.
Thread unsafe | Can not be called simultaneously from multiple threads.

### Compiling {: #Compiling }
Define <code>JANICE_LIBRARY</code> during compilation to export JanICE symbols.

### Versioning {: #Versioning }

This API follows the [semantic versioning](http://semver.org) paradigm. Each
released iteration is tagged with a major.minor.patch version. A change in
the major version indicates a breaking change. A change in the minor version
indicates a backwards-compatible change. A change in the patch version
indicates a backwards-compatible bug fix.

## Enumerations {: #Enumerations }

### JaniceError {: #JaniceError }
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

### JaniceEnrollmentType {: #JaniceEnrollmentType }
Often times, the templates produced by algorithms will require different data for different use cases. The enrollment type indicates what the use case for the created template will be, allowing implementors to specialize their templates if they so desire. The use cases supported by the API are:

Type | Description
---|---
<code>Janice11Reference</code> | The template will be used as a reference template for 1 to 1 verification.
<code>Janice11Verification</code> | The template will be used for verification against a reference template in 1 to 1 verification.
<code>Janice1NProbe</code> | The template will be used as a probe in 1 to N search.
<code>Janice1NGallery</code> | The template will be enrolled into a gallery and searched against for 1 to N search.
<code>JaniceCluster</code> | The template will be used for clustering

## Objects {: #Objects }

### JaniceMediaType {: #JaniceMediaType }

An opaque pointer to a struct that represents an image or video. See [the
paragraph on media](#Media) for more information. The API deals exclusively
with pointers to this object and provides the following typedefs for convienience

Name             | Definition
---------------- | ----------
JaniceMedia      | <code>typedef struct JaniceMediaType\*</code>
JaniceConstMedia | <code>typedef const struct JaniceMediaType\*</code>

### JaniceRect {: #JaniceRect }

A simple struct that represents a rectangle

#### Fields {: #JaniceRectFields }

Name   | Type     | Description
------ | -------- | -----------
x      | uint32_t | The x offset of the rectangle in pixels
y      | uint32_t | The y offset of the rectangle in pixels
width  | uint32_t | The width of the rectangle in pixels
height | uint32_t | The height of the rectangle in pixels

### JaniceDetectionInstance {: #JaniceDetectionInstance }

A single detection in an image or video, represented as a rectangle,
confidence, and frame number. See [the paragraph on detection](#Detection) for
information on why this structure is necessary.

#### Confidence {: #JaniceDetectionInstanceConfidence }

The confidence value indicates a likelihood that the rectangle actually bounds
an object of interest. It is **NOT** required to be a probability and often
only has meaning relative to other confidence values from the same algorithm.
The only restriction is that a larger confidence value indicates a greater
likelihood that the rectangle bounds an object.

#### Fields {: #JaniceDetectionInstanceFields }

Name       | Type                      | Description
---------- | ------------------------- | -----------
rect       | [JaniceRect](#JaniceRect) | The rectangle that bounds the object of interest
confidence | double                    | A likelihood that the rectangle bounds an object of interest. See [this description](#Confidence)
frame      | uint32_t                  | A frame index indicating which frame the rectangle came from. If the rectangle comes from an image this should be set to 0

### JaniceDetectionType {: #JaniceDetectionType }

An opaque pointer to a struct that represents a detection. See [the paragraph
on detection](#Detection) for more information. The API deals exclusively with
pointers to this object and provides the following typedefs for convienience.

Name                 | Definition
-------------------- | ----------
JaniceDetection      | <code>typedef struct JaniceDetectionType\*</code>
JaniceConstDetection | <code>typedef const struct JaniceDetectionType\*</code>

### JaniceTemplateType {: #JaniceTemplateType }

An opaque pointer to a struct that represents a template. See [the paragraph on
feature extraction](#FeatureExtraction) for more information. The API deals
exclusively with pointers to this object and provides the following typedefs 
for convienience.

Name                | Definition
------------------- | ----------
JaniceTemplate      | <code>typedef struct JaniceTemplateType\*</code>
JaniceConstTemplate | <code>typedef const struct JaniceTemplateType\*</code>

### JaniceGalleryType {: #JaniceGalleryType }

An opaque pointer to a struct that represents a gallery. See [the paragraph on
galleries](#Galleries) for more information. The API deals exclusively with
pointers to this object and provides the following typedefs for convienience.

Name               | Definition
------------------ | ----------
JaniceGallery      | <code>typedef struct JaniceGalleryType\*</code>
JaniceConstGallery | <code>typedef const struct JaniceGalleryType\*</code>

### JaniceClusterItem {: #JaniceClusterItem }



## Functions {: #Functions }

There are a number of use cases associated with this API. The meaning of the 
concepts this API tries to convey changes depending on the use case. In the
following documentation, if the use of the function changes depending on the 
use case, the description will clearly state what the use is for each relevant
use case. If the use of the function does not change, a single description will
be provided.

All API functions return an [error code](#JaniceError). Because it is universal
the return type is not stated in the following documentation.

### janice_initialize {: #JaniceInitialize }

Initialize global or shared state for the implementation. This function should
be called once at the start of the application, before making any other calls 
to the API.

#### Thread safety {: #JaniceInitializeThreadSafety }

This function is thread unsafe.

#### Parameters {: #JaniceInitializeParameters }

Name      | Type         | Description
--------- | ------------ | -----------
sdk_path  | const char\* | Path to a *read-only* directory containing the JanICE compliant SDK as specified by the implementor
temp_path | const char\* | Path to an existing empty *read-write* directory for use as temporary file storage by the implementation. This path must be guaranteed until [janice_finalize](#JaniceFinalize).
algorithm | const char\* | An empty string indicating the a default algorithm, or an implementation defined string containing an alternative configuration
gpu_dev   | int          | An index indicated a GPU device to use. If no GPU is available users should pass -1. If implementors do not offer a GPU solution they can ignore this value.


### janice_train {: #JaniceTrain }

Train an implementation using new data.

#### Thread safety {: #JaniceTrainThreadSafety }

This function is thread unsafe.

#### Parameters {: #JaniceTrainParameters }

Name        | Type         | Description
----------- | ------------ | -----------
data_prefix | const char\* | A prefix path pointing to the location of training data
data_train  | const char\* | A list of training data and labels. The format is currently unspecified

#### Notes {: #JaniceTrainNotes }

This function is untested, unstable and most likely subject to breaking changes
in future releases.


### janice_error_to_string {: #JaniceErrorToString }

Convert an [error code](#JaniceError) into a string for printing.

#### Thread safety {: #JaniceErrorToStringThreadSafety }

This function is thread safe.

#### Parameters {: #JaniceErrorToStringParameters }

Name  | Type                        | Description
----- | --------------------------- | -----------
error | [JaniceError](#JaniceError) | An error code

#### Return value {: #JaniceErrorToStringReturnValue }

This is the only function in the API that does not return 
[JaniceError](#JaniceError). It returns <code>const char\*</code> which is a 
null-terminated list of characters that describe the input error.


### janice_sdk_version {: #JaniceSDKVersion }

Query the implementation for the version of the API it was designed to
implement. See [the section on software versioning](#Versioning) for more
information on the versioning process for this API.

#### Thread Safety {: #JaniceSDKVersionThreadSafety }

This function is thread safe.

#### Parameters

Name  | Type       | Description
----- | ---------- | -----------
major | uint32_t\* | The supported major version of the API
minor | uint32_t\* | The supported minor version of the API
patch | uint32_t\* | The supported patch version of the API

### janice_create_media {: #JaniceCreateMedia }

Create a [JaniceMediaType](#JaniceMediaType) object from a file.

#### Thread safety {: #JaniceCreateMediaThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceCreateMediaParameters }

Name     | Type                              | Description
-------- | --------------------------------- | -----------
filename | const char\*                      | A path to an image or video on disk
media    | [JaniceMedia\*](#JaniceMediaType) | An uninitialized pointer to a media object.

#### Example {: #JaniceCreateMediaExample }

```
JaniceMedia media = NULL;
if (janice_create_media("example.media", &media) != JANICE_SUCCESS)
    // ERROR!
```

### janice_free_media {: #JaniceFreeMedia }

Free any memory associated with a [JaniceMediaType](#JaniceMediaType) object.

#### Thread safety {: #JaniceFreeMediaThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeMediaParameters }

Name  | Type                              | Description
----- | --------------------------------- | -----------
media | [JaniceMedia\*](#JaniceMediaType) | A media object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeMediaExample }

```
JaniceMedia media; // Where media is a valid media object created previously
if (janice_free_media(&media) != JANICE_SUCCESS)
    // ERROR!
```

### janice_create_detection {: #JaniceCreateDetection }

Create a detection from a known rectangle. This is useful if a human has
indentified an object of interest and would like to run subsequent API 
functions on it. In the case where the input media is a video the given 
rectangle is considered an initial sighting of the object of interest. The
implementation may detect additional sightings of the object in successive
frames.

#### Thread safety {: #JaniceCreateDetectionThreadSafety }

This function is reentrant.

#### Face Recognition {: #JaniceCreateDetectionFaceRecognition }

The provided rectangle will encapsulate a face in the media.

#### Camera Identification {: #JaniceCreateDetectionCameraIdentification }

The provided rectangle will encapsulate a region of interest in the media. Often the 
rectangle will simply border the entire image. Note that at this time camera
identification is only implemented for images and will return an error if the
media is a video. Because the implementation is image-only the value in the
frame parameter is ignored.

#### Parameters {: #JaniceCreateDetectionParameters }

Name      | Type                                      | Description
--------- | ----------------------------------------- | -----------
media     | [JaniceConstMedia](#JaniceMediaType)      | A media object to create the detection from
rect      | const [JaniceRect](#JaniceRect)           | A rectangle that bounds the object of interest
frame     | uint32_t                                  | An index to the frame in the media the rectangle refers to. If the media is an image this should be 0.
detection | [JaniceDetection\*](#JaniceDetectionType) | An uninitialized pointer to a detection object.

#### Example {: #JaniceCreateDetectionExample }

```
JaniceMedia media; // Where media is a valid media object created previously

JaniceRect rect; // Create a bounding rectangle around an object of interest
rect.x      = 10; // The rectangle should fall within the bounds of the media
rect.y      = 10; // This code assumes media width > 110 and media height > 110
rect.width  = 100;
rect.height = 100;

JaniceDetection detection = NULL; // best practice to initialize to NULL
if (janice_create_detection(media, rect, 0 /* frame */, &detection) != JANICE_SUCCESS)
    // ERROR!
```

### janice_detect {: #JaniceDetect }

Automatically detect objects in a media object. See [the paragraph on 
detection](#Detection) for an overview of detection in the context of this API.

#### Thread safety {: #JaniceDetectThreadSafety }

This function is reentrant.

#### Minimum Object Size {: #JaniceDetectMinimumObjectSize }

This function specifies a minimum object size as one of its parameters. This
value indicates the minimum size of objects that the user would like to see
detected. Often, increasing the minimum size can improve runtime of algorithms.
The size is in pixels and corresponds to the length of the smaller side of the
rectangle. This means a detection will be returned if and only if its smaller
side is larger than the value specified. If the user does not wish to specify a
minimum width 0 can be provided.

#### Face Recogntion {: #JaniceDetectFaceRecognition }

This function detects faces in the media.

#### Camera Identification {: #JaniceDetectCameraIdentification }

This function provides a list of size 1 where the only entry is a detection
with a rect that encapsulates the entire image.

#### Parameters {: #JaniceDetectParameters }

Name            | Type                                      | Description
--------------- | ----------------------------------------- | -----------
media           | [JaniceConstMedia](#JaniceMediaType)      | A media object to create the detection from.
min_object_size | uint32_t                                  | A minimum object size. See [Minimum Object Size](#Minimum_Object_Size)
detections      | [JaniceDetection\*](#JaniceDetectionType) | An uninitialized array to hold all of the detections detected in the media object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.
num_detections  | uint32_t\*                                | The number of detections detected

#### Example {: #JaniceDetectExample }

```
JaniceMedia media; // Where media is a valid media object created previously
const uint32_t min_object_size = 24; // Only find objects where the smaller
                                     // side is > 24 pixels
JaniceDetection* detections = NULL; // best practice to initialize to NULL
uint32_t num_detections; // Will be populated with the size of detections

if (janice_detect(media, min_object_size, &detections, &num_detections) != JANICE_SUCCESS)
    // ERROR!
```

### janice_detection_get_instances {: #JaniceDetectionGetInstances }

Get a list of the [JaniceDetectionInstances](#JaniceDetectionInstance) that
comprise the detection.

#### Thread Safety {: #JaniceDetectionGetInstancesThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceDetectionGetInstancesParameters }

Name          | Type                                                    | Description
------------- | ------------------------------------------------------- | -----------
detection     | [JaniceConstDetection](#JaniceDetectionType)            | A detection object to get instances from
instances     | [JaniceDetectionInstance\*\*](#JaniceDetectionInstance) | An uninitialized array to hold the instances that comprise the detection. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.
num_instances | uint32_t\*                                              | The number of instances that comprise the detection

#### Example {: #JaniceDetectionGetInstancesExample }

```
JaniceDetection detection; // Where detection is a valid detection object
                           // created previously
JaniceDetectionInstance* instances = NULL; // best practice to intialize to NULL
uint32_t num_instances; // Will be populated with the size of instances

if (janice_detection_get_instances(detection, &instances, &num_instances) != JANICE_SUCCESS)
    // ERROR!
```

### janice_serialize_detection {: #JaniceSerializeDetection }

Serialize a [JaniceDetection](#JaniceDetectionType) object to a flat buffer.

#### Thread Safety {: #JaniceSerializeDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSerializeDetectionParameters }

Name      | Type                                         | Description
--------- | -------------------------------------------- | -----------
detection | [JaniceConstDetection](#JaniceDetectionType) | A detection object to serialize
data      | unsigned char\*\*                            | An uninitialized buffer to hold the flattened data. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.
len       | size_t\*                                     | The length of the flat buffer after it is filled.

#### Example {: #JaniceSerializeDetectionExample }

```

JaniceDetection detection; // Where detection is a valid detection created
                           // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_detection(detection, &buffer, &buffer_len);
```

### janice_deserialize_detection {: #JaniceDeserializeDetection }

Deserialize a [JaniceDetection](#JaniceDetectionType) object from a flat buffer.

#### Thread Safety {: #JaniceDeserializeDetectionThreadSafety }

This function is reentrant.

#### Parameters

Name      | Type                                      | Description
--------- | ----------------------------------------- | -----------
data      | const unsigned char\*                     | A buffer containing data from a flattened detection object.
len       | size_t                                    | The length of the flat buffer
detection | [JaniceDetection\*](#JaniceDetectionType) | An uninitialized detection object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization. 

#### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.detection", "r");
fread(buffer, 1, buffer_len, file);

JaniceDetection detection = nullptr;
janice_deserialize_detection(buffer, buffer_len, detection);

fclose(file);
```

### janice_read_detection {: #JaniceReadDetection }

Read a detection from a file on disk. This method is functionally equivalent
to the following-

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.detection", "r");
fread(buffer, 1, buffer_len, file);

JaniceDetection detection = nullptr;
janice_deserialize_detection(buffer, buffer_len, detection);

fclose(file);
```

It is provided for memory efficiency and ease of use when reading from disk.

#### Thread Safety {: #JaniceReadDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceReadDetectionParameters }

Name      | Type                                      | Description
--------- | ----------------------------------------- | -----------
filename  | const char\*                              | The path to a file on disk
detection | [JaniceDetection\*](#JaniceDetectionType) | An uninitialized detection object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.

#### Example {: #JaniceReadDetectionExample }

```
JaniceDetection detection = NULL;
if (janice_read_detection("example.detection", &detection) != JANICE_SUCCESS)
    // ERROR!
```

### janice_write_detection {: #JaniceWriteDetection }

Write a detection to a file on disk. This method is functionally equivalent 
to the following-

```

JaniceDetection detection; // Where detection is a valid detection created
                           // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_detection(detection, &buffer, &buffer_len);

FILE* file = fopen("serialized.detection", "w+");
fwrite(buffer, 1, buffer_len, file);

fclose(file);
```

It is provided for memory efficiency and ease of use when writing to disk.

#### ThreadSafety {: #JaniceWriteDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceWriteDetectionParameters }

Name      | Type                                         | Description
--------- | -------------------------------------------- | -----------
detection | [JaniceConstDetection](#JaniceDetectionType) | The detection object to write to disk
filename  | const char\*                                 | The path to a file on disk

#### Example {: #JaniceWriteDetectionExample }

```
JaniceDetection detection; // Where detection is a valid detection created
                           // previously
if (janice_write_detection(detection, "example.detection") != JANICE_SUCCESS)
    // ERROR!
```

### janice_free_detection {: #JaniceFreeDetection }

Free any memory associated with a [JaniceDetectionType](#JaniceDetectionType) object.

#### Thread safety {: #JaniceFreeDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeDetectionParameters }

Name      | Type                                      | Description
--------- | ----------------------------------------- | -----------
detection | [JaniceDetection\*](#JaniceDetectionType) | A detection object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeDetectionExample }

```
JaniceDetection detection; // Where detection is a valid detection object 
                           // created previously
if (janice_free_detection(&detection) != JANICE_SUCCESS)
    // ERROR!
```

### janice_create_template {: #JaniceCreateTemplate }

Create a [JaniceTemplate](#JaniceTemplateType) object from an array of detections.

#### Thread Safety {: #JaniceCreateTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceCreateTemplateParameters }

Name           | Type                                           | Description
-------------- | ---------------------------------------------- | -----------
detections     | [JaniceConstDetection\*](#JaniceDetectionType) | An array of detection objects
num_detections | uint32_t                                       | The number of input detections
role           | [JaniceEnrollmentType](#JaniceEnrollmentType)  | The use case for the template
tmpl           | [JaniceTemplate\*](#JaniceTemplateType)        | An uninitialized template object.

#### Example {: #JaniceCreateTemplateExample }

```
JaniceDetection* detections; // Where detections is a valid array of valid 
                             // detection objects created previously
const uint32_t num_detections = K; // Where K is the number of detections in
                                   // the detections array
JaniceEnrollmentType role = Janice1NProbe; // This template will be used as a
                                           // probe in 1-N search
JaniceTemplate tmpl = NULL; // Best practice to initialize to NULL

if (janice_create_template(detections, num_detections, rolw, &tmpl) != JANICE_SUCCESS)
    // ERROR!
```

### janice_serialize_template {: #JaniceSerializeTemplate }

Serialize a [JaniceTemplate](#JaniceTemplateType) object to a flat buffer.

#### Thread Safety {: #JaniceSerializeTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSerializeTemplateParameters }

Name | Type                                       | Description
---- | ------------------------------------------ | -----------
tmpl | [JaniceConstTemplate](#JaniceTemplateType) | A template object to serialize
data | unsigned char\*\*                          | An uninitialized buffer to hold the flattened data. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.
len  | size_t\*                                   | The length of the flat buffer after it is filled.

#### Example {: #JaniceSerializeTemplateExample }

```

JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_template(tmpl, &buffer, &buffer_len);
```

### janice_deserialize_template {: #JaniceDeserializeTemplate }

Deserialize a [JaniceTemplate](#JaniceTemplateType) object from a flat buffer.

#### Thread Safety {: #JaniceDeserializeTemplateThreadSafety }

This function is reentrant.

#### Parameters

Name | Type                                    | Description
---- | --------------------------------------- | ---------
data | const unsigned char\*                   | A buffer containing data from a flattened template object.
len  | size_t                                  | The length of the flat buffer
tmpl | [JaniceTemplate\*](#JaniceTemplateType) | An uninitialized template object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization. 

#### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.template", "r");
fread(buffer, 1, buffer_len, file);

JaniceTemplate tmpl = NULL; // best practice to initialize to NULL
janice_deserialize_template(buffer, buffer_len, tmpl);

fclose(file);
```

### janice_read_template {: #JaniceReadTemplate }

Read a template from a file on disk. This method is functionally equivalent
to the following-

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.template", "r");
fread(buffer, 1, buffer_len, file);

JaniceTemplate tmpl = nullptr;
janice_deserialize_template(buffer, buffer_len, tmpl);

fclose(file);
```

It is provided for memory efficiency and ease of use when reading from disk.

#### Thread Safety {: #JaniceReadTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceReadTemplateParameters }

Name     | Type                                    | Description
-------- | --------------------------------------- | -----------
filename | const char\*                            | The path to a file on disk
tmpl     | [JaniceTemplate\*](#JaniceTemplateType) | An uninitialized template object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.

#### Example {: #JaniceReadTemplateExample }

```
JaniceTemplate tmpl = NULL;
if (janice_read_template("example.template", &tmpl) != JANICE_SUCCESS)
    // ERROR!
```

### janice_write_template {: #JaniceWriteTemplate }

Write a template to a file on disk. This method is functionally equivalent 
to the following-

```

JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_template(tmpl, &buffer, &buffer_len);

FILE* file = fopen("serialized.template", "w+");
fwrite(buffer, 1, buffer_len, file);

fclose(file);
```

It is provided for memory efficiency and ease of use when writing to disk.

#### ThreadSafety {: #JaniceWriteTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceWriteTemplateParameters }

Name     | Type                                       | Description
-------- | ------------------------------------------ | -----------
tmpl     | [JaniceConstTemplate](#JaniceTemplateType) | The template object to write to disk
filename | const char\*                               | The path to a file on disk

#### Example {: #JaniceWriteTemplateExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously
if (janice_write_template(tmpl, "example.template") != JANICE_SUCCESS)
    // ERROR!
```

### janice_free_template {: #JaniceFreeTemplate }

Free any memory associated with a [JaniceTemplateType](#JaniceTemplateType) object.

#### Thread safety {: #JaniceFreeTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeTemplateParameters }

Name | Type                                    | Description
---- | --------------------------------------- | -----------
tmpl | [JaniceTemplate\*](#JaniceTemplateType) | A template object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeTemplateExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created previously
if (janice_free_template(&tmpl) != JANICE_SUCCESS)
    // ERROR!
```

### janice_verify {: #JaniceVerify }

Compare two templates with the difference expressed as a similarity score.

#### Thread Safety {: #JaniceVerifyThreadSafety }

This function is reentrant.

#### Similarity Score {: #JaniceVerifySimilarityScore }

This API expects that the comparison of two templates results in a single value
that quantifies the similarity between them. A similarity score is constrained
by the following requirements:

    1. Higher scores indicate greater similarity
    2. Scores can be asymmetric. This mean verify(a, b) does not necessarily
       equal verify(b, a)

#### Parameters {: #JaniceVerifyParameters }

Name         | Type                                       | Description
------------ | ------------------------------------------ | -----------
reference    | [JaniceConstTemplate](#JaniceTemplateType) | A reference template. This template was created with the [Janice11Reference](#JaniceEnrollmentType) role.
verification | [JaniceConstTemplate](#JaniceTemplateType) | A verification template. This template was created with the [Janice11Verification](#JaniceEnrollmentType) role.
similarity   | double\*                                   | A similarity score. See [this section](#JaniceVerifySimilarityScore) for more information.

#### Example {: #JaniceVerifyExample }

```
JaniceTemplate reference; // Where reference is a valid template object created
                          // previously
JaniceTemplate verification; // Where verification is a valid template object
                             // created previously
double similarity;
if (janice_verify(reference, verification, &similarity) != JANICE_SUCCESS)
    // ERROR!
```

### janice_create_gallery {: #JaniceCreateGallery }

Create a [JaniceGalleryType](#JaniceGalleryType) object from a list of
templates and unique ids.

#### Thread Safety {: #JaniceCreateGalleryThreadSafety

This function is reentrant.

#### Parameters {: #JaniceCreateGalleryParameters }

Name    | Type                                         | Description
------- | -------------------------------------------- | -----------
tmpls   | [JaniceConstTemplate\*](#JaniceTemplateType) | An array of templates to add to the gallery
ids     | const uint32_t\*                             | A set of unique identifiers to associate with the input templates. The *ith* id corresponds to the *ith* input template.
gallery | [JaniceGallery\*](#JaniceGalleryType)        | An uninitialized gallery object.

#### Example {: #JaniceCreateGalleryExample }

```
JaniceTemplate* tmpls; // Where tmpls is a valid array of valid template
                       // objects created previously
uint32_t* ids; // Where ids is a valid array of unique unsigned integers that
               // is the same length as tmpls
JaniceGallery gallery = NULL; // best practice to initialize to NULL

if (janice_create_gallery(tmpls, ids, &gallery) != JANICE_SUCCESS)
    // ERROR!
```

### janice_gallery_insert {: #JaniceGalleryInsert }

Insert a template into a gallery object. The template data should be copied
into the gallery as the template may be deleted after this function.

#### Thread Safety {: #JaniceGalleryInsertThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceGalleryInsertParameters }

Name    | Type                                       | Description
------- | ------------------------------------------ | -----------
gallery | [JaniceGallery](#JaniceGalleryType)        | A gallery object to insert the template into.
tmpl    | [JaniceConstTemplate](#JaniceTemplateType) | A template object to insert into the gallery.
id      | uint32_t                                   | A unique id to associate with the input template

#### Example {: #JaniceGalleryInsertExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created
                     // previously
uint32_t id; // Where id is a unique integer to associate with tmpl. This
             // integer should not exist in the gallery
JaniceGallery gallery; // Where gallery is a valid gallery object created
                       // previously

if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
    // ERROR!
```

### janice_gallery_remove {: #JaniceGalleryRemove }

Remove a template from a gallery object using its unique id.

#### Thread Safety {: #JaniceGalleryRemoveThreadSafety }

This function is reentrant.

#### Parameters

Name | Type | Description
---- | ---- | -----------
gallery | [JaniceGallery](#JaniceGalleryType) | A gallery object to remove the template from
id | uint32_t | A unique id associated with a template in the gallery that indicates which template should be remove.

#### Example

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created
                     // previously
const uint32_t id = 0; // A unique integer id to associate with tmpl.

JaniceGallery gallery; // Where gallery is a valid gallery object created
                       // previously that does not have a template with id '0'
                       // already inserted in it.

// Insert the template with id 0
if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
    // ERROR!
    
// Now we can remove the template
if (janice_gallery_remove(gallery, id) != JANICE_SUCCESS)
    // ERROR!
```

### janice_gallery_prepare {: #JaniceGalleryPrepare }

Prepare a gallery for search. Implementors can use this function as an
opportunity to streamline gallery objects to accelerate the search process. The
calling convention for this function is **NOT** specified by the API, this
means that this function is not guaranteed to be called before
[janice_search](#JaniceSearch). It also means that templates can be added
to a gallery before and after this function is called. Implementations should
handle all of these calling conventions. However, users should be aware that
this function may be computationally expensive. They should strive to call it
only at critical junctions before search and as few times as possible overall.

#### Thread Safety {: #JaniceGalleryPrepareThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceGalleryPrepareParameters }

Name    | Type                                | Description
------- | ----------------------------------- | -----------
gallery | [JaniceGallery](#JaniceGalleryType) | A gallery object to prepare

#### Example {: #JaniceGalleryPrepareExample }

```
JaniceTemplate* tmpls; // Where tmpls is a valid array of valid template
                       // objects created previously
uint32_t* ids; // Where ids is a valid array of unique unsigned integers that
               // is the same length as tmpls
JaniceTemplate tmpl; // Where tmpl is a valid template object created
                     // previously
uint32_t id; // Where id is a unique integer id to associate with tmpl.

JaniceGallery gallery = NULL; // best practice to initialize to NULL

if (janice_create_gallery(tmpls, ids, &gallery) != JANICE_SUCCESS)
    // ERROR!

// It is valid to run search without calling prepare
if (janice_search(tmpl, gallery ... ) != JANICE_SUCCESS)
    // ERROR!

// Prepare can be called after search
if (janice_gallery_prepare(gallery) != JANICE_SUCCESS)
    // ERROR!

// Search can be called again right after prepare
if (janice_search(tmpl, gallery ... ) != JANICE_SUCCESS)
    // ERROR!

// Insert another template into the gallery. This is valid after the gallery
// has been prepared
if (janice_gallery_insert(gallery, tmpl, 112) != JANICE_SUCCESS)
    // ERROR!

// Prepare the gallery again
if (janice_gallery_prepare(gallery) != JANICE_SUCCESS)
    // ERROR!
```

### janice_serialize_gallery {: #JaniceSerializeGallery }

Serialize a [JaniceGallery](#JaniceGalleryType) object to a flat buffer.

#### Thread Safety {: #JaniceSerializeGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSerializeGalleryParameters }

Name    | Type                                     | Description
------- | ---------------------------------------- | -----------
gallery | [JaniceConstGallery](#JaniceGalleryType) | A gallery object to serialize
data    | unsigned char\*\*                        | An uninitialized buffer to hold the flattened data. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.
len     | size_t\*                                 | The length of the flat buffer after it is filled.

#### Example {: #JaniceSerializeGalleryExample }

```

JaniceGallery gallery; // Where gallery is a valid gallery created
                       // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_gallery(gallery, &buffer, &buffer_len);
```

### janice_deserialize_gallery {: #JaniceDeserializeGallery }

Deserialize a [JaniceGallery](#JaniceGalleryType) object from a flat buffer.

#### Thread Safety {: #JaniceDeserializeGalleryThreadSafety }

This function is reentrant.

#### Parameters

Name    | Type                                  | Description
------- | ------------------------------------- | ---------
data    | const unsigned char\*                 | A buffer containing data from a flattened gallery object.
len     | size_t                                | The length of the flat buffer
gallery | [JaniceGallery\*](#JaniceGalleryType) | An uninitialized gallery object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization. 

#### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.gallery", "r");
fread(buffer, 1, buffer_len, file);

JaniceGallery gallery = NULL; // best practice to initialize to NULL
janice_deserialize_gallery(buffer, buffer_len, gallery);

fclose(file);
```

### janice_read_gallery {: #JaniceReadGallery }

Read a gallery from a file on disk. This method is functionally equivalent
to the following-

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.gallery", "r");
fread(buffer, 1, buffer_len, file);

JaniceGallery gallery = NULL; // best practice to initialize to NULL
janice_deserialize_gallery(buffer, buffer_len, gallery);

fclose(file);
```

It is provided for memory efficiency and ease of use when reading from disk.

#### Thread Safety {: #JaniceReadGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceReadGalleryParameters }

Name     | Type                                  | Description
-------- | ------------------------------------- | -----------
filename | const char\*                          | The path to a file on disk
gallery  | [JaniceGallery\*](#JaniceGalleryType) | An uninitialized gallery object. See [the section on memory allocation](#MemoryAllocation) for requirements for initialization.

#### Example {: #JaniceReadGalleryExample }

```
JaniceGallery gallery = NULL;
if (janice_read_gallery("example.gallery", &gallery) != JANICE_SUCCESS)
    // ERROR!
```

### janice_write_gallery {: #JaniceWriteGallery }

Write a gallery to a file on disk. This method is functionally equivalent 
to the following-

```

JaniceGallery gallery; // Where gallery is a valid gallery created previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_gallery(gallery, &buffer, &buffer_len);

FILE* file = fopen("serialized.gallery", "w+");
fwrite(buffer, 1, buffer_len, file);

fclose(file);
```

It is provided for memory efficiency and ease of use when writing to disk.

#### ThreadSafety {: #JaniceWriteGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceWriteGalleryParameters }

Name     | Type                                     | Description
-------- | ---------------------------------------- | -----------
gallery  | [JaniceConstGallery](#JaniceGalleryType) | The gallery object to write to disk
filename | const char\*                             | The path to a file on disk

#### Example {: #JaniceWriteGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery created previously
if (janice_write_gallery(gallery, "example.gallery") != JANICE_SUCCESS)
    // ERROR!
```

### janice_free_gallery {: #JaniceFreeGallery }

Free any memory associated with a [JaniceGalleryType](#JaniceGalleryType) object.

#### Thread safety {: #JaniceFreeGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeGalleryParameters }

Name    | Type                                  | Description
------- | ------------------------------------- | -----------
gallery | [JaniceGallery\*](#JaniceGalleryType) | A gallery object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery object created previously
if (janice_free_gallery(&gallery) != JANICE_SUCCESS)
    // ERROR!
```

### janice_search {: #JaniceSearch }

Compute 1-N search results between a query template object and a target gallery
object. The function allocates two arrays of equal size, one containing 
[similarity scores](#JaniceVerifySimilarityScore) and the other containing the
unique id of the template the score was computed with (along with the query).
Often it is desirable (and perhaps computationally efficient) to only see the
top K scores out of N possible templates. The option to set a K is provided to
the user as part of the function parameters.

#### Thread Safety {: #JaniceSearchThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSearchParameters }

Name | Type | Description
---- | ---- | -----------
probe | [JaniceConstTemplate](#JaniceTemplateType) | A template object to use as a query
gallery | [JaniceConstGallery](#JaniceGalleryType) | A gallery object to search against
num_requested | uint32_t | The number of requested returns. If the user would like as many returns as there are templates in the gallery they can pass this parameter with the value 0.
similarities | double\*\* | An array of [similarity scores](#JaniceVerifySimilarityScore). The scores are expected to be sorted in descending order (i.e. the highest scores is stored at index 0).
ids | uint32_t\*\* | An array of unique ids identifying the target templates associated with the similarity scores. This array must be the same size as the similarities array. The *ith* id corresponds with the *ith* similarity.
num_returned | uint32_t\* | The number of elements in the similarities and ids arrays. This number can be different from **num_requested**. 

#### Example {: #JaniceSearchExample }

```
JaniceTemplate probe;  // Where probe is a valid template object created
                       // previously
JaniceGallery gallery; // Where gallery is a valid gallery object created
                       // previously
const uint32_t num_requested = 50; // Request the top 50 matches

double* similarities = NULL;
uint32_t* ids = NULL;
uint32_t num_returned;

// Run search
if (janice_search(probe, gallery, num_requested, &similarities, &ids, &num_returned) != JANICE_SUCCESS)
    // ERROR!

num_requested == num_returned; // This might not be true!
```

### janice_cluster_media {: #JaniceClusterMedia }

### janice_cluster_templates {: #JaniceClusterTemplates }

### janice_finalize {: #JaniceFinalize }

Destroy any resources created by [janice_initialize](#JaniceInitialize) and
finalize the application. This should be called once after all other API calls.

#### Thread Safety

This function is thread unsafe.
