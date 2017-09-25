.. _detection:

Detection
=========

Overview
--------

In the context of this API, detection is used to refer to the identification of
objects of interest within a :ref:`io` object. Detections are represented using
the :ref:`JaniceDetectionType` object which implementors are free to define
however they would like. For images, a detection is defined as a rectangle that
bounds an object of interest and an associated confidence value. For video, a
single object can exist in multiple frames. A rectangle and confidence are only
relevant in a single frame. In this case, we define a detection as a list of
(rectangle, confidence) pairs that track a single object through a video. It is
not required that this list be dense however (i.e. frames can be skipped). To
support this, we extend our representation of a detection to a (rectangle,
confidence, frame) tuple where frame gives the index of the frame the rectangle
was found in.

Structs
-------

.. _JaniceRect:

JaniceRect
~~~~~~~~~~

A simple struct that represents a rectangle

Fields
^^^^^^

+----------+-------------+-------------------------------------------+
| Name     | Type        | Description                               |
+==========+=============+===========================================+
| x        | uint32\_t   | The x offset of the rectangle in pixels   |
+----------+-------------+-------------------------------------------+
| y        | uint32\_t   | The y offset of the rectangle in pixels   |
+----------+-------------+-------------------------------------------+
| width    | uint32\_t   | The width of the rectangle in pixels      |
+----------+-------------+-------------------------------------------+
| height   | uint32\_t   | The height of the rectangle in pixels     |
+----------+-------------+-------------------------------------------+

.. _JaniceTrack:

JaniceTrack
~~~~~~~~~~~

A structure to represent a track through a :ref:`JaniceMediaIterator` object.
Tracks may be sparse (i.e. frames do not need to be sequential). Tracks are
meant to follow a single object or area of interest, for example a face through
multiple frames of a video.

.. _detection\_confidence:

Confidence
^^^^^^^^^^

The confidence value indicates a likelihood that the rectangle actually
bounds an object of interest. It is **NOT** required to be a probability
and often only has meaning relative to other confidence values from the
same algorithm. The only restriction is that a larger confidence value
indicates a greater likelihood that the rectangle bounds an object.


Fields
^^^^^^

+-------------+---------------------+------------------------------------------+
| Name        | Type                | Description                              |
+=============+=====================+==========================================+
| rects       | :ref:`JaniceRect`\* | A list of rectangles surrounding areas   |
|             |                     | of interest in the media. This list      |
|             |                     | should be *length* elements.             |
+-------------+---------------------+------------------------------------------+
| confidences | float\*             | A confidence to associate with each      |
|             |                     | rectangle in *rects*. See                |
|             |                     | :ref:`detection\_confidence` for details |
|             |                     | about confidence values in this API. This|
|             |                     | list should be *length* elements.        |
+-------------+---------------------+------------------------------------------+
| frames      | uint32_t\*          | The frame indices associated with each   |
|             |                     | rectangle in *rects*. A track may be     |
|             |                     | sparse and the indicies in this list are |
|             |                     | required to be sequential. This list     |
|             |                     | should be *length* elements.             |
+-------------+---------------------+------------------------------------------+
| length      | size_t              | The number of rectangles, confidences,   |
|             |                     | and frames in this structure.            |
+-------------+---------------------+------------------------------------------+

.. _JaniceTracks:

JaniceTracks
~~~~~~~~~~~~

An array of :ref:`JaniceTrack` objects.

Signature
^^^^^^^^^

::

    typedef JaniceTrack* JaniceTracks;

.. _JaniceAttribute:

JaniceAttribute
~~~~~~~~~~~~~~~

A null-terminated string with an implementation defined format representing
an attribute or a detection, template or gallery object. Implementations are
free to define and implement attributes of their choice. For example, with face
recognition an attribute might be:

    * Gender
    * Age
    * Ethnicity
    * Glasses
    * etc.

Signature
^^^^^^^^^

::

    typedef char* JaniceAttribute;

.. _JaniceMediaId:

JaniceMediaId
~~~~~~~~~~~~~

A unique identifier for a :ref:`JaniceMediaIterator` object.

Signature
^^^^^^^^^

::

    typedef size_t JaniceMediaId;

.. _JaniceMediaIds:

JaniceMediaIds
~~~~~~~~~~~~~~

An array of :ref:`JaniceMediaId` objects.

Signature
^^^^^^^^^

::

    typedef JaniceMediaId* JaniceMediaIds;


.. _JaniceDetectionType:

