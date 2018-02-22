.. _io:

I/O
===

Overview
--------

As a computer vision API it is a requirement that images and videos are
loaded into a common structure that can be processed by the rest of the
API. In this case, we strive to isolate the I/O functions from the rest
of the API. This serves three purposes:

1. It allows implementations to be agnostic to the method and type of
   image storage, compression techniques and other factors
2. It keeps implementations from having to worry about licenses, patents
   and other factors that can arise from distributing proprietary image
   formats
3. It allows implementations to be "future-proof" with regards to future
   developments of image or video formats

To accomplish this goal the API defines a simple interface of two
structures, :ref:`JaniceImageType` and :ref:`JaniceMediaIteratorType` which
correspond to a single image or frame and an entire media respectively.
These interfaces allow pixel-level access for implementations and can be
changed independently to work with new formats.

Structs
-------

.. _JaniceImageType:

JaniceImageType
~~~~~~~~~~~~~~~

A structure representing a single frame or an image

Fields
^^^^^^

+----------+-----------+------------------------------------------------------------------------+
|   Name   |   Type    |                              Description                               |
+==========+===========+========================================================================+
| channels | uint32\_t | The number of channels in the image.                                   |
+----------+-----------+------------------------------------------------------------------------+
| rows     | uint32\_t | The number of rows in the image.                                       |
+----------+-----------+------------------------------------------------------------------------+
| cols     | uint32\_t | The number of columns in the image.                                    |
+----------+-----------+------------------------------------------------------------------------+
| data     | uint8_t\* | A contiguous, row-major array containing pixel data.                   |
+----------+-----------+------------------------------------------------------------------------+
| owner    | bool      | True if the image owns its data and should delete it, false otherwise. |
+----------+-----------+------------------------------------------------------------------------+

.. _JaniceImage:

JaniceImage
~~~~~~~~~~~

A pointer to a :ref:`JaniceImageType`.

Signature
^^^^^^^^^

::

    typedef struct JaniceImageType* JaniceImage;

.. _JaniceMediaIteratorState:

JaniceMediaIteratorState
~~~~~~~~~~~~~~~~~~~~~~~~

A void pointer to a user-defined structure that contains state required
for a :ref:`JaniceMediaIteratorType`.

.. _JaniceMediaIteratorType:

JaniceMediaIteratorType
~~~~~~~~~~~~~~~~~~~~~~~

An interface representing a single image, a sparse selection of video frames
or a full video. JaniceMediaIteratorType implements an iterator interface on 
media to enable lazy loading via function pointers.

Fields
^^^^^^

+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|      Name      |                                          Type                                           |                                                                                                                                                                                     Description                                                                                                                                                                                      |
+================+=========================================================================================+======================================================================================================================================================================================================================================================================================================================================================================================+
| is_video       | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*, bool\*)                           | A function pointer to check if the iterator is a video. If the iterator is a video or a sparse collection of video frames the second parameter should be set to true, otherwise it should be set to false. If the second parameter can be set this function should return *JANICE_SUCCESS*.                                                                                          |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| get_frame_rate | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*, float\*)                          | A function pointer to get the frame rate of a video. If the iterator is a video the second parameter should be set to the video frame rate and this function should return *JANICE_SUCCESS*. Otherwise this function should return *JANICE_INVALID_MEDIA*.                                                                                                                           |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| next           | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*, :ref:`JaniceImage`\*\)            | A function pointer that advances the iterator one frame. The next video frame or image should be stored in the :ref:`JaniceImage` parameter. If the next frame or image is stored in the image parameter this function should return *JANICE_SUCCESS*.                                                                                                                               |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| seek           | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*, uint32\_t\)                       | A function pointer that advances the iterator to a specific frame. If the iterator is a video and the seek is successful, this function should return *JANICE_SUCCESS*. If the iterator is an image, this function should return *JANICE_INVALID_MEDIA*.                                                                                                                             |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| get            | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*, :ref:`JaniceImage`\*, uint32\_t\) | A function pointer that finds a specific frame and stores that frame in the :ref:`JaniceImage` parameter. This function should not modify the state of the iterator. If the iterator is a video and the seek and subsequent retreival are successful, this function should return *JANICE_SUCCESS*. If the iterator is an image, this function should return *JANICE_INVALID_MEDIA*. |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tell           | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*, uint32\_t\*\)                     | A function pointer that reports the current position of the iterator. If the iterator is a video and the current position is successfully queried this function should return *JANICE_SUCCESS*. If the iterator is an image, this function should return *JANICE_INVALID_MEDIA*.                                                                                                     |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| reset          | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*\)                                  | A function that resets an iterator to an initial, valid state.                                                                                                                                                                                                                                                                                                                       |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| free\_image    | :ref:`JaniceError`\(:ref:`JaniceImage`\*\)                                              | A function pointer to free a :ref:`JaniceImage` object.                                                                                                                                                                                                                                                                                                                              |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| free           | :ref:`JaniceError`\(:ref:`JaniceMediaIteratorType`\*\*\)                                | A function pointer to free a :ref:`JaniceMediaIteratorType` object.                                                                                                                                                                                                                                                                                                                  |
+----------------+-----------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Typedefs
--------

.. _JaniceMediaIterator:

JaniceMediaIterator
~~~~~~~~~~~~~~~~~~~

A pointer to a :ref:`JaniceMediaIteratorType` object. 

Signature
^^^^^^^^^

::

    typedef struct JaniceMediaIteratorType* JaniceMediaIterator;

.. _JaniceMediaIterators:

JaniceMediaIterators
~~~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceMediaIterator` objects.

Fields
^^^^^^

+--------+------------------------------+-------------------------------------+
|  Name  |             Type             |             Description             |
+========+==============================+=====================================+
| media  | :ref:`JaniceMediaIterator`\* | An array of media iterator objects. |
+--------+------------------------------+-------------------------------------+
| length | size_t                       | The number of elements in *media*   |
+--------+------------------------------+-------------------------------------+

.. _JaniceMediaIteratorsGroup:

JaniceMediaIteratorsGroup
~~~~~~~~~~~~~~~~~~~~~~~~~

A structure to represent a list of :ref:`JaniceMediaIterators` objects.

Fields
^^^^^^

+--------+-----------------------------+-----------------------------------+
|  Name  |            Type             |            Description            |
+========+=============================+===================================+
| group  | :ref:`JaniceMediaIterators` | An array of media objects.        |
+--------+-----------------------------+-----------------------------------+
| length | size\_t                     | The number of elements in *group* |
+--------+-----------------------------+-----------------------------------+
