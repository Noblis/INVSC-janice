.. _gallery:

Gallery
=======

Overview
--------

This API defines a gallery object that represents a collection of templates. 
Galleries are useful in the 1-N use case (see :ref:`comparison`) when a user 
would like to query an unknown probe template against a set of known identities.
A naive implementation of a gallery might be a simple array of templates. Often 
however, implementations have optimized algorithms or data structures that can 
lead to more efficient search times. It is recommended that advanced data 
structures be implemented as part of a gallery. Please note however the rules on 
gallery modification:

1. Gallery objects may be modified (templates inserted or removed) at any time.
2. It is understood that some preprocessing might need to be done between 
   gallery modification and efficient search. A function
   :ref:`janice\_gallery\_prepare` exists for this purpose. The calling of this
   function is **OPTIONAL**. Please see :ref:`janice\_gallery\_prepare` for
   more information.

Structs
-------

.. _JaniceGalleryType:

JaniceGalleryType 
~~~~~~~~~~~~~~~~~

An opaque pointer to a struct that represents a gallery.

Typedefs
--------

.. _JaniceGallery:

JaniceGallery 
~~~~~~~~~~~~~

A pointer to a :ref:`JaniceGalleryType` object.

Signature 
^^^^^^^^^

::

    typedef struct JaniceGalleryType* JaniceGallery;

.. _JaniceConstGallery:

JaniceConstGallery 
~~~~~~~~~~~~~~~~~~

A pointer to a constant :ref:`JaniceGalleryType` object.

Signature 
^^^^^^^^^

::

    typedef const struct JaniceGalleryType* JaniceConstGallery;

.. _JaniceTemplateId:

JaniceTemplateId 
~~~~~~~~~~~~~~~~

A unique identifier for a template.

Signature 
^^^^^^^^^

::

    typedef uint32_t JaniceTemplateId;

.. _JaniceTemplateIds:

