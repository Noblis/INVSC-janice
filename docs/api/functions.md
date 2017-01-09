# Overview

There are a number of use cases associated with this API. The meaning of the
concepts this API tries to convey changes depending on the use case. In the
following documentation, if the use of the function changes depending on the
use case, the description will clearly state what the use is for each relevant
use case. If the use of the function does not change, a single description will
be provided.

All API functions return an [error code](enums.md#JaniceError). Because it is universal
the return type is not stated in the following documentation.

## janice_initialize {: #JaniceInitialize }

Initialize global or shared state for the implementation. This function should
be called once at the start of the application, before making any other calls
to the API.

### Signature {: #JaniceInitializeSignature }

```
JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                            const char* temp_path,
                                            const char* algorithm,
                                            const int gpu_dev);
```

### Thread Safety {: #JaniceInitializeThreadSafety }

This function is thread unsafe.

### Parameters {: #JaniceInitializeParameters }

Name      | Type         | Description
--------- | ------------ | -----------
sdk_path  | const char\* | Path to a *read-only* directory containing the JanICE compliant SDK as specified by the implementor
temp_path | const char\* | Path to an existing empty *read-write* directory for use as temporary file storage by the implementation. This path must be guaranteed until [janice_finalize](#JaniceFinalize).
algorithm | const char\* | An empty string indicating the a default algorithm, or an implementation defined string containing an alternative configuration
gpu_dev   | int          | An index indicated a GPU device to use. If no GPU is available users should pass -1. If implementors do not offer a GPU solution they can ignore this value.


## janice_train {: #JaniceTrain }

Train an implementation using new data.

### Signature {: #JaniceTrainSignature }
```
JANICE_EXPORT JaniceError janice_train(const char* data_prefix,
                                       const char* data_list);
```

### Thread Safety {: #JaniceTrainThreadSafety }

This function is thread unsafe.

### Parameters {: #JaniceTrainParameters }

Name        | Type         | Description
----------- | ------------ | -----------
data_prefix | const char\* | A prefix path pointing to the location of training data
data_train  | const char\* | A list of training data and labels. The format is currently unspecified

### Notes {: #JaniceTrainNotes }

This function is untested, unstable and most likely subject to breaking changes
in future releases.


## janice_error_to_string {: #JaniceErrorToString }

Convert an [error code](enums.md#JaniceError) into a string for printing.

### Signature {: #JaniceErrorToStringSignature }
```
JANICE_EXPORT const char* janice_error_to_string(JaniceError error);
```

### Thread Safety {: #JaniceErrorToStringThreadSafety }

This function is thread safe.

### Parameters {: #JaniceErrorToStringParameters }

Name  | Type                                | Description
----- | ----------------------------------- | -----------
error | [JaniceError](enums.md#JaniceError) | An error code

### Return Value {: #JaniceErrorToStringReturnValue }

This is the only function in the API that does not return
[JaniceError](enums.md#JaniceError). It returns <code>const char\*</code> which is a
null-terminated list of characters that describe the input error.


## janice_sdk_version {: #JaniceSDKVersion }

Query the implementation for the version of the API it was designed to
implement. See [the section on software versioning](software_concepts.md#Versioning) for more
information on the versioning process for this API.

### Signature {: #JaniceSDKVersionSignature }
```
JANICE_EXPORT JaniceError janice_sdk_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);
```

### Thread Safety {: #JaniceSDKVersionThreadSafety }

This function is thread safe.

### Parameters

Name  | Type       | Description
----- | ---------- | -----------
major | uint32_t\* | The supported major version of the API
minor | uint32_t\* | The supported minor version of the API
patch | uint32_t\* | The supported patch version of the API

## janice_create_media {: #JaniceCreateMedia }

Create a [JaniceMediaType](objects.md#JaniceMediaType) object from a file.

### Signature {: #JaniceCreateMediaSignature }
```
JANICE_EXPORT JaniceError janice_create_media(const char* filename,
                                              JaniceMedia* media);
```

### Thread Safety {: #JaniceCreateMediaThreadSafety }

This function is reentrant.

### Parameters {: #JaniceCreateMediaParameters }

Name     | Type                                        | Description
-------- | ------------------------------------------- | -----------
filename | const char\*                                | A path to an image or video on disk
media    | [JaniceMedia\*](objects.md#JaniceMediaType) | An uninitialized pointer to a media object.

### Example {: #JaniceCreateMediaExample }

```
JaniceMedia media = NULL;
if (janice_create_media("example.media", &media) != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_media {: #JaniceFreeMedia }

Free any memory associated with a [JaniceMediaType](objects.md#JaniceMediaType) object.

### Signature {: #JaniceFreeMediaSignature }
```
JANICE_EXPORT JaniceError janice_free_media(JaniceMedia* media);
```

### Thread Safety {: #JaniceFreeMediaThreadSafety }

This function is reentrant.

### Parameters {: #JaniceFreeMediaParameters }

Name  | Type                                        | Description
----- | ------------------------------------------- | -----------
media | [JaniceMedia\*](objects.md#JaniceMediaType) | A media object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

### Example {: #JaniceFreeMediaExample }

```
JaniceMedia media; // Where media is a valid media object created previously
if (janice_free_media(&media) != JANICE_SUCCESS)
    // ERROR!
```

## janice_create_detection {: #JaniceCreateDetection }

Create a detection from a known rectangle. This is useful if a human has
indentified an object of interest and would like to run subsequent API
functions on it. In the case where the input media is a video the given
rectangle is considered an initial sighting of the object of interest. The
implementation may detect additional sightings of the object in successive
frames.

### Signature {: #JaniceCreateDetectionSignature }
```
JANICE_EXPORT JaniceError janice_create_detection(JaniceConstMedia media,
                                                  const JaniceRect rect,
                                                  uint32_t frame,
                                                  JaniceDetection* detection);
```

### Thread Safety {: #JaniceCreateDetectionThreadSafety }

This function is reentrant.

### Face Recognition {: #JaniceCreateDetectionFaceRecognition }

The provided rectangle will encapsulate a face in the media.

### Camera Identification {: #JaniceCreateDetectionCameraIdentification }

The provided rectangle will encapsulate a region of interest in the media. Often the
rectangle will simply border the entire image. Note that at this time camera
identification is only implemented for images and will return an error if the
media is a video. Because the implementation is image-only the value in the
frame parameter is ignored.

### Parameters {: #JaniceCreateDetectionParameters }

Name      | Type                                                | Description
--------- | --------------------------------------------------- | -----------
media     | [JaniceConstMedia](objects.md#JaniceMediaType)      | A media object to create the detection from
rect      | const [JaniceRect](objects.md#JaniceRect)           | A rectangle that bounds the object of interest
frame     | uint32_t                                            | An index to the frame in the media the rectangle refers to. If the media is an image this should be 0.
detection | [JaniceDetection\*](objects.md#JaniceDetectionType) | An uninitialized pointer to a detection object.

### Example {: #JaniceCreateDetectionExample }

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

## janice_detect {: #JaniceDetect }

Automatically detect objects in a media object. See [the paragraph on
detection](cv_concepts.md#Detection) for an overview of detection in the context of this API.

### Signature {: #JaniceDetectSignature }
```
JANICE_EXPORT JaniceError janice_detect(JaniceConstMedia media,
                                        uint32_t min_object_size,
                                        JaniceDetection* detections,
                                        uint32_t* num_detections);
```

### Thread Safety {: #JaniceDetectThreadSafety }

This function is reentrant.

### Minimum Object Size {: #JaniceDetectMinimumObjectSize }

This function specifies a minimum object size as one of its parameters. This
value indicates the minimum size of objects that the user would like to see
detected. Often, increasing the minimum size can improve runtime of algorithms.
The size is in pixels and corresponds to the length of the smaller side of the
rectangle. This means a detection will be returned if and only if its smaller
side is larger than the value specified. If the user does not wish to specify a
minimum width 0 can be provided.

### Face Recogntion {: #JaniceDetectFaceRecognition }

This function detects faces in the media.

### Camera Identification {: #JaniceDetectCameraIdentification }

This function provides a list of size 1 where the only entry is a detection
with a rect that encapsulates the entire image.

### Parameters {: #JaniceDetectParameters }

Name            | Type                                                | Description
--------------- | --------------------------------------------------- | -----------
media           | [JaniceConstMedia](objects.md#JaniceMediaType)      | A media object to create the detection from.
min_object_size | uint32_t                                            | A minimum object size. See [Minimum Object Size](#JaniceDetectMinimumObjectSize)
detections      | [JaniceDetection\*](objects.md#JaniceDetectionType) | An uninitialized array to hold all of the detections detected in the media object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.
num_detections  | uint32_t\*                                          | The number of detections detected

### Example {: #JaniceDetectExample }

```
JaniceMedia media; // Where media is a valid media object created previously
const uint32_t min_object_size = 24; // Only find objects where the smaller
                                     // side is > 24 pixels
JaniceDetection* detections = NULL; // best practice to initialize to NULL
uint32_t num_detections; // Will be populated with the size of detections

if (janice_detect(media, min_object_size, &detections, &num_detections) != JANICE_SUCCESS)
    // ERROR!
```

## janice_detection_get_instances {: #JaniceDetectionGetInstances }

Get a list of the [JaniceDetectionInstances](objects.md#JaniceDetectionInstance) that
comprise the detection.

### Signature {: #JaniceDetectionGetInstancesSignature }
```
JANICE_EXPORT JaniceError janice_detection_get_instances(JaniceConstDetection detection,
                                                         JaniceDetectionInstance** instances,
                                                         uint32_t* num_instances);
```

### Thread Safety {: #JaniceDetectionGetInstancesThreadSafety }

This function is reentrant.

### Parameters {: #JaniceDetectionGetInstancesParameters }

Name          | Type                                                              | Description
------------- | ----------------------------------------------------------------- | -----------
detection     | [JaniceConstDetection](objects.md#JaniceDetectionType)            | A detection object to get instances from
instances     | [JaniceDetectionInstance\*\*](objects.md#JaniceDetectionInstance) | An uninitialized array to hold the instances that comprise the detection. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.
num_instances | uint32_t\*                                                        | The number of instances that comprise the detection

### Example {: #JaniceDetectionGetInstancesExample }

```
JaniceDetection detection; // Where detection is a valid detection object
                           // created previously
JaniceDetectionInstance* instances = NULL; // best practice to intialize to NULL
uint32_t num_instances; // Will be populated with the size of instances

if (janice_detection_get_instances(detection, &instances, &num_instances) != JANICE_SUCCESS)
    // ERROR!
```

## janice_serialize_detection {: #JaniceSerializeDetection }

Serialize a [JaniceDetection](objects.md#JaniceDetectionType) object to a flat buffer.

### Signature {: #JaniceSerializeDetectionSignature }
```
JANICE_EXPORT JaniceError janice_serialize_detection(JaniceConstDetection detection,
                                                     unsigned char** data,
                                                     size_t* len);
```

### Thread Safety {: #JaniceSerializeDetectionThreadSafety }

This function is reentrant.

### Parameters {: #JaniceSerializeDetectionParameters }

Name      | Type                                                   | Description
--------- | ------------------------------------------------------ | -----------
detection | [JaniceConstDetection](objects.md#JaniceDetectionType) | A detection object to serialize
data      | unsigned char\*\*                                      | An uninitialized buffer to hold the flattened data. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.
len       | size_t\*                                               | The length of the flat buffer after it is filled.

### Example {: #JaniceSerializeDetectionExample }

```

JaniceDetection detection; // Where detection is a valid detection created
                           // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_detection(detection, &buffer, &buffer_len);
```

## janice_deserialize_detection {: #JaniceDeserializeDetection }

Deserialize a [JaniceDetection](objects.md#JaniceDetectionType) object from a flat buffer.

### Signature {: #JaniceDeserializeDetectionSignature }
```
JANICE_EXPORT JaniceError janice_deserialize_detection(const unsigned char* data,
                                                       size_t len,
                                                       JaniceDetection* detection);
```

### Thread Safety {: #JaniceDeserializeDetectionThreadSafety }

This function is reentrant.

### Parameters

Name      | Type                                                | Description
--------- | --------------------------------------------------- | -----------
data      | const unsigned char\*                               | A buffer containing data from a flattened detection object.
len       | size_t                                              | The length of the flat buffer
detection | [JaniceDetection\*](objects.md#JaniceDetectionType) | An uninitialized detection object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.

### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.detection", "r");
fread(buffer, 1, buffer_len, file);

JaniceDetection detection = nullptr;
janice_deserialize_detection(buffer, buffer_len, detection);

fclose(file);
```

## janice_read_detection {: #JaniceReadDetection }

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

### Signature {: #JaniceReadDetectionSignature }
```
JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                JaniceDetection* detection);
```

### Thread Safety {: #JaniceReadDetectionThreadSafety }

This function is reentrant.

### Parameters {: #JaniceReadDetectionParameters }

Name      | Type                                                | Description
--------- | --------------------------------------------------- | -----------
filename  | const char\*                                        | The path to a file on disk
detection | [JaniceDetection\*](objects.md#JaniceDetectionType) | An uninitialized detection object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.

### Example {: #JaniceReadDetectionExample }

```
JaniceDetection detection = NULL;
if (janice_read_detection("example.detection", &detection) != JANICE_SUCCESS)
    // ERROR!
```

## janice_write_detection {: #JaniceWriteDetection }

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

### Signature {: #JaniceWriteDetectionSignature }
```
JANICE_EXPORT JaniceError janice_write_detection(JaniceConstDetection detection,
                                                 const char* filename);
```

### ThreadSafety {: #JaniceWriteDetectionThreadSafety }

This function is reentrant.

### Parameters {: #JaniceWriteDetectionParameters }

Name      | Type                                                   | Description
--------- | ------------------------------------------------------ | -----------
detection | [JaniceConstDetection](objects.md#JaniceDetectionType) | The detection object to write to disk
filename  | const char\*                                           | The path to a file on disk

### Example {: #JaniceWriteDetectionExample }

```
JaniceDetection detection; // Where detection is a valid detection created
                           // previously
if (janice_write_detection(detection, "example.detection") != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_detection {: #JaniceFreeDetection }

Free any memory associated with a [JaniceDetectionType](objects.md#JaniceDetectionType) object.

### Signature {: #JaniceFreeDetectionSignature }
```
JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);
```

### Thread Safety {: #JaniceFreeDetectionThreadSafety }

This function is reentrant.

### Parameters {: #JaniceFreeDetectionParameters }

Name      | Type                                                | Description
--------- | --------------------------------------------------- | -----------
detection | [JaniceDetection\*](objects.md#JaniceDetectionType) | A detection object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

### Example {: #JaniceFreeDetectionExample }

```
JaniceDetection detection; // Where detection is a valid detection object
                           // created previously
if (janice_free_detection(&detection) != JANICE_SUCCESS)
    // ERROR!
```

## janice_create_template {: #JaniceCreateTemplate }

Create a [JaniceTemplate](objects.md#JaniceTemplateType) object from an array of detections.

### Signature {: #JaniceCreateTemplateSignature }
```
JANICE_EXPORT JaniceError janice_create_template(JaniceConstDetection* detections,
                                                 uint32_t num_detections,
                                                 JaniceEnrollmentType role,
                                                 JaniceTemplate* tmpl);
```

### Thread Safety {: #JaniceCreateTemplateThreadSafety }

This function is reentrant.

### Parameters {: #JaniceCreateTemplateParameters }

Name           | Type                                                     | Description
-------------- | -------------------------------------------------------- | -----------
detections     | [JaniceConstDetection\*](objects.md#JaniceDetectionType) | An array of detection objects
num_detections | uint32_t                                                 | The number of input detections
role           | [JaniceEnrollmentType](enums.md#JaniceEnrollmentType)    | The use case for the template
tmpl           | [JaniceTemplate\*](objects.md#JaniceTemplateType)        | An uninitialized template object.

### Example {: #JaniceCreateTemplateExample }

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

## janice_serialize_template {: #JaniceSerializeTemplate }

Serialize a [JaniceTemplate](objects.md#JaniceTemplateType) object to a flat buffer.

### Signature {: #JaniceSerializeTemplateSignature }
```
JANICE_EXPORT JaniceError janice_serialize_template(JaniceConstTemplate tmpl,
                                                    unsigned char** data,
                                                    size_t* len);
```

### Thread Safety {: #JaniceSerializeTemplateThreadSafety }

This function is reentrant.

### Parameters {: #JaniceSerializeTemplateParameters }

Name | Type                                                 | Description
---- | ---------------------------------------------------- | -----------
tmpl | [JaniceConstTemplate](objects.md#JaniceTemplateType) | A template object to serialize
data | unsigned char\*\*                                    | An uninitialized buffer to hold the flattened data. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.
len  | size_t\*                                             | The length of the flat buffer after it is filled.

### Example {: #JaniceSerializeTemplateExample }

```

JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_template(tmpl, &buffer, &buffer_len);
```

## janice_deserialize_template {: #JaniceDeserializeTemplate }

Deserialize a [JaniceTemplate](objects.md#JaniceTemplateType) object from a flat buffer.

### Signature {: #JaniceDeserializeTemplateSignature }
```
JANICE_EXPORT JaniceError janice_deserialize_template(const unsigned char** data,
                                                      size_t len,
                                                      JaniceTemplate* tmpl);
```

### Thread Safety {: #JaniceDeserializeTemplateThreadSafety }

This function is reentrant.

### Parameters

Name | Type                                              | Description
---- | ------------------------------------------------- | -----------
data | const unsigned char\*                             | A buffer containing data from a flattened template object.
len  | size_t                                            | The length of the flat buffer
tmpl | [JaniceTemplate\*](objects.md#JaniceTemplateType) | An uninitialized template object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.

### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.template", "r");
fread(buffer, 1, buffer_len, file);

JaniceTemplate tmpl = NULL; // best practice to initialize to NULL
janice_deserialize_template(buffer, buffer_len, tmpl);

fclose(file);
```

## janice_read_template {: #JaniceReadTemplate }

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

### Signature {: #JaniceReadTemplateSignature }
```
JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                               JaniceTemplate* tmpl);
```

### Thread Safety {: #JaniceReadTemplateThreadSafety }

This function is reentrant.

### Parameters {: #JaniceReadTemplateParameters }

Name     | Type                                              | Description
-------- | ------------------------------------------------- | -----------
filename | const char\*                                      | The path to a file on disk
tmpl     | [JaniceTemplate\*](objects.md#JaniceTemplateType) | An uninitialized template object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.

### Example {: #JaniceReadTemplateExample }

```
JaniceTemplate tmpl = NULL;
if (janice_read_template("example.template", &tmpl) != JANICE_SUCCESS)
    // ERROR!
```

## janice_write_template {: #JaniceWriteTemplate }

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

### Signature {: #JaniceWriteTemplateSignature }
```
JANICE_EXPORT JaniceError janice_write_template(JaniceConstTemplate tmpl,
                                                const char* filename);
```

### ThreadSafety {: #JaniceWriteTemplateThreadSafety }

This function is reentrant.

### Parameters {: #JaniceWriteTemplateParameters }

Name     | Type                                                 | Description
-------- | ---------------------------------------------------- | -----------
tmpl     | [JaniceConstTemplate](objects.md#JaniceTemplateType) | The template object to write to disk
filename | const char\*                                         | The path to a file on disk

### Example {: #JaniceWriteTemplateExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously
if (janice_write_template(tmpl, "example.template") != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_template {: #JaniceFreeTemplate }

Free any memory associated with a [JaniceTemplateType](objects.md#JaniceTemplateType) object.

### Signature {: #JaniceFreeTemplateSignature }
```
JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);
```

### Thread Safety {: #JaniceFreeTemplateThreadSafety }

This function is reentrant.

### Parameters {: #JaniceFreeTemplateParameters }

Name | Type                                              | Description
---- | ------------------------------------------------- | -----------
tmpl | [JaniceTemplate\*](objects.md#JaniceTemplateType) | A template object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

### Example {: #JaniceFreeTemplateExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created previously
if (janice_free_template(&tmpl) != JANICE_SUCCESS)
    // ERROR!
```

## janice_verify {: #JaniceVerify }

Compare two templates with the difference expressed as a similarity score.

### Signature {: #JaniceVerifySignature }
```
JANICE_EXPORT JaniceError janice_verify(JaniceConstTemplate reference,
                                        JaniceConstTemplate verification,
                                        double* similarity);
```

### Thread Safety {: #JaniceVerifyThreadSafety }

This function is reentrant.

### Similarity Score {: #JaniceVerifySimilarityScore }

This API expects that the comparison of two templates results in a single value
that quantifies the similarity between them. A similarity score is constrained
by the following requirements:

    1. Higher scores indicate greater similarity
    2. Scores can be asymmetric. This mean verify(a, b) does not necessarily
       equal verify(b, a)

### Parameters {: #JaniceVerifyParameters }

Name         | Type                                                 | Description
------------ | ---------------------------------------------------- | -----------
reference    | [JaniceConstTemplate](objects.md#JaniceTemplateType) | A reference template. This template was created with the [Janice11Reference](#JaniceEnrollmentType) role.
verification | [JaniceConstTemplate](objects.md#JaniceTemplateType) | A verification template. This template was created with the [Janice11Verification](enums.md#JaniceEnrollmentType) role.
similarity   | double\*                                             | A similarity score. See [this section](#JaniceVerifySimilarityScore) for more information.

### Example {: #JaniceVerifyExample }

```
JaniceTemplate reference; // Where reference is a valid template object created
                          // previously
JaniceTemplate verification; // Where verification is a valid template object
                             // created previously
double similarity;
if (janice_verify(reference, verification, &similarity) != JANICE_SUCCESS)
    // ERROR!
```

## janice_create_gallery {: #JaniceCreateGallery }

Create a [JaniceGalleryType](objects.md#JaniceGalleryType) object from a list of
templates and unique ids.

### Signature {: #JaniceCreateGallerySignature }
```
JANICE_EXPORT JaniceError janice_create_gallery(JaniceConstTemplate* tmpls,
                                                const uint32_t* ids,
                                                JaniceGallery* gallery);
```

### Thread Safety {: #JaniceCreateGalleryThreadSafety

This function is reentrant.

### Parameters {: #JaniceCreateGalleryParameters }

Name    | Type                                                   | Description
------- | ------------------------------------------------------ | -----------
tmpls   | [JaniceConstTemplate\*](objects.md#JaniceTemplateType) | An array of templates to add to the gallery
ids     | const uint32_t\*                                       | A set of unique identifiers to associate with the input templates. The *ith* id corresponds to the *ith* input template.
gallery | [JaniceGallery\*](objects.md#JaniceGalleryType)        | An uninitialized gallery object.

### Example {: #JaniceCreateGalleryExample }

```
JaniceTemplate* tmpls; // Where tmpls is a valid array of valid template
                       // objects created previously
uint32_t* ids; // Where ids is a valid array of unique unsigned integers that
               // is the same length as tmpls
JaniceGallery gallery = NULL; // best practice to initialize to NULL

if (janice_create_gallery(tmpls, ids, &gallery) != JANICE_SUCCESS)
    // ERROR!
```

## janice_gallery_insert {: #JaniceGalleryInsert }

Insert a template into a gallery object. The template data should be copied
into the gallery as the template may be deleted after this function.

### Signature {: #JaniceGalleryInsertSignature }
```
JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                JaniceConstTemplate tmpl,
                                                uint32_t id);
```

### Thread Safety {: #JaniceGalleryInsertThreadSafety }

This function is reentrant.

### Parameters {: #JaniceGalleryInsertParameters }

Name    | Type                                                 | Description
------- | ---------------------------------------------------- | -----------
gallery | [JaniceGallery](objects.md#JaniceGalleryType)        | A gallery object to insert the template into.
tmpl    | [JaniceConstTemplate](objects.md#JaniceTemplateType) | A template object to insert into the gallery.
id      | uint32_t                                             | A unique id to associate with the input template

### Example {: #JaniceGalleryInsertExample }

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

## janice_gallery_remove {: #JaniceGalleryRemove }

Remove a template from a gallery object using its unique id.

### Signature {: #JaniceGalleryRemoveSignature }
```
JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                uint32_t id);
```

### Thread Safety {: #JaniceGalleryRemoveThreadSafety }

This function is reentrant.

### Parameters

Name    | Type                                          | Description
------- | --------------------------------------------- | -----------
gallery | [JaniceGallery](objects.md#JaniceGalleryType) | A gallery object to remove the template from
id      | uint32_t                                      | A unique id associated with a template in the gallery that indicates which template should be remove.

### Example

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

## janice_gallery_prepare {: #JaniceGalleryPrepare }

Prepare a gallery for search. Implementors can use this function as an
opportunity to streamline gallery objects to accelerate the search process. The
calling convention for this function is **NOT** specified by the API, this
means that this function is not guaranteed to be called before
[janice_search](#JaniceSearch). It also means that templates can be added
to a gallery before and after this function is called. Implementations should
handle all of these calling conventions. However, users should be aware that
this function may be computationally expensive. They should strive to call it
only at critical junctions before search and as few times as possible overall.

### Signature {: #JaniceGalleryPrepareSignature }
```
JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);
```

### Thread Safety {: #JaniceGalleryPrepareThreadSafety }

This function is reentrant.

### Parameters {: #JaniceGalleryPrepareParameters }

Name    | Type                                          | Description
------- | --------------------------------------------- | -----------
gallery | [JaniceGallery](objects.md#JaniceGalleryType) | A gallery object to prepare

### Example {: #JaniceGalleryPrepareExample }

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

## janice_serialize_gallery {: #JaniceSerializeGallery }

Serialize a [JaniceGallery](objects.md#JaniceGalleryType) object to a flat buffer.

### Signature {: #JaniceSerializeGallerySignature }
```
JANICE_EXPORT JaniceError janice_serialize_gallery(JaniceConstGallery gallery,
                                                   unsigned char** data,
                                                   size_t* len);
```

### Thread Safety {: #JaniceSerializeGalleryThreadSafety }

This function is reentrant.

### Parameters {: #JaniceSerializeGalleryParameters }

Name    | Type                                               | Description
------- | -------------------------------------------------- | -----------
gallery | [JaniceConstGallery](objects.md#JaniceGalleryType) | A gallery object to serialize
data    | unsigned char\*\*                                  | An uninitialized buffer to hold the flattened data. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.
len     | size_t\*                                           | The length of the flat buffer after it is filled.

### Example {: #JaniceSerializeGalleryExample }

```

JaniceGallery gallery; // Where gallery is a valid gallery created
                       // previously.

unsigned char* buffer = NULL;
size_t buffer_len;
janice_serialize_gallery(gallery, &buffer, &buffer_len);
```

## janice_deserialize_gallery {: #JaniceDeserializeGallery }

Deserialize a [JaniceGallery](objects.md#JaniceGalleryType) object from a flat buffer.

### Signature {: #JaniceDeserializeGallerySignature }
```
JANICE_EXPORT JaniceError janice_deserialize_gallery(const unsigned char** data,
                                                     size_t len,
                                                     JaniceGallery* gallery);
```

### Thread Safety {: #JaniceDeserializeGalleryThreadSafety }

This function is reentrant.

### Parameters

Name    | Type                                            | Description
------- | ----------------------------------------------- | ---------
data    | const unsigned char\*                           | A buffer containing data from a flattened gallery object.
len     | size_t                                          | The length of the flat buffer
gallery | [JaniceGallery\*](objects.md#JaniceGalleryType) | An uninitialized gallery object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.

### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.gallery", "r");
fread(buffer, 1, buffer_len, file);

JaniceGallery gallery = NULL; // best practice to initialize to NULL
janice_deserialize_gallery(buffer, buffer_len, gallery);

fclose(file);
```

## janice_read_gallery {: #JaniceReadGallery }

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

### Signature {: #JaniceReadGallerySignature }
```
JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                              JaniceGallery* gallery);
```

### Thread Safety {: #JaniceReadGalleryThreadSafety }

This function is reentrant.

### Parameters {: #JaniceReadGalleryParameters }

Name     | Type                                            | Description
-------- | ----------------------------------------------- | -----------
filename | const char\*                                    | The path to a file on disk
gallery  | [JaniceGallery\*](objects.md#JaniceGalleryType) | An uninitialized gallery object. See [the section on memory allocation](software_concepts.md#MemoryAllocation) for requirements for initialization.

### Example {: #JaniceReadGalleryExample }

```
JaniceGallery gallery = NULL;
if (janice_read_gallery("example.gallery", &gallery) != JANICE_SUCCESS)
    // ERROR!
```

## janice_write_gallery {: #JaniceWriteGallery }

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

### Signature {: #JaniceWriteGallerySignature }
```
JANICE_EXPORT JaniceError janice_write_gallery(JaniceConstGallery gallery,
                                               const char* filename);
```

### ThreadSafety {: #JaniceWriteGalleryThreadSafety }

This function is reentrant.

### Parameters {: #JaniceWriteGalleryParameters }

Name     | Type                                               | Description
-------- | -------------------------------------------------- | -----------
gallery  | [JaniceConstGallery](objects.md#JaniceGalleryType) | The gallery object to write to disk
filename | const char\*                                       | The path to a file on disk

### Example {: #JaniceWriteGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery created previously
if (janice_write_gallery(gallery, "example.gallery") != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_gallery {: #JaniceFreeGallery }

Free any memory associated with a [JaniceGalleryType](objects.md#JaniceGalleryType) object.

### Signature {: #JaniceFreeGallerySignature }
```
JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);
```

### Thread Safety {: #JaniceFreeGalleryThreadSafety }

This function is reentrant.

### Parameters {: #JaniceFreeGalleryParameters }

Name    | Type                                            | Description
------- | ----------------------------------------------- | -----------
gallery | [JaniceGallery\*](objects.md#JaniceGalleryType) | A gallery object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

### Example {: #JaniceFreeGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery object created previously
if (janice_free_gallery(&gallery) != JANICE_SUCCESS)
    // ERROR!
```

## janice_search {: #JaniceSearch }

Compute 1-N search results between a query template object and a target gallery
object. The function allocates two arrays of equal size, one containing
[similarity scores](#JaniceVerifySimilarityScore) and the other containing the
unique id of the template the score was computed with (along with the query).
Often it is desirable (and perhaps computationally efficient) to only see the
top K scores out of N possible templates. The option to set a K is provided to
the user as part of the function parameters.

### Signature {: #JaniceSearchSignature }
```
JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                        JaniceConstGallery gallery,
                                        uint32_t num_requested,
                                        double** similarities,
                                        uint32_t** ids,
                                        uint32_t* num_returned);
```

### Thread Safety {: #JaniceSearchThreadSafety }

This function is reentrant.

### Parameters {: #JaniceSearchParameters }

Name          | Type                                                 | Description
------------- | ---------------------------------------------------- | -----------
probe         | [JaniceConstTemplate](objects.md#JaniceTemplateType) | A template object to use as a query
gallery       | [JaniceConstGallery](objects.md#JaniceGalleryType)   | A gallery object to search against
num_requested | uint32_t                                             | The number of requested returns. If the user would like as many returns as there are templates in the gallery they can pass this parameter with the value 0.
similarities  | double\*\*                                           | An array of [similarity scores](#JaniceVerifySimilarityScore). The scores are expected to be sorted in descending order (i.e. the highest scores is stored at index 0).
ids           | uint32_t\*\*                                         | An array of unique ids identifying the target templates associated with the similarity scores. This array must be the same size as the similarities array. The *ith* id corresponds with the *ith* similarity.
num_returned  | uint32_t\*                                           | The number of elements in the similarities and ids arrays. This number can be different from **num_requested**.

### Example {: #JaniceSearchExample }

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

## janice_cluster_media {: #JaniceClusterMedia }

Cluster a collection of media objects into groups. Each media object may
contain 0 or more objects of interest. The resulting array of cluster items
must contain a detection object that indicates the object of interest being
referred to by the item.

### Signature {: #JaniceClusterMediaSignature }
```
JANICE_EXPORT JaniceError janice_cluster_media(JaniceConstMedia* input,
                                               const uint32_t* input_ids,
                                               const uint32_t num_inputs,
                                               const uint32_t hint,
                                               JaniceClusterItem** clusters);
```

### Thread Safety {: #JaniceClusterMediaThreadSafety }

This function is reentrant.

### Hint {: #JaniceClusterMediaHint }

Clustering is generally considered to be an ill-defined problem, and most
algorithms require some help determining the appropriate number of clusters.
The hint parameter helps influence the number of clusters, though the
implementation is free to ignore it. The goal of the hint is to provide user
input for two use cases:

1. If the hint is between 0 - 1 it should be regarded as a purity requirement for the algorithm. A 1 indicates the user wants perfectly pure clusters, even if that means more clusters are returned. A 0 indicates that the user wants very few clusters returned and accepts there may be some errors.
2. If the hint is > 1 it represents an estimated upper bound on the number of object types in the set.

### Parameters {: #JaniceClusterMediaParameters }

Name         | Type                                                  | Description
------------ | ----------------------------------------------------- | -----------
input        | [JaniceConstMedia\*](objects.md#JaniceMediaType)      | An array of media objects.
input_ids    | const uint32_t\*                                      | An array of unique ids for the input objects. This array must be the same size as input.
num_inputs   | const uint32_t                                        | The size of the input and input_ids arrays
hint         | const uint32_t                                        | See [hint](#JaniceClusterMediaHint).
clusters     | [JaniceClusterItem\*\*](objects.md#JaniceClusterItem) | An uninitialized pointer to hold an array of cluster items.
num_clusters | uint32_t\*                                            | The size of the clusters array.

## janice_cluster_templates {: #JaniceClusterTemplates }

Cluster a collection of previously enrolled templates into groups. The
templates must be enrolled with the [JaniceCluster](enums.md#JaniceEnrollmentType) role.

### Signature {: #JaniceClusterTemplatesSignature }
```
JANICE_EXPORT JaniceError janice_cluster_templates(const JaniceTemplate* input,
                                                   const uint32_t* input_ids,
                                                   const uint32_t num_inputs,
                                                   const uint32_t hint,
                                                   JaniceClusterItem** clusters);
```

### Thread Safety {: #JaniceClusterTemplatesThreadSafety }

This function is reentrant.

### Parameters

Name         | Type                                                   | Description
------------ | ------------------------------------------------------ | -----------
input        | [JaniceConstTemplate\*](objects.md#JaniceTemplateType) | An array of template objects
input_ids    | const uint32_t\*                                       | An array of unique ids for the input objects. This array must be the same size as input.
num_inputs   | const uint32_t                                         | The size of the input and input_ids arrays
hint         | const uint32_t                                         | See [hint](#JaniceClusterMediaHint).
clusters     | [JaniceClusterItem\*\*](objects.md#JaniceClusterItem)  | An uninitialized pointer to hold an array of cluster items.
num_clusters | uint32_t\*                                             | The size of the clusters array.

## janice_finalize {: #JaniceFinalize }

Destroy any resources created by [janice_initialize](#JaniceInitialize) and
finalize the application. This should be called once after all other API calls.

### Signature {: #JaniceFinalizeSignature }
```
JANICE_EXPORT JaniceError janice_finalize();
```

### Thread Safety

This function is thread unsafe.