JaniceDetectionType
~~~~~~~~~~~~~~~~~~~

An opaque pointer to a struct that represents a detection. See :ref:`detection`
for more information.

.. _JaniceDetection:

JaniceDetection
~~~~~~~~~~~~~~~

A pointer to a :ref:`JaniceDetectionType` object.

Signature
^^^^^^^^^

::

    typedef struct JaniceDetectionType* JaniceDetection;

.. _JaniceConstDetection:

JaniceConstDetection
~~~~~~~~~~~~~~~~~~~~

A pointer to a constant :ref:`JaniceDetectionType` object.

Signature
^^^^^^^^^

::

    typedef const struct JaniceDetectionType* JaniceConstDetection;

.. _JaniceDetections:

JaniceDetections
~~~~~~~~~~~~~~~~

An array of :ref:`JaniceDetection` objects.

Signature
^^^^^^^^^

::

    typedef struct JaniceDetection* JaniceDetections;

.. _JaniceConstDetections:

JaniceConstDetections
~~~~~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceConstDetection` objects.

Signature
^^^^^^^^^

::

    typedef struct JaniceConstDetection* JaniceConstDetections;

.. _JaniceDetectionPolicy:

JaniceDetectionPolicy
~~~~~~~~~~~~~~~~~~~~~

A policy that controls the types of objects that should be detected by a call
to :ref:`janice\_detect`. Supported policies are:

+---------------------+-------------------------------------------------------+
| Policy              | Description                                           |
+=====================+=======================================================+
| JaniceDetectAll     | Detect all objects present in the media.              |
+---------------------+-------------------------------------------------------+
| JaniceDetectLargest | Detect the largest object present in the media.       |
|                     | Running detection with this policy should produce at  |
|                     | most one detection.                                   |
+---------------------+-------------------------------------------------------+
| JaniceDetectBest    | Detect the best object present in the media. The      |
|                     | implementor is responsible for defining what "best"   |
|                     | entails in the context of their algorithm. Running    |
|                     | detection with this policy should produce at most one |
|                     | detection.                                            |
+---------------------+-------------------------------------------------------+

Functions
---------

.. _janice\_create\_detection\_from\_rect:

janice\_create\_detection\_from\_rect
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a detection from a known rectangle. This is useful if a human has
identified an object of interest and would like to run subsequent API
functions on it. In the case where the input media is a video the given
rectangle is considered an initial sighting of an object or region of interest.
The implementation may detect additional sightings of the object in successive
frames.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_detection_from_rect(JaniceMediaIterator media,
                                                                const JaniceRect rect,
                                                                uint32_t frame,
                                                                JaniceDetection* detection);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+----------------------------+-------------------------------------+
| Name      | Type                       | Description                         |
+===========+============================+=====================================+
| media     | :ref:`JaniceMediaIterator` | A media object to create the        |
|           |                            | detection from.                     |
+-----------+----------------------------+-------------------------------------+
| rect      | const :ref:`JaniceRect`    | A rectangle that bounds the object  |
|           |                            | of interest.                        |
+-----------+----------------------------+-------------------------------------+
| frame     | uint32\_t                  | An index to the frame in the media  |
|           |                            | where the object of interest appears|
|           |                            | If the media is an image this should|
|           |                            | be 0.                               |
+-----------+----------------------------+-------------------------------------+
| detection | :ref:`JaniceDetection`\*   | An uninitialized pointer to a       |
|           |                            | detection object. The object should |
|           |                            | allocated by the implementor during |
|           |                            | function execution. The user is     |
|           |                            | responsible for freeing the object  |
|           |                            | using                               |
|           |                            | :ref:`janice\_free\_detection`.     |
+-----------+----------------------------+-------------------------------------+

Example
^^^^^^^

::

    JaniceMedia media; // Where media is a valid media object created previously

    JaniceRect rect; // Create a bounding rectangle around an object of interest
    rect.x      = 10; // The rectangle should fall within the bounds of the media
    rect.y      = 10; // This code assumes media width > 110 and media height > 110
    rect.width  = 100;
    rect.height = 100;

    JaniceDetection detection = NULL; // best practice to initialize to NULL
    if (janice_create_detection(media, rect, 0 /* frame */, &detection) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_create\_detection\_from\_track:

janice\_create\_detection\_from\_track
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a detection from a known track. This is useful if a human has
identified an object of interest and would like to run subsequent API
functions on it.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_detection_from_track(JaniceMediaIterator media,
                                                                const JaniceTrack track,
                                                                JaniceDetection* detection);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+----------------------------+-------------------------------------+
| Name      | Type                       | Description                         |
+===========+============================+=====================================+
| media     | :ref:`JaniceMediaIterator` | A media object to create the        |
|           |                            | detection from.                     |
+-----------+----------------------------+-------------------------------------+
| track     | :ref:`JaniceTrack`         | A track bounding a region of        |
|           |                            | through 1 or more frames.           |
+-----------+----------------------------+-------------------------------------+
| detection | :ref:`JaniceDetection`\*   | An uninitialized pointer to a       |
|           |                            | detection object. The object should |
|           |                            | allocated by the implementor during |
|           |                            | function execution. The user is     |
|           |                            | responsible for freeing the object  |
|           |                            | using                               |
|           |                            | :ref:`janice\_free\_detection`.     |
+-----------+----------------------------+-------------------------------------+

.. _janice\_detect:

janice\_detect
~~~~~~~~~~~~~~

Automatically detect objects in a media object. See :ref:`detection` for an
overview of detection in the context of this API.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_detect(JaniceMediaIterator media,
                                            uint32_t min_object_size,
                                            JaniceDetectionPolicy policy,
                                            JaniceDetections* detections,
                                            uint32_t* num_detections);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

.. _detection_min_object_size:

Minimum Object Size
^^^^^^^^^^^^^^^^^^^

This function specifies a minimum object size as one of its parameters.
This value indicates the minimum size of objects that the user would
like to see detected. Often, increasing the minimum size can improve
runtime of algorithms. The size is in pixels and corresponds to the
length of the smaller side of the rectangle. This means a detection will
be returned if and only if its smaller side is larger than the value
specified. If the user does not wish to specify a minimum width 0 can be
provided.

.. detection\_tracking:

Tracking
^^^^^^^^

When the input media is a video, implementations may implement a
form of object tracking to correlate multiple sightings of the same
object into a single structure. There are a number of approaches and
algorithms to implement object tracking. This API makes NO attempt to
define or otherwise constrain how implementations handle tracking. Users
should be warned that an implementation might output multiple tracks for
a single object and that a single track might contain multiple objects
in it by mistake. In some cases, which should be clearly documented in
implementation documentation, it might be beneficial to perform a
post-processing clustering step on the results tracks, which could help
correlate multiple tracks of the same object.

Parameters
^^^^^^^^^^

+-------------------+------------------------------+------------------------------------+
| Name              | Type                         | Description                        |
+===================+==============================+====================================+
| media             | :ref:`JaniceMediaIterator`   | A media object to run detection on.|
+-------------------+------------------------------+------------------------------------+
| min\_object\_size | uint32\_t                    | A minimum object size. See         |
|                   |                              | :ref:`detection\_min\_object\_size`|
+-------------------+------------------------------+------------------------------------+
| policy            | :ref:`JaniceDetectionPolicy` | The detection policy to follow.    |
+-------------------+------------------------------+------------------------------------+
| detections        | :ref:`JaniceDetections` \*   | An uninitialized array to hold all |
|                   |                              | of the detections detected in the  |
|                   |                              | in the media object. This object   |
|                   |                              | should be allocated by the         |
|                   |                              | implementor during the call. The   |
|                   |                              | user is required to free the object|
|                   |                              | by calling                         |
|                   |                              | :ref:`janice\_free\_detections`.   |
+-------------------+------------------------------+------------------------------------+
| num\_detections   | uint32\_t\*                  | The number of detections returned  |
|                   |                              | in the *detections* array.         |
+-------------------+------------------------------+------------------------------------+

Example
^^^^^^^

::

    JaniceMedia media; // Where media is a valid media object created previously
    const uint32_t min_object_size = 24; // Only find objects where the smaller
                                         // side is > 24 pixels
    JaniceDetectionPolicy policy = JaniceDetectAll; // Detect all objects in the media
    JaniceDetection* detections = NULL; // best practice to initialize to NULL
    uint32_t num_detections; // Will be populated with the size of detections

    if (janice_detect(media, min_object_size, policy, &detections, &num_detections) != JANICE_SUCCESS)
        // ERROR!


.. _janice\_detect\_batch:

janice\_detect\_batch
~~~~~~~~~~~~~~~~~~~~~

Detect faces in a batch of media objects. Batch processing can often be more 
efficient than serial processing, particularly if a GPU or co-processor is being 
utilized.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_detect_batch(JaniceMediaIterators media, 
                                                  JaniceMediaIds media_ids, 
                                                  uint32_t num_media,
                                                  uint32_t min_object_size,
                                                  JaniceDetectionPolicy policy,
                                                  JaniceDetections* detections,
                                                  JaniceMediaIds* detection_ids,
                                                  uint32_t* num_detections);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------------------+------------------------------+-------------------------------------+
| Name              | Type                         | Description                         |
+===================+==============================+=====================================+
| media             | :ref:`JaniceMediaIterators`  | An array of media iterators to run  |
|                   |                              | detection on. The array contains    | 
|                   |                              | *num_media* elements.               |
+-------------------+------------------------------+-------------------------------------+
| media_ids         | :ref:`JaniceMediaIds`        | An array of unique identifiers for  |
|                   |                              | the media iterators. The array      |
|                   |                              | contains *num_media* elements.      | 
+-------------------+------------------------------+-------------------------------------+
| num_media         | uint32_t                     | The size of the *media* and         |
|                   |                              | *media_ids* arrays.                 |
+-------------------+------------------------------+-------------------------------------+
| min\_object\_size | uint32_t                     | A minimum object size. See          |
|                   |                              | :ref:`detection\_min\_object\_size` |
+-------------------+------------------------------+-------------------------------------+
| policy            | :ref:`JaniceDetectionPolicy` | The detection policy to follow.     |
+-------------------+------------------------------+-------------------------------------+
| detections        | :ref:`JaniceDetections`\*    | An uninitialized array to hold all  |
|                   |                              | of the detections detected in the   |
|                   |                              | the media objects. This object      |
|                   |                              | should be allocated by the          |
|                   |                              | implementor during the call. The    |
|                   |                              | user is required to free the object |
|                   |                              | by calling                          |
|                   |                              | :ref:`janice\_free\_detections`.    |
+-------------------+------------------------------+-------------------------------------+
| detection_ids     | :ref:`JaniceMediaIds`\*      | An uninitializedarray of media ids  |
|                   |                              | that associates a detection to it's |
|                   |                              | source media. This array should have|
|                   |                              | the same number of elements as      |
|                   |                              | *detections*. This object should be |
|                   |                              | allocated by the implementor during |
|                   |                              | the call. The user is require to    |
|                   |                              | free the object by calling          |
|                   |                              | :ref:`janice\_free\_media\_ids`.    |
+-------------------+------------------------------+-------------------------------------+
| num_detections    | uint32_t\*                   | The number of elements in           | 
|                   |                              | *detections* and *num_detections*.  |
+-------------------+------------------------------+-------------------------------------+

.. _janice\_detection\_get\_track:

janice\_detection\_get\_track
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Get a track object from a detection. The returned track should contain all
rectangles, confidences, and frame indicies stored in the detection.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_detection_get_track(JaniceConstDetection detection,
                                                         JaniceTrack* track);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+-----------------------------+------------------------------------+
| Name      | Type                        | Description                        |
+===========+=============================+====================================+
| detection | :ref:`JaniceConstDetection` | The detection to get the track     |
|           |                             | from.                              |
+-----------+-----------------------------+------------------------------------+
| track     | :ref:`JaniceTrack`\*        | An uninitialized track object. This|
|           |                             | object should be allocated by the  |
|           |                             | implementor during the call. The   |
|           |                             | user is responsible for free this  |
|           |                             | object by calling                  |
|           |                             | :ref:`janice\_free\_track`.        |
+-----------+-----------------------------+------------------------------------+

.. _janice\_detection\_get\_attribute:

janice\_detection\_get\_attribute
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Get an attribute from a detection. Attributes are additional metadata that an
implementation might have when creating a detection. Examples from face 
detection include gender, ethnicity, and / or landmark locations. Implementors
are responsible for providing documentation on any attributes they support,
valid key values and possible return values.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_detection_get_attribute(JaniceConstDetection detection,
                                                             const char* key,
                                                             JaniceAttribute& value);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+-----------------------------+------------------------------------+
| Name      | Type                        | Description                        |
+===========+=============================+====================================+
| detection | :ref:`JaniceConstDetection` | The detection object to extract the|
|           |                             | attribute from.                    |
+-----------+-----------------------------+------------------------------------+
| key       | const char\*                | A key to look up a specific        |
|           |                             | attribute. Valid keys must be      |
|           |                             | defined and documented by the      |
|           |                             | implementor.                       | 
+-----------+-----------------------------+------------------------------------+
| value     | :ref:`JaniceAttribute`\*    | An uninitialized char\* to hold    |
|           |                             | the value of the attribute. This   |
|           |                             | object should be allocated by the  |
|           |                             | implementor during the function    |
|           |                             | call. The user is responsible for  |
|           |                             | the object by calling              |
|           |                             | :ref:`janice\_free\_attribute.     |
+-----------+-----------------------------+------------------------------------+

