.. io:

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
correspond to a single image or frame and an entire video respectively.
These interfaces allow pixel-level access for implementations and can be
changed independently to work with new formats.

Structs
-------

.. _JaniceImageType:

JaniceImageType
~~~~~~~~~~~~~~~

An interface representing a single frame or an image

Fields
^^^^^^

+-----------+---------------------+--------------------------------------------+
| Name      | Type                | Description                                |
+===========+=====================+============================================+
| channels  | uint32\_t           | The number of channels in the image.       |
+-----------+---------------------+--------------------------------------------+
| rows      | uint32\_t           | The number of rows in the image.           |
+-----------+---------------------+--------------------------------------------+
| cols      | uint32\_t           | The number of columns in the image.        |
+-----------+---------------------+--------------------------------------------+
| data      | :ref:`JaniceBuffer` | A contiguous, row-major array containing   |
|           |                     | pixel data.                                |
+-----------+---------------------+--------------------------------------------+
| owner     | bool                | True if the image owns its data and should |
|           |                     | delete it, false otherwise.                |
+-----------+---------------------+--------------------------------------------+

.. _JaniceMediaIteratorState:

JaniceMediaIteratorState
~~~~~~~~~~~~~~~~~~~~~~~~

A void pointer to a user-defined structure that contains state required
for a :ref:`JaniceMediaIteratorType`.

.. _JaniceMediaIteratorType:

JaniceMediaIteratorType
~~~~~~~~~~~~~~~~~~~~~~~

An interface representing a single image or video.
JaniceMediaIteratorType implements an iterator interface on media to
enable lazy loading via function pointers.

Fields
^^^^^^

+---------+-----------------------------------+--------------------------------+
| Name    | Type                              | Description                    |
+=========+===================================+================================+
| next    | :ref:`JaniceError`\(              | A function pointer that        |
|         | :ref:`JaniceMediaIteratorType` \*,| advances the iterators one     |
|         | :ref:`JaniceImage` \*\)           | frame. The next frame or video |
|         |                                   | image should be stored in the  |
|         |                                   | :ref:`JaniceImage` parameter.  |
+---------+-----------------------------------+--------------------------------+
| seek    | :ref:`JaniceError`\(              | A function pointer that        |
|         | :ref:`JaniceMediaIteratorType` \*,| advances the iterator to a     |
|         | uint32\_t\)                       | specific frame. This function  |
|         |                                   | is not applicable to images.   |
+---------+-----------------------------------+--------------------------------+
| get     | :ref:`JaniceError`\(              | A function pointer that        |
|         | :ref:`JaniceMediaIteratorType` \*,| advances the iterator to a     |
|         | :ref:`JaniceImage`\*,             | specific frame and retrieves   |
|         | uint32\_t\)                       | that frame. This function is   |
|         |                                   | not applicable to images.      |
+---------+-----------------------------------+--------------------------------+
| tell    | :ref:`JaniceError`\(              | A function pointer to report   |
|         | :ref:`JaniceMediaIteratorType` \*,| the current position of the    |
|         | uint32\_t \*\)                    | iterator. This function is not |
|         |                                   | applicable to images.          |
+---------+-----------------------------------+--------------------------------+
| free\_i | :ref:`JaniceError`\(              | A function pointer to free an  |
|         | :ref:`JaniceImage` \*\)           | image allocated by *next* or   |
|         |                                   | *get*.                         |
+---------+-----------------------------------+--------------------------------+
| free    | :ref:`JaniceError`\(              | A function pointer to free a   |
|         | :ref:`JaniceMediaIteratorType`    | :ref:`JaniceMediaIteratorType` |
|         | \*\*\)                            | object.                        |
+---------+-----------------------------------+--------------------------------+

Typedefs
--------

.. _JaniceBuffer:

JaniceBuffer
~~~~~~~~~~~~

An array of uint8\_t

Signature
^^^^^^^^^

::

    typedef uint8_t* JaniceBuffer;

.. _JaniceImage:

JaniceImage
~~~~~~~~~~~

A pointer to a :ref:`JaniceImageType` object.

Signature
^^^^^^^^^

::

    typedef struct JaniceImageType* JaniceImage;

.. _JaniceConstImage:

JaniceConstImage
~~~~~~~~~~~~~~~~

A pointer to a constant :ref:`JaniceImageType` object.

Signature
^^^^^^^^^

::

    typedef const struct JaniceImageType* JaniceConstImage;

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

A pointer to an array of :ref:`JaniceMediaIterator` objects.

Signature
^^^^^^^^^

::

    typedef struct JaniceMediaIterator* JaniceMediaIterators;


Functions
---------

.. _janice_free_buffer:

janice\_free\_buffer
~~~~~~~~~~~~~~~~~~~~

Release the memory for an allocated buffer.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_buffer(JaniceBuffer* buffer);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant

Parameters
^^^^^^^^^^

+----------+------------------------+----------------------+
| Name     | Type                   | Description          |
+==========+========================+======================+
| buffer   | :ref:`JaniceBuffer` \* | The buffer to free   |
+----------+------------------------+----------------------+

.. _janice_image_access:

janice\_image\_access
~~~~~~~~~~~~~~~~~~~~~

Get a pixel value at a given row, column and channel.

Signature
^^^^^^^^^

::

    inline uint8_t janice_image_access(JaniceConstImage image, uint32_t channel, uint32_t row, uint32_t col);

Thread Safety
^^^^^^^^^^^^^

This function is reentrant

Parameters
^^^^^^^^^^

+-----------+-------------------------+----------------------------------------+
| Name      | Type                    | Description                            |
+===========+=========================+========================================+
| image     | :ref:`JaniceConstImage` | An image object                        |
+-----------+-------------------------+----------------------------------------+
| channel   | uint32\_t               | The channel to access. Must be less    |
|           |                         | image->channels.                       |
+-----------+-------------------------+----------------------------------------+
| row       | uint32\_t               | The row to access. Must be less than   |
|           |                         | image->rows.                           |
+-----------+-------------------------+----------------------------------------+
| col       | uint32\_t               | The column to access. Must be less     | 
|           |                         | than image->cols.                      |
+-----------+-------------------------+----------------------------------------+
