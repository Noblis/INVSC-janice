## janice_detection_it_next {: #JaniceDetectionItNext }

Get the next element in a detection track.

#### Signature {: #JaniceDetectionItNextSignature }

```
JANICE_EXPORT JaniceError janice_detection_it_next(JaniceDetectionIterator it,
                                                   JaniceRect* rect,
                                                   uint32_t* frame,
                                                   float* confidence);
```

#### Thread Safety {: #JaniceDetectionItNextThreadSafety }

This function is reentrant.

#### Confidence {: #JaniceDetectionItNextConfidence }

The confidence value indicates a likelihood that the rectangle actually bounds
an object of interest. It is **NOT** required to be a probability and often
only has meaning relative to other confidence values from the same algorithm.
The only restriction is that a larger confidence value indicates a greater
likelihood that the rectangle bounds an object.

#### Parameters {: #JaniceDetectionItNextParameters }

Name       | Type                                                           | Description
---------- | -------------------------------------------------------------- | -----------
it         | [JaniceDetectionIterator](typedefs.md#JaniceDetectionIterator) | A detection iterator object
rect       | [JaniceRect\*](structs.md#JaniceRect)                          | The location of a object of interest
frame      | [uint32_t\*]                                                   | The frame index for an object of interest.
confidence | [float\*]                                                      | The [confidence](#JaniceDetectionItNextConfidence) of the location.

## janice_detection_it_reset {: #JaniceDetectionItReset }

Reset an iterator back to its initial state.

#### Signature {: #JaniceDetectionItResetSignature }

```
JANICE_EXPORT JaniceError janice_detection_it_reset(JaniceDetectionIterator it);
```

#### Thread Safety {: #JaniceDetectionItResetThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceDetectionItResetParameters }

Name | Type                                                           | Description
---- | -------------------------------------------------------------- | -----------
it   | [JaniceDetectionIterator](typedefs.md#JaniceDetectionIterator) | The iterator object to reset.

## janice_free_detection_it {: #JaniceFreeDetectionIt }

Free any memory associated with a detection iterator object.

#### Signature {: #JaniceFreeDetectionItSignature }

```
JANICE_EXPORT JaniceError janice_free_detection_it(JaniceDetectionIterator* it);
```

#### Thread Safety {: #JaniceFreeDetectionItThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeDetectionItParameters }

Name | Type                                                             | Description
---- | ---------------------------------------------------------------- | -----------
it   | [JaniceDetectionIterator\*](typedefs.md#JaniceDetectionIterator) | The iterator object to free.

## janice_create_detection {: #JaniceCreateDetection }

Create a detection from a known rectangle. This is useful if a human has
identified an object of interest and would like to run subsequent API
functions on it. In the case where the input media is a video the given
rectangle is considered an initial sighting of the object of interest. The
implementation may detect additional sightings of the object in successive
frames.

#### Signature {: #JaniceCreateDetectionSignature }

```
JANICE_EXPORT JaniceError janice_create_detection(JaniceConstMedia media,
                                                  const JaniceRect rect,
                                                  uint32_t frame,
                                                  JaniceDetection* detection);
```

#### Thread Safety {: #JaniceCreateDetectionThreadSafety }

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

Name      | Type                                                   | Description
--------- | ------------------------------------------------------ | -----------
media     | [JaniceConstMedia](../io/typedefs.md#JaniceConstMedia) | A media object to create the detection from
rect      | const [JaniceRect](structs.md#JaniceRect)              | A rectangle that bounds the object of interest
frame     | uint32_t                                               | An index to the frame in the media the rectangle refers to. If the media is an image this should be 0.
detection | [JaniceDetection\*](typedefs.md#JaniceDetection)       | An uninitialized pointer to a detection object.

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

## janice_detect {: #JaniceDetect }

Automatically detect objects in a media object. See
[the detection overview](overview.md) for an overview of detection in the
context of this API.

#### Signature {: #JaniceDetectSignature }

```
JANICE_EXPORT JaniceError janice_detect(JaniceConstMedia media,
                                        uint32_t min_object_size,
                                        JaniceDetections* detections,
                                        uint32_t* num_detections);
```

#### Thread Safety {: #JaniceDetectThreadSafety }

This function is reentrant.

#### Minimum Object Size {: #JaniceDetectMinimumObjectSize }

This function specifies a minimum object size as one of its parameters. This
value indicates the minimum size of objects that the user would like to see
detected. Often, increasing the minimum size can improve runtime of algorithms.
The size is in pixels and corresponds to the length of the smaller side of the
rectangle. This means a detection will be returned if and only if its smaller
side is larger than the value specified. If the user does not wish to specify a
minimum width 0 can be provided.

#### Tracking {: #JaniceDetectTracking }

When the input media is a video many implementations will implement a form of
object tracking to correlate multiple sightings of the same object into a
single structure. There are a number of approaches and algorithms to implement
object tracking. This API makes NO attempt to define or otherwise constrain how
implementations handle tracking. Users should be warned that an implementation
might output multiple tracks for a single object and that a single track might
contain multiple objects in it by mistake. In some cases, which should be
clearly documented in implementation documentation, it might be beneficial to
perform a post-processing clustering step on the results tracks, which could
help correlate multiple tracks of the same object.

#### Face Recognition {: #JaniceDetectFaceRecognition }

This function detects faces in the media.

#### Camera Identification {: #JaniceDetectCameraIdentification }

This function provides a list of size 1 where the only entry is a detection
with a rect that encapsulates the entire image.

#### Parameters {: #JaniceDetectParameters }

Name            | Type                                                   | Description
--------------- | ------------------------------------------------------ | -----------
media           | [JaniceConstMedia](../io/typedefs.md#JaniceConstMedia) | A media object to create the detection from.
min_object_size | uint32_t                                               | A minimum object size. See [Minimum Object Size](#JaniceDetectMinimumObjectSize)
detections      | [JaniceDetections\*](typedefs.md#JaniceDetections)     | An uninitialized array to hold all of the detections detected in the media object.
num_detections  | uint32_t\*                                             | The number of detections detected

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

## janice_create_detection_it {: #JaniceCreateDetectionIt }

Create an iterator to iterate over detection elements.

#### Signature {: #JaniceCreateDetectionItSignature }

```
JANICE_EXPORT JaniceError janice_create_detection_it(JaniceConstDetection detection,
                                                     JaniceDetectionIterator* it);
```

#### Thread Safety {: #JaniceCreateDetectionItThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceCreateDetectionItParameters }

Name      | Type                                                             | Description
--------- | ---------------------------------------------------------------- | -----------
detection | [JaniceConstDetection](typedefs.md#JaniceConstDetection)         | The detection to create the iterator from.
it        | [JaniceDetectionIterator\*](typedefs.md#JaniceDetectionIterator) | An uninitialized detection iterator object. It is initialized as part of the function call.

## janice_serialize_detection {: #JaniceSerializeDetection }

Serialize a [JaniceDetection](typedefs.md#JaniceDetection) object to a flat buffer.

#### Signature {: #JaniceSerializeDetectionSignature }

```
JANICE_EXPORT JaniceError janice_serialize_detection(JaniceConstDetection detection,
                                                     JaniceBuffer* data,
                                                     size_t* len);
```

#### Thread Safety {: #JaniceSerializeDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSerializeDetectionParameters }

Name      | Type                                                     | Description
--------- | -------------------------------------------------------- | -----------
detection | [JaniceConstDetection](typedefs.md#JaniceConstDetection) | A detection object to serialize
data      | [JaniceBuffer\*](#../io/typedefs.md#JaniceBuffer)        | An uninitialized buffer to hold the flattened data.
len       | size_t\*                                                 | The length of the flat buffer after it is filled.

#### Example {: #JaniceSerializeDetectionExample }

```

JaniceDetection detection; // Where detection is a valid detection created
                           // previously.

JaniceBuffer buffer = NULL;
size_t buffer_len;
janice_serialize_detection(detection, &buffer, &buffer_len);
```

## janice_deserialize_detection {: #JaniceDeserializeDetection }

Deserialize a [JaniceDetection](typedefs.md#JaniceDetection) object from a flat buffer.

#### Signature {: #JaniceDeserializeDetectionSignature }

```
JANICE_EXPORT JaniceError janice_deserialize_detection(const JaniceBuffer data,
                                                       size_t len,
                                                       JaniceDetection* detection);
```

#### Thread Safety {: #JaniceDeserializeDetectionThreadSafety }

This function is reentrant.

#### Parameters

Name      | Type                                                 | Description
--------- | ---------------------------------------------------- | -----------
data      | const [JaniceBuffer](../io/typedefs.md#JaniceBuffer) | A buffer containing data from a flattened detection object.
len       | size_t                                               | The length of the flat buffer
detection | [JaniceDetection\*](typedefs.md#JaniceDetection)     | An uninitialized detection object.

#### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
JaniceBuffer buffer[buffer_len];

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
JaniceBuffer buffer[buffer_len];

FILE* file = fopen("serialized.detection", "r");
fread(buffer, 1, buffer_len, file);

JaniceDetection detection = nullptr;
janice_deserialize_detection(buffer, buffer_len, detection);

fclose(file);
```

It is provided for memory efficiency and ease of use when reading from disk.

#### Signature {: #JaniceReadDetectionSignature }

```
JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                JaniceDetection* detection);
```

#### Thread Safety {: #JaniceReadDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceReadDetectionParameters }

Name      | Type                                             | Description
--------- | ------------------------------------------------ | -----------
filename  | const char\*                                     | The path to a file on disk
detection | [JaniceDetection\*](typedefs.md#JaniceDetection) | An uninitialized detection object.

#### Example {: #JaniceReadDetectionExample }

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

JaniceBuffer buffer = NULL;
size_t buffer_len;
janice_serialize_detection(detection, &buffer, &buffer_len);

FILE* file = fopen("serialized.detection", "w+");
fwrite(buffer, 1, buffer_len, file);

fclose(file);
```

It is provided for memory efficiency and ease of use when writing to disk.

#### Signature {: #JaniceWriteDetectionSignature }

```
JANICE_EXPORT JaniceError janice_write_detection(JaniceConstDetection detection,
                                                 const char* filename);
```

#### ThreadSafety {: #JaniceWriteDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceWriteDetectionParameters }

Name      | Type                                                     | Description
--------- | -------------------------------------------------------- | -----------
detection | [JaniceConstDetection](typedefs.md#JaniceConstDetection) | The detection object to write to disk
filename  | const char\*                                             | The path to a file on disk

#### Example {: #JaniceWriteDetectionExample }

```
JaniceDetection detection; // Where detection is a valid detection created
                           // previously
if (janice_write_detection(detection, "example.detection") != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_detection {: #JaniceFreeDetection }

Free any memory associated with a [JaniceDetectionType](structs.md#JaniceDetectionType) object.

#### Signature {: #JaniceFreeDetectionSignature }

```
JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);
```

#### Thread Safety {: #JaniceFreeDetectionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeDetectionParameters }

Name      | Type                                             | Description
--------- | ------------------------------------------------ | -----------
detection | [JaniceDetection\*](typedefs.md#JaniceDetection) | A detection object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeDetectionExample }

```
JaniceDetection detection; // Where detection is a valid detection object
                           // created previously
if (janice_free_detection(&detection) != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_detections {: #JaniceFreeDetections }

Free any memory associated with an array of
[JaniceDetection](typedefs.md#JaniceDetection) objects. This function should
free memory allocated for the underlying detection objects AND memory allocated
for the array.

#### Signature {: #JaniceFreeDetectionsSignature }

```
JANICE_EXPORT JaniceError janice_free_detections(JaniceDetections* detection,
                                                 uint32_t num_detections);
```

#### Thread Safety {: #JaniceFreeDetectionsThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeDetectionsParameters }

Name       | Type                                               | Description
---------- | -------------------------------------------------- | -----------
detections | [JaniceDetections\*](typedefs.md#JaniceDetections) | An array of detections to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.
