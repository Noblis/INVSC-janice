.. _io:

I/O
===

Overview
--------

As a computer vision API it is a requirement that images and videos are loaded
into a common structure that can be processed by the rest of the API. In this
case, we strive to isolate the I/O functions from the rest of the API. This
serves three purposes:

1. It allows implementations to be agnostic to the method and type of
   image storage, compression techniques and other factors
2. It keeps implementations from having to worry about licenses, patents
   and other factors that can arise from distributing proprietary image
   formats
3. It allows implementations to be "future-proof" with regards to future
   developments of image or video formats

To accomplish this goal the API defines a simple interface of two structures,
:ref:`JaniceImage` and :ref:`JaniceMediaIterator` which correspond to a single
image or frame and an entire media respectively. These interfaces allow
pixel-level access for implementations and can be changed independently to work
with new formats.

Structs
-------

.. _JaniceImage:

JaniceImage
~~~~~~~~~~~

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

.. _JaniceMediaIteratorState:

JaniceMediaIteratorState
~~~~~~~~~~~~~~~~~~~~~~~~

A void pointer to a user-defined structure that contains state required for a
:ref:`JaniceMediaIterator`.

.. _JaniceMediaIterator:

JaniceMediaIterator
~~~~~~~~~~~~~~~~~~~

An interface representing a single image, a sparse selection of video frames or
a full video. JaniceMediaIterator implements an iterator interface on media
to enable lazy loading via function pointers.

.. _is_video:

is\_video
^^^^^^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, bool* video)

The function sets :code:`video` to True if :code:`it` is a video. Otherwise, it
sets :code:`video` to False. :code:`it` should be considered a video if multiple
still images can be retrieved with successive calls to :ref:`next`. This
function should return :code:`JANICE_SUCCESS` if :code:`video` can be set to
True or False.

.. _get_frame_rate:

get\_frame\_rate
^^^^^^^^^^^^^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, float* frame_rate)

The function sets :code:`frame_rate` to the frame rate of :code:`it`, if that
information is available. If :code:`frame_rate` can be set to a value this
function should return :code:`JANICE_SUCCESS`, otherwise it should return
:code:`JANICE_INVALID_MEDIA`. In the case of downsampling, this should return
the observed frame rate.

.. _get_physical_frame_rate:

get\_physical\_frame\_rate
^^^^^^^^^^^^^^^^^^^^^^^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, float* physical_frame_rate)

The physical frame rate is the actual frame rate of the video, independent of
processing done by media iterator. The function sets :code:`physical_frame_rate`
to the physical frame rate of :code:`it`, if that information is available. If
:code:`frame_rate` can be set to a value this function should return
:code:`JANICE_SUCCESS`, otherwise it should return :code:`JANICE_INVALID_MEDIA`.

.. _next:

next
^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, JaniceImage* img)

The functions sets :code:`img` to the next still image or frame from :code:`it`
and and advances :code:`it` one position. If :code:`img` is successfully set
this function should return :code:`JANICE_SUCCESS`. If :code:`it` has already
iterated through all available images, this function should return
:code:`JANICE_MEDIA_AT_END`. Otherwise, a relevant error code should be
returned.

.. _seek:

seek
^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, uint32_t frame)


The function sets the internal state of :code:`it` such that a successive call
to :ref:`next` will return the image with index :code:`frame`. If :code:`it` is
an image, this function should work for :code:`frame` == 0, in which case it is
equivalent to :ref:`reset`, otherwise :code:`JANICE_INVALID_MEDIA` should be
returned. If :code:`it` is a video, the implementation may optionally do bounds
checking on :code:`frame`. If the seek is successful, :code:`JANICE_SUCCESS`
should be returned.

.. _get:

get
^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, JaniceImage* img, uint32_t frame)

This function gets a specific frame from :code:`it` and stores it in
:code:`img`. It should not modify the internal state of :code:`it`. If
:code:`it` is an image, this function should work or :code`frame` == 0. If
:code:`frame` != 0 and :code:`it` is an image, this function should return
:code:`JANICE_INVALID_MEDIA`. If :code:`it` is a video, the implementation may
optionally do bounds checking on :code:`frame`. If the get is successful, this
function should return :code:`JANICE_SUCCESS`. If the get is not successful, an
appropriate error code should be returned and :code:`it` may be left in an
undefined state.

.. _tell:

tell
^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, uint32_t* frame)