.. _janice\_serialize\_detection:

janice\_serialize\_detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Serialize a :ref:`JaniceDetection` object to a flat buffer.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_serialize_detection(JaniceConstDetection detection,
                                                         JaniceBuffer* data,
                                                         size_t* len);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------------+-----------------------------+----------------------------------+
| Name        | Type                        | Description                      |
+=============+=============================+==================================+
| detection   | :ref:`JaniceConstDetection` | A detection object to serialize  |
+-------------+-----------------------------+----------------------------------+
| data        | :ref:`JaniceBuffer` \*      | An uninitialized buffer to hold  |
|             |                             | the flattened data. The          |
|             |                             | implementor should allocate this |
|             |                             | object during the function call. |
|             |                             | The user is required to free the |
|             |                             | object with                      |
|             |                             | :ref:`janice\_free\_buffer`.     |
+-------------+-----------------------------+----------------------------------+
| len         | size\_t\*                   | The length of the flat buffer    |
|             |                             | after it is filled.              |
+-------------+-----------------------------+----------------------------------+

Example
^^^^^^^

::


    JaniceDetection detection; // Where detection is a valid detection created
                               // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_detection(detection, &buffer, &buffer_len);

.. _janice\_deserialize\_detection:

janice\_deserialize\_detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Deserialize a :ref:`JaniceDetection` object from a flat buffer.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_deserialize_detection(const JaniceBuffer data,
                                                           size_t len,
                                                           JaniceDetection* detection);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+---------------------------+--------------------------------------+
