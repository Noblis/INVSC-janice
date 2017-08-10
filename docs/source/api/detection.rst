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

.. _JaniceDetectionIteratorType:

JaniceDetectionIteratorType
~~~~~~~~~~~~~~~~~~~~~~~~~~~

An opaque pointer to an iterator class through a detection. If the
detection was computed from an image, the iterator should only move over
a single value. If the detection was computed from a video, the iterator
should move over an array of elements, the length of which is less than
or equal to the number of frames in the video, and might be sparse (i.e.
frames can be skipped).

.. _JaniceDetectionType:

JaniceDetectionType
~~~~~~~~~~~~~~~~~~~

An opaque pointer to a struct that represents a detection. See :ref:`detection`
for more information.

Typedefs
--------

.. _JaniceDetectionIterator:

JaniceDetectionIterator
~~~~~~~~~~~~~~~~~~~~~~~

A pointer to a :ref:`JaniceDetectionIteratorType` object.

Signature
^^^^^^^^^

::

    typedef struct JaniceDetectionIteratorType* JaniceDetectionIterator;

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
    
Functions
---------

.. _janice\_detection\_it\_next:

janice\_detection\_it\_next
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Get the next element in a detection track.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_detection_it_next(JaniceDetectionIterator it,
                                                       JaniceRect* rect,
                                                       uint32_t* frame,
                                                       float* confidence);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

.. _detection\_confidence:

Confidence
^^^^^^^^^^

The confidence value indicates a likelihood that the rectangle actually
bounds an object of interest. It is **NOT** required to be a probability
and often only has meaning relative to other confidence values from the
same algorithm. The only restriction is that a larger confidence value
indicates a greater likelihood that the rectangle bounds an object.

Parameters
^^^^^^^^^^

+--------------+---------------------------------+-----------------------------+ 
| Name         | Type                            | Description                 |
+==============+=================================+=============================+
| it           | :ref:`JaniceDetectionIterator`  | A detection iterator        |
|              |                                 | object.                     |
+--------------+---------------------------------+-----------------------------+
| rect         | :ref:`JaniceRect` \*            | The location of a object of |
|              |                                 | interest.                   |
+--------------+---------------------------------+-----------------------------+
| frame        | uint32\_t \*                    | The frame index for an      |
|              |                                 | object of interest.         |
+--------------+---------------------------------+-----------------------------+
| confidence   | float \*                        | The                         |
|              |                                 | :ref:`detection\_confidence`|
|              |                                 | of the location.            |
+--------------+---------------------------------+-----------------------------+

.. _janice\_detection\_it\_reset:

janice\_detection\_it\_reset
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Reset an iterator back to its initial state.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_detection_it_reset(JaniceDetectionIterator it);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+--------+--------------------------------+---------------------------------+
| Name   | Type                           | Description                     |
+========+================================+=================================+
| it     | :ref:`JaniceDetectionIterator` | The iterator object to reset.   |
+--------+--------------------------------+---------------------------------+

.. _janice\_free\_detection\_it:

janice\_free\_detection\_it
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a detection iterator object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_detection_it(JaniceDetectionIterator* it);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+--------+-------------------------------------+--------------------------------+
| Name   | Type                                | Description                    |
+========+=====================================+================================+
| it     | :ref:`JaniceDetectionIterator` \*   | The iterator object to free.   |
+--------+-------------------------------------+--------------------------------+

.. _janice\_create\_detection:

janice\_create\_detection
~~~~~~~~~~~~~~~~~~~~~~~~~

Create a detection from a known rectangle. This is useful if a human has
identified an object of interest and would like to run subsequent API
functions on it. In the case where the input media is a video the given
rectangle is considered an initial sighting of the object of interest.
The implementation may detect additional sightings of the object in
successive frames.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_detection(JaniceMediaIterator media,
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
|           |                            | using :ref:`janice\_free\_detection`|
|           |                            | .                                   |
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

When the input media is a video many implementations will implement a
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

+---------------+----------------------------+------------------------------------+
| Name          | Type                       | Description                        |
+===============+============================+====================================+
| media         | :ref:`JaniceMediaIterator` | A media object to run detection    |
|               |                            | on.                                |
+---------------+----------------------------+------------------------------------+
| min\_object\_ | uint32\_t                  | A minimum object size. See         |
| size          |                            | :ref:`detection\_min\_object\_size`|
+---------------+----------------------------+------------------------------------+
| detections    | :ref:`JaniceDetections` \* | An uninitialized array to hold     |
|               |                            | all of the detections detected     |
|               |                            | in the media object. This          |
|               |                            | object should be allocated by      |
|               |                            | the implementor during the         |
|               |                            | call. The user is required to      |
|               |                            | free the object by calling         |
|               |                            | :ref:`janice\_free\_detections`.   |
+---------------+----------------------------+------------------------------------+
| num\_         | uint32\_t\*                | The number of detections           |
| detections    |                            | returned in the *detections*       |
|               |                            | array.                             |
+---------------+----------------------------+------------------------------------+

Example
^^^^^^^

::

    JaniceMedia media; // Where media is a valid media object created previously
    const uint32_t min_object_size = 24; // Only find objects where the smaller
                                         // side is > 24 pixels
    JaniceDetection* detections = NULL; // best practice to initialize to NULL
    uint32_t num_detections; // Will be populated with the size of detections

    if (janice_detect(media, min_object_size, &detections, &num_detections) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_create\_detection\_it:

janice\_create\_detection\_it
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create an iterator to iterate over detection elements.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_detection_it(JaniceConstDetection detection,
                                                         JaniceDetectionIterator* it);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+-----------------------------------+-----------------------------------+
| Name      | Type                              | Description                       |
+===========+===================================+===================================+
| detection | :ref:`JaniceConstDetection`       | The detection object to           |
|           |                                   | create an iterator from.          |
+-----------+-----------------------------------+-----------------------------------+
| it        | :ref:`JaniceDetectionIterator` \* | An uninitialized detection        |
|           |                                   | iterator object. The              |
|           |                                   | implementor should allocate       |
|           |                                   | this object during the            |
|           |                                   | function call. Users are          |
|           |                                   | required to free the object       |
|           |                                   | with                              |
|           |                                   | :ref:`janice\_free\_detection\_it`|
|           |                                   | .                                 |
+-----------+-----------------------------------+-----------------------------------+

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

+------------+----------------------------+------------------------------------+
| Name       | Type                       | Description                        |
+============+============================+====================================+
| detections | :ref:`JaniceDetections` \* | An array of detections to free.    |
|            |                            | Best practice dictates the pointer |
|            |                            | should be set to *NULL* after it   |
|            |                            | is freed.                          |
+------------+----------------------------+------------------------------------+