Get the current position of :code:`it` and store it in :code:`frame`. If
:code:`it` is an image, this function should return
:code:`JANICE_INVALID_MEDIA`. If :code:`it` is a video and its position can be
successfully queried, this function should return :code:`JANICE_SUCCESS`.
Otherwise, an appropriate error code should be returned.

.. _reset:

reset
^^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it)

Reset :code:`it` to an initial valid state. This function should return
:code:`JANICE_SUCCESS` if :code:`it` can be reset, otherwise an appropriate
error code should be returned.

.. _physical_frame:

physical\_frame
^^^^^^^^^^^^^^^

A function pointer with signature:

::

    JaniceError(const JaniceMediaIterator* it, uint32_t frame, uint32_t* physical_frame)

Map an observed frame to a physical frame. If a mapping is possible this
function should return :code:`JANICE_SUCCESS`. Otherwise, an appropriate error
code should be returned.

.. _free_image:

free\_image
^^^^^^^^^^^

A function pointer with signature:

::

    JaniceError(JaniceImage* img)

Free any memory associated with :code:`img`. :ref:`free_image` should be called with
the same iterator that allocated :code:`img` with a call to either :ref:`next` or
:ref:`get`. This function should return :code:`JANICE_SUCCESS` if :code:`img` is 
successfully freed, otherwise an appropriate error code should be returned.

.. _free:

free
^^^^

A function pointer with signature:

::

    JaniceError(JaniceMediaIterator** it)

Free any memory associated with :code:`it`. This function should return 
:code:`JANICE_SUCCESS` if :code:`it` is freed successfully, otherwise and appropriate
error code should be returned.

Fields
^^^^^^

+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
|            Name            |                                        Type                                         |                                                          Description                                                           |
+============================+=====================================================================================+================================================================================================================================+
| is\_video                  | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, bool\*\)                          | See :ref:`is_video`.                                                                                                           |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| get\_frame\_rate           | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, float\*\)                         | See :ref:`get_frame_rate`.                                                                                                     |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| get\_physical\_frame\_rate | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, float\*\)                         | See :ref:`get_physical_frame_rate`.                                                                                            |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| next                       | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, :ref:`JaniceImage`\*\)            | See :ref:`next`.                                                                                                               |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| seek                       | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, uint32\_t\)                       | See :ref:`seek`.                                                                                                               |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| get                        | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, :ref:`JaniceImage`\*, uint32\_t\) | See :ref:`get`.                                                                                                                |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| tell                       | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, uint32\_t\*\)                     | See :ref:`tell`.                                                                                                               |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| reset                      | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*\)                                  | See :ref:`reset`.                                                                                                              |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| physical\_frame            | :ref:`JaniceError`\(const :ref:`JaniceMediaIterator`\*, uint32\_t, uint32\_t\*\)          | See :ref:`physical_frame`.                                                                                                     |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| free\_image                | :ref:`JaniceError`\(:ref:`JaniceImage`\*\)                                          | See :ref:`free_image`.                                                                                                         |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| free                       | :ref:`JaniceError`\(:ref:`JaniceMediaIterator`\*\*\)                                | See :ref:`free`.                                                                                                               |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+
| _internal                  | :ref:`JaniceMediaIteratorState`                                                     | A pointer to memory meant for internal use only. The implementation may use this to store persistent state about the iterator. |
+----------------------------+-------------------------------------------------------------------------------------+--------------------------------------------------------------------------------------------------------------------------------+

.. _JaniceMediaIterators:

JaniceMediaIterators
~~~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceMediaIterator` objects.

Fields
^^^^^^

+--------+------------------------------+-----------------------------------------+
|  Name  |             Type             |               Description               |
+========+==============================+=========================================+
| media  | :ref:`JaniceMediaIterator`\* | An array of media iterator objects.     |
+--------+------------------------------+-----------------------------------------+
| length | size_t                       | The number of elements in :code:`media` |
+--------+------------------------------+-----------------------------------------+

.. _JaniceMediaIteratorsGroup:

JaniceMediaIteratorsGroup
~~~~~~~~~~~~~~~~~~~~~~~~~

A structure to represent a list of :ref:`JaniceMediaIterators` objects.

Fields
^^^^^^

+--------+-----------------------------+-----------------------------------------+
|  Name  |            Type             |               Description               |
+========+=============================+=========================================+
| group  | :ref:`JaniceMediaIterators` | An array of media objects.              |
+--------+-----------------------------+-----------------------------------------+
| length | size\_t                     | The number of elements in :code:`group` |
+--------+-----------------------------+-----------------------------------------+