JaniceTemplateIds 
~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceTemplateId` objects.

Signature 
^^^^^^^^^

::

    typedef JaniceTemplateId* JaniceTemplateIds;

Functions
---------

.. _janice\_create\_gallery:

janice\_create\_gallery 
~~~~~~~~~~~~~~~~~~~~~~~

Create a :ref:`JaniceGallery` object from a list of templates and unique ids.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_gallery(JaniceConstTemplates tmpls,
                                                    const JaniceTemplateIds ids,
                                                    uint32_t num_tmpls,
                                                    JaniceGallery* gallery);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+---------+--------------------------------+-----------------------------------+
| Name    | Type                           | Description                       |
+=========+================================+===================================+
| tmpls   | :ref:`JaniceConstTemplates`    | An array of templates to add to   |
|         |                                | the gallery. This can be *NULL*   |
|         |                                | which would create an empty       |
|         |                                | gallery. Data should be copied    |
|         |                                | into the gallery, leaving the     |
|         |                                | templates in a valid state after  |
|         |                                | this operation.                   |
+---------+--------------------------------+-----------------------------------+
| ids     | const :ref:`JaniceTemplateIds` | A set of unique indentifiers to   |
|         |                                | associate with the input          |
|         |                                | templates. The *ith* id in this   |
|         |                                | array corresponds to the *ith*    |
|         |                                | input template. This array must be|
|         |                                | the same length as *tmpls*. If    |
|         |                                | *tmpls* is *NULL* this object     |
|         |                                | should also be *NULL*             |
+---------+--------------------------------+-----------------------------------+
| num\_tm | uint32\_t                      | The length of *tmpls* and *ids*.  |
| pls     |                                |                                   |
+---------+--------------------------------+-----------------------------------+
| gallery | :ref:`JaniceGallery` \*        | An uninitialized gallery object.  |
|         |                                | The implementor should allocate   |
|         |                                | this object during the function   |
|         |                                | call. The user is required to free|
|         |                                | the object by calling             |
|         |                                | :ref:`janice\_free\_gallery`.     |
+---------+--------------------------------+-----------------------------------+

Example 
^^^^^^^

::

    JaniceTemplates tmpls; // Where tmpls is a valid array of valid template
                           // objects created previously
    JaniceTemplateIds ids; // Where ids is a valid array of unique unsigned integers that
                           // is the same length as tmpls
    JaniceGallery gallery = NULL; // best practice to initialize to NULL

    if (janice_create_gallery(tmpls, ids, &gallery) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_gallery\_reserve:

janice\_gallery\_reserve 
~~~~~~~~~~~~~~~~~~~~~~~~

Reserve space in a gallery for N templates.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_reserve(size_t n);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+--------+-----------+------------------------------------------------+
| Name   | Type      | Description                                    |
+========+===========+================================================+
| n      | size\_t   | The number of templates to reserve space for   |
+--------+-----------+------------------------------------------------+

.. _janice\_gallery\_insert:

janice\_gallery\_insert 
~~~~~~~~~~~~~~~~~~~~~~~

Insert a template into a gallery object. The template data should be
copied into the gallery as the template may be deleted after this
function.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                    JaniceConstTemplate tmpl,
                                                    JaniceTemplateId id);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+----------------------------+-------------------------------------+
| Name      | Type                       | Description                         |
+===========+============================+=====================================+
| gallery   | :ref:`JaniceGallery`       | A gallery object to insert the      |
|           |                            | template into.                      |
+-----------+----------------------------+-------------------------------------+
| tmpl      | :ref:`JaniceConstTemplate` | A template object to insert into the|
|           |                            | gallery. The template has the role  |
|           |                            | Janice1NGallery. The template should|
|           |                            | be copied into the gallery. It must |
|           |                            | remain in a valid state after this  |
|           |                            | function call.                      |
+-----------+----------------------------+-------------------------------------+
| id        | :ref:`JaniceTemplateId`    | A unique id to associate with the   |
|           |                            | input template.                     |
+-----------+----------------------------+-------------------------------------+

Example 
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template object created
                         // previously
    JaniceTemplateId id; // Where id is a unique integer to associate with tmpl. This
                         // integer should not exist in the gallery
    JaniceGallery gallery; // Where gallery is a valid gallery object created
                           // previously

    if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_gallery\_remove:

janice\_gallery\_remove 
~~~~~~~~~~~~~~~~~~~~~~~

Remove a template from a gallery object using its unique id.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                    uint32_t id);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+----------+-------------------------+-----------------------------------------+
| Name     | Type                    | Description                             |
+==========+=========================+=========================================+
| gallery  | :ref:`JaniceGallery`    | A gallery object to remove a template   |
|          |                         | from. The template to remove is         |
|          |                         | indicated by its unique id.             |
+----------+-------------------------+-----------------------------------------+
| id       | :ref:`JaniceTemplateId` | A unique id associated with a template  |
|          |                         | int the gallery.                        |
+----------+-------------------------+-----------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template object created
                         // previously
    const JaniceTemplateId id = 0; // A unique integer id to associate with tmpl.

    JaniceGallery gallery; // Where gallery is a valid gallery object created
                           // previously that does not have a template with id '0'
                           // already inserted in it.

    // Insert the template with id 0
    if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
        // ERROR!

    // Now we can remove the template
    if (janice_gallery_remove(gallery, id) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_gallery\_prepare:

janice\_gallery\_prepare
~~~~~~~~~~~~~~~~~~~~~~~~

Prepare a gallery for search. Implementors can use this function as an
opportunity to streamline gallery objects to accelerate the search process. The
calling convention for this function is **NOT** specified by the API, this 
means that this function is not guaranteed to be called before 
:ref:`janice\_search`. It also means that templates can be added to a gallery 
before and after this function is called. Implementations should handle all of 
these calling conventions. However, users should be aware that this function may
be computationally expensive. They should strive to call it only at critical 
junctions before search and as few times as possible overall.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+-----------+----------------------+-------------------------------+
| Name      | Type                 | Description                   |
+===========+======================+===============================+
| gallery   | :ref:`JaniceGallery` | A gallery object to prepare   |
+-----------+----------------------+-------------------------------+

Example 
^^^^^^^

::

    JaniceTemplate* tmpls; // Where tmpls is a valid array of valid template
                           // objects created previously
    JaniceTemplateIds ids; // Where ids is a valid array of unique unsigned integers that
                           // is the same length as tmpls
    JaniceTemplate tmpl; // Where tmpl is a valid template object created
                         // previously
    JaniceTemplateId id; // Where id is a unique integer id to associate with tmpl.

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

.. _janice\_serialize\_gallery:

janice\_serialize\_gallery 
~~~~~~~~~~~~~~~~~~~~~~~~~~

Serialize a :ref:`JaniceGallery` object to a flat buffer.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_serialize_gallery(JaniceConstGallery gallery,
                                                       JaniceBuffer* data,
                                                       size_t* len);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+---------------------------+--------------------------------------+
| Name      | Type                      | Description                          |
+===========+===========================+======================================+
| gallery   | :ref:`JaniceConstGallery` | A gallery object to serialize        |
+-----------+---------------------------+--------------------------------------+
| data      | :ref:`JaniceBuffer`       | An uninitialized buffer to hold the  |
|           |                           | flattened data. The implementor      |
|           |                           | allocate this object during the      |
|           |                           | function call. The user is           |
|           |                           | responsible for freeing this object  |
|           |                           | by calling                           |
|           |                           | :ref:`janice\_free\_buffer`.         |
+-----------+---------------------------+--------------------------------------+
| len       | size\_t \*                | The length of the flat buffer after  |
|           |                           | it is allocated.                     |
+-----------+---------------------------+--------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery; // Where gallery is a valid gallery created
                           // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_gallery(gallery, &buffer, &buffer_len);

.. _janice\_deserialize\_gallery:

janice\_deserialize\_gallery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Deserialize a :ref:`JaniceGallery` object from a flat buffer.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_deserialize_gallery(const JaniceBuffer data,
                                                         size_t len,
                                                         JaniceGallery* gallery);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+----------+---------------------------+---------------------------------------+
| Name     | Type                      | Description                           |
+==========+===========================+=======================================+
| data     | const :ref:`JaniceBuffer` | A buffer containing data from a       |
|          |                           | flattened gallery object.             |
+----------+---------------------------+---------------------------------------+
| len      | size\_t                   | The length of the flat buffer.        |
+----------+---------------------------+---------------------------------------+
| gallery  | :ref:`JaniceGallery` \*   | An uninitialized gallery object. The  |
|          |                           | implementor should allocate this      |
|          |                           | object during the function call. The  |
|          |                           | user is responsible for freeing the   |
|          |                           | object by calling                     |
|          |                           | :ref:`janice\_free\_gallery`.         |
+----------+---------------------------+---------------------------------------+

Example
^^^^^^^

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    unsigned char buffer[buffer_len];

    FILE* file = fopen("serialized.gallery", "r");
    fread(buffer, 1, buffer_len, file);

    JaniceGallery gallery = NULL; // best practice to initialize to NULL
    janice_deserialize_gallery(buffer, buffer_len, gallery);

    fclose(file);

.. _janice\_read\_gallery:

janice\_read\_gallery 
~~~~~~~~~~~~~~~~~~~~~

Read a gallery from a file on disk. This method is functionally
equivalent to the following-

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    JaniceBuffer buffer[buffer_len];

    FILE* file = fopen("serialized.gallery", "r");
    fread(buffer, 1, buffer_len, file);

    JaniceGallery gallery = NULL; // best practice to initialize to NULL
    janice_deserialize_gallery(buffer, buffer_len, gallery);

    fclose(file);

It is provided for memory efficiency and ease of use when reading from
disk.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                                  JaniceGallery* gallery);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+------------+-------------------------+---------------------------------------+
| Name       | Type                    | Description                           |
+============+=========================+=======================================+
| filename   | const char\*            | The path to a file on disk            |
+------------+-------------------------+---------------------------------------+
| gallery    | :ref:`JaniceGallery` \* | An uninitialized gallery object. The  |
|            |                         | implementor should allocate this      |
|            |                         | object during the function call. The  |
|            |                         | user is responsible for freeing this  |
|            |                         | object by calling                     |
|            |                         | :ref:`janice\_free\_gallery`.         |
+------------+-------------------------+---------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery = NULL;
    if (janice_read_gallery("example.gallery", &gallery) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_write\_gallery:

janice\_write\_gallery 
~~~~~~~~~~~~~~~~~~~~~~

Write a gallery to a file on disk. This method is functionally
equivalent to the following-

::

    JaniceGallery gallery; // Where gallery is a valid gallery created previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_gallery(gallery, &buffer, &buffer_len);

    FILE* file = fopen("serialized.gallery", "w+");
    fwrite(buffer, 1, buffer_len, file);

    fclose(file);

It is provided for memory efficiency and ease of use when writing to
disk.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_write_gallery(JaniceConstGallery gallery,
                                                   const char* filename);

ThreadSafety 
^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+------------+---------------------------+-------------------------------------+
| Name       | Type                      | Description                         |
+============+===========================+=====================================+
| gallery    | :ref:`JaniceConstGallery` | The gallery object to write to disk.|
+------------+---------------------------+-------------------------------------+
| filename   | const char \*             | The path to a file on disk          |
+------------+---------------------------+-------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery; // Where gallery is a valid gallery created previously
    if (janice_write_gallery(gallery, "example.gallery") != JANICE_SUCCESS)
        // ERROR!

.. _janice\_free\_gallery:

janice\_free\_gallery 
~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceGalleryType` object.

Signature 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);

Thread Safety 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+----------+-------------------------+-----------------------------------------+
| Name     | Type                    | Description                             |
+==========+=========================+=========================================+
| gallery  | :ref:`JaniceGallery` \* | A gallery object to free. Best practice |
|          |                         | dicates the pointer should be set to    |
|          |                         | *NULL* after it is freed.               |
+----------+-------------------------+-----------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery; // Where gallery is a valid gallery object created previously
    if (janice_free_gallery(&gallery) != JANICE_SUCCESS)
        // ERROR!