| Name      | Type                      | Description                          |
+===========+===========================+======================================+
| data      | const :ref:`JaniceBuffer` | A buffer containing data from a      |
|           |                           | flattened detection object.          |
+-----------+---------------------------+--------------------------------------+
| len       | size\_t                   | The length of the flat buffer.       |
+-----------+---------------------------+--------------------------------------+
| detection | :ref:`JaniceDetection` \* | An uninitialized detection object.   |
|           |                           | This object should be allocated by   |
|           |                           | the implementor during the function  |
|           |                           | call. Users are required to free the |
|           |                           | object with                          |
|           |                           | :ref:`janice\_free\_detection`.      |
+-----------+---------------------------+--------------------------------------+

Example
^^^^^^^

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    JaniceBuffer buffer[buffer_len];

    FILE* file = fopen("serialized.detection", "r");
    fread(buffer, 1, buffer_len, file);

    JaniceDetection detection = nullptr;
    janice_deserialize_detection(buffer, buffer_len, detection);

    fclose(file);

.. _janice\_read\_detection:

janice\_read\_detection
~~~~~~~~~~~~~~~~~~~~~~~

Read a detection from a file on disk. This method is functionally
equivalent to the following-

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    JaniceBuffer buffer[buffer_len];

    FILE* file = fopen("serialized.detection", "r");
    fread(buffer, 1, buffer_len, file);

    JaniceDetection detection = nullptr;
    janice_deserialize_detection(buffer, buffer_len, detection);

    fclose(file);

It is provided for memory efficiency and ease of use when reading from
disk.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_read_detection(const char* filename,
                                                    JaniceDetection* detection);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------------+---------------------------+--------------------------------------+
| Name        | Type                      | Description                          |
+=============+===========================+======================================+
| filename    | const char\*              | The path to a file on disk           |
+-------------+---------------------------+--------------------------------------+
| detection   | :ref:`JaniceDetection` \* | An uninitialized detection object.   |
+-------------+---------------------------+--------------------------------------+

Example
^^^^^^^

::

    JaniceDetection detection = NULL;
    if (janice_read_detection("example.detection", &detection) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_write\_detection:

janice\_write\_detection
~~~~~~~~~~~~~~~~~~~~~~~~

Write a detection to a file on disk. This method is functionally
equivalent to the following-

::

    JaniceDetection detection; // Where detection is a valid detection created
                               // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_detection(detection, &buffer, &buffer_len);

    FILE* file = fopen("serialized.detection", "w+");
    fwrite(buffer, 1, buffer_len, file);

    fclose(file);

It is provided for memory efficiency and ease of use when writing to
disk.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_write_detection(JaniceConstDetection detection,
                                                     const char* filename);

ThreadSafety
^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------------+-----------------------------+----------------------------------+
| Name        | Type                        | Description                      |
+=============+=============================+==================================+
| detection   | :ref:`JaniceConstDetection` | The detection object to write to |
|             |                             | disk.                            |
+-------------+-----------------------------+----------------------------------+
| filename    | const char\*                | The path to a file on disk       |
+-------------+-----------------------------+----------------------------------+

Example
^^^^^^^

::

    JaniceDetection detection; // Where detection is a valid detection created
                               // previously
    if (janice_write_detection(detection, "example.detection") != JANICE_SUCCESS)
        // ERROR!

.. _janice\_free\_detection:

janice\_free\_detection
~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceDetection` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_detection(JaniceDetection* detection);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+------------+---------------------------+-------------------------------------+
| Name       | Type                      | Description                         |
+============+===========================+=====================================+
| detection  | :ref:`JaniceDetection` \* | A detection object to free. Best    |
|            |                           | practice dictates the pointer       |
|            |                           | should be set to *NULL* after it is |
|            |                           | freed.                              |
+------------+---------------------------+-------------------------------------+

Example
^^^^^^^

::

    JaniceDetection detection; // Where detection is a valid detection object
                               // created previously
    if (janice_free_detection(&detection) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_free\_detections:

janice\_free\_detections
~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a:ref:`JaniceDetections` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_detections(JaniceDetections* detection,
                                                     uint32_t num_detections);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+----------------+----------------------------+--------------------------------+
| Name           | Type                       | Description                    |
+================+============================+================================+
| detections     | :ref:`JaniceDetections` \* | An array of detections to free.|
|                |                            | Best practice dictates the     |
|                |                            | pointer should be set to       |
|                |                            | *NULL* after it is freed.      |
+----------------+----------------------------+--------------------------------+
| num_detections | uint32_t                   | The number of elements in      |
|                |                            | *detections*.                  |
+----------------+----------------------------+--------------------------------+

.. _janice\_free\_track:

janice\_free\_track
~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceTrack` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_track(JaniceTrack* track);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------+--------------------+-------------------------------------------------+
| Name  | Type               | Description                                     |
+=======+====================+=================================================+
| track | :ref:`JaniceTrack` | The track object to free. Best practice dictates|
|       |                    | the pointer should be set to *NULL* after it is |
|       |                    | freed.                                          |
+-------+--------------------+-------------------------------------------------+

.. _janice\_free\_tracks:

janice\_free\_tracks
~~~~~~~~~~~~~~~~~~~~

Free any memory associated with an array of :ref:`JaniceTrack` objects.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_tracks(JaniceTracks* tracks,
                                                 uint32_t num_tracks);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+------------+---------------------+-------------------------------------------+
| Name       | Type                | Description                               |
+============+=====================+===========================================+
| tracks     | :ref:`JaniceTracks` | An array of track objects to free. Best   |
|            |                     | practice dictates the pointer should be   |
|            |                     | set to *NULL* after the object is freed.  |
+------------+---------------------+-------------------------------------------+
| num_tracks | uint32_t            | The number of elements in *tracks*.       |
+------------+---------------------+-------------------------------------------+

.. _janice\_free\_attribute:

janice\_free\_attribute
~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with an attribute value.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_attribute(JaniceAttribute* value);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+--------------------------+---------------------------------------+
| Name      | Type                     | Description                           |
+===========+==========================+=======================================+
| attribute | :ref:`JaniceAttribute`\* | The attribute to free. Best practice  |
|           |                          | dictates the pointer should be set to |
|           |                          | *NULL* after it is freed.             |
+-----------+--------------------------+---------------------------------------+

.. _janice\_free\_media\_ids:

janice\_free\_media\_ids
~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with an array of media ids.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_media_ids(JaniceMediaIds* ids,
                                                    uint32_t num_ids);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+---------+-----------------------+-------------------------------------------+
| Name    | Type                  | Description                               |
+=========+=======================+===========================================+
| ids     | :ref:`JaniceMediaIds` | The array of media ids to free. Best      |
|         |                       | practice dictates the pointer should be   |
|         |                       | set to *NULL* after it is freed.          |
+---------+-----------------------+-------------------------------------------+
| num_ids | uint32_t              | The number of elements in *ids*.          |
+---------+-----------------------+-------------------------------------------+
