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
   :ref:`janice_gallery_prepare` exists for this purpose. The calling of this
   function is **OPTIONAL**. Please see :ref:`janice_gallery_prepare` for
   more information.

Structs
-------

.. _JaniceGalleryType:

JaniceGalleryType 
~~~~~~~~~~~~~~~~~

A struct that represents a gallery.

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

.. _JaniceTemplateId:

JaniceTemplateId
~~~~~~~~~~~~~~~~

A unique identifier for a template.

Signature
^^^^^^^^^

::

    typedef size_t JaniceTemplateId;
    
.. _JaniceTemplateIds:

JaniceTemplateIds
~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceTemplateId` objects.

Fields
^^^^^^

+--------+---------------------------+---------------------------------------+
|  Name  |           Type            |              Description              |
+========+===========================+=======================================+
| ids    | :ref:`JaniceTemplateId`\* | An array of template id objects       |
+--------+---------------------------+---------------------------------------+
| length | size\_t                   | The number of elements in :code:`ids` |
+--------+---------------------------+---------------------------------------+

.. _JaniceTemplateIdsGroup:

JaniceTemplateIdsGroup
~~~~~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceTemplateIds` objects.

Fields
^^^^^^

+--------+----------------------------+-----------------------------------------+
|  Name  |            Type            |               Description               |
+========+============================+=========================================+
| group  | :ref:`JaniceTemplateIds`\* | An array of template ids objects.       |
+--------+----------------------------+-----------------------------------------+
| length | size\_t                    | The number of elements in :code:`group` |
+--------+----------------------------+-----------------------------------------+

Functions
---------

.. _janice_create_gallery:

janice\_create\_gallery 
~~~~~~~~~~~~~~~~~~~~~~~

Create a :ref:`JaniceGallery` object from a list of templates and unique ids.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_gallery(JaniceTemplates tmpls,
                                                    JaniceTemplateIds ids,
                                                    JaniceGallery* gallery);

Thread Safety 
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters 
^^^^^^^^^^

+---------+--------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |           Type           |                                                                                                                                                                 Description                                                                                                                                                                  |
+=========+==========================+==============================================================================================================================================================================================================================================================================================================================================+
| tmpls   | :ref:`JaniceTemplates`   | An array of templates to add to the gallery. This can be :code:`NULL` which would create an empty gallery. Data should be copied into the gallery. It is valid to pass an array with length 0 into this function, in which case an empty gallery should be initialized. This structure must have the same number of elements as :code:`ids`. |
+---------+--------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ids     | :ref:`JaniceTemplateIds` | A set of unique indentifiers to associate with the templates in :code:`tmpls`. The :code:`ith` id in this array corresponds to the :code:`ith` input template. This structure must have the same number of elements as :code:`tmpls`.                                                                                                        |
+---------+--------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| gallery | :ref:`JaniceGallery`\*   | An uninitialized gallery object. The implementor should allocate this object during the function call. The user is required to free this object by calling :ref:`janice_free_gallery`.                                                                                                                                                       |
+---------+--------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

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

.. _janice_gallery_reserve:

janice\_gallery\_reserve 
~~~~~~~~~~~~~~~~~~~~~~~~

Reserve space in a gallery for N templates. This can save repeated allocations
when doing multiple iterative inserts.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_reserve(JaniceGallery gallery,
                                                     size_t n);

Thread Safety 
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters 
^^^^^^^^^^

+---------+----------------------+-----------------------------------------------+
|  Name   |         Type         |                  Description                  |
+=========+======================+===============================================+
| gallery | :ref:`JaniceGallery` | The gallery to reserve space in.              |
+---------+----------------------+-----------------------------------------------+
| n       | size\_t              | The number of templates to reserve space for. |
+---------+----------------------+-----------------------------------------------+

.. _janice_gallery_insert:

janice\_gallery\_insert 
~~~~~~~~~~~~~~~~~~~~~~~

Insert a template into a gallery object. The template data should be
copied into the gallery as the template may be deleted after this
function.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                    JaniceTemplate tmpl,
                                                    JaniceTemplateId id);

Thread Safety 
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+-------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |          Type           |                                                                                                           Description                                                                                                            |
+=========+=========================+==================================================================================================================================================================================================================================+
| gallery | :ref:`JaniceGallery`    | A gallery object to insert the template into.                                                                                                                                                                                    |
+---------+-------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpl    | :ref:`JaniceTemplate`   | A template object to insert into the gallery. The template was created with the :code:`Janice1NGallery` role. The template should be copied into the gallery. This object must remain in a valid state after this function call. |
+---------+-------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| id      | :ref:`JaniceTemplateId` | A unique id to associate with the input template. If the id is not unique the implementor should return :code:`JANICE_DUPLICATE_ID`.                                                                                             |
+---------+-------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

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

.. _janice_gallery_insert_batch:

janice\_gallery\_insert\_batch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Insert a batch of templates into a gallery.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_insert_batch(JaniceGallery gallery,
                                                          JaniceTemplates tmpls,
                                                          JaniceTemplateIds ids);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+--------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |           Type           |                                                                                                                                                       Description                                                                                                                                                       |
+=========+==========================+=========================================================================================================================================================================================================================================================================================================================+
| gallery | :ref:`JaniceGallery`     | The gallery to insert the templates into.                                                                                                                                                                                                                                                                               |
+---------+--------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls   | :ref:`JaniceTemplates`   | The array of templates to insert in to the gallery. Each template was created with the :code:`Janice1NGallery` role. Each template should be copied into the gallery by the implementor and must remain in a valid state after this function call. This structure must have the same number of elements as :code:`ids`. |
+---------+--------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ids     | :ref:`JaniceTemplateIds` | The array of unique ids to associate with :code:`tmpls`. The :code:`ith` id in this structure corresponds to the :code:`ith` template in :code:`tmpls`. This structure must have the same number of elements as :code:`tmpls`.                                                                                          |
+---------+--------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+


.. _janice_gallery_remove:

janice\_gallery\_remove 
~~~~~~~~~~~~~~~~~~~~~~~

Remove a template from a gallery object using its unique id.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                    JaniceTemplateId id);

Thread Safety 
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |          Type           |                                                                                    Description                                                                                     |
+=========+=========================+====================================================================================================================================================================================+
| gallery | :ref:`JaniceGallery`    | The gallery object to remove a template from.                                                                                                                                      |
+---------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| id      | :ref:`JaniceTemplateId` | The unique identifier for the template to remove from the gallery. If no template with the given ID is found in the gallery this function should return :code:`JANICE_MISSING_ID`. |
+---------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template object created
                         // previously
    JaniceTemplateId id = 0; // A unique integer id to associate with tmpl.

    JaniceGallery gallery; // Where gallery is a valid gallery object created
                           // previously that does not have a template with id '0'
                           // already inserted in it.

    // Insert the template with id 0
    if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
        // ERROR!

    // Now we can remove the template
    if (janice_gallery_remove(gallery, id) != JANICE_SUCCESS)
        // ERROR!

.. _janice_gallery_remove_batch:

janice\_gallery\_remove\_batch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Remove a batch of templates from a gallery.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_gallery_remove_batch(JaniceGallery gallery,
                                                          JaniceTemplateIds ids);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+--------------------------+----------------------------------------------------------------------+
|  Name   |           Type           |                             Description                              |
+=========+==========================+======================================================================+
| gallery | :ref:`JaniceGallery`     | The gallery object to remove the templates from.                     |
+---------+--------------------------+----------------------------------------------------------------------+
| ids     | :ref:`JaniceTemplateIds` | The unique identifiers for the templates to remove from the gallery. |
+---------+--------------------------+----------------------------------------------------------------------+

.. _janice_gallery_prepare:

janice\_gallery\_prepare
~~~~~~~~~~~~~~~~~~~~~~~~

Prepare a gallery for search. Implementors can use this function as an
opportunity to streamline gallery objects to accelerate the search process. The
calling convention for this function is **NOT** specified by the API, this 
means that this function is not guaranteed to be called before 
:ref:`janice_search`. It also means that templates can be added to a gallery 
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

This function is :ref:`reentrant`.

Parameters 
^^^^^^^^^^

+---------+----------------------+-----------------------------+
|  Name   |         Type         |         Description         |
+=========+======================+=============================+
| gallery | :ref:`JaniceGallery` | A gallery object to prepare |
+---------+----------------------+-----------------------------+

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

.. _janice_serialize_gallery:

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

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |         Type         |                                                                                               Description                                                                                                |
+=========+======================+==========================================================================================================================================================================================================+
| gallery | :ref:`JaniceGallery` | A gallery object to serialize                                                                                                                                                                            |
+---------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| data    | :ref:`JaniceBuffer`  | An uninitialized buffer to hold the flattened data. The implementor allocate this object during the function call. The user is responsible for freeing this object by calling :ref:`janice_free_buffer`. |
+---------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| len     | size\_t\*            | The length of the flat buffer after it is allocated.                                                                                                                                                     |
+---------+----------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery; // Where gallery is a valid gallery created
                           // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_gallery(gallery, &buffer, &buffer_len);

.. _janice_deserialize_gallery:

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

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |           Type            |                                                                                         Description                                                                                          |
+=========+===========================+==============================================================================================================================================================================================+
| data    | const :ref:`JaniceBuffer` | A buffer containing data from a flattened gallery object.                                                                                                                                    |
+---------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| len     | size\_t                   | The length of the flat buffer.                                                                                                                                                               |
+---------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| gallery | :ref:`JaniceGallery`\*    | An uninitialized gallery object. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_gallery`. |
+---------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

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

.. _janice_read_gallery:

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

This function is :ref:`reentrant`.

Parameters 
^^^^^^^^^^

+----------+------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|   Name   |          Type          |                                                                                          Description                                                                                          |
+==========+========================+===============================================================================================================================================================================================+
| filename | const char\*           | The path to a file on disk                                                                                                                                                                    |
+----------+------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| gallery  | :ref:`JaniceGallery`\* | An uninitialized gallery object. The implementor should allocate this object during the function call. The user is responsible for freeing this object by calling :ref:`janice_free_gallery`. |
+----------+------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery = NULL;
    if (janice_read_gallery("example.gallery", &gallery) != JANICE_SUCCESS)
        // ERROR!

.. _janice_write_gallery:

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

This function is :ref:`reentrant`.

Parameters 
^^^^^^^^^^

+----------+----------------------+--------------------------------------+
|   Name   |         Type         |             Description              |
+==========+======================+======================================+
| gallery  | :ref:`JaniceGallery` | The gallery object to write to disk. |
+----------+----------------------+--------------------------------------+
| filename | const char\*         | The path to a file on disk           |
+----------+----------------------+--------------------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery; // Where gallery is a valid gallery created previously
    if (janice_write_gallery(gallery, "example.gallery") != JANICE_SUCCESS)
        // ERROR!

.. _janice_free_gallery:

janice\_free\_gallery 
~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceGalleryType` object.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);

Thread Safety 
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters 
^^^^^^^^^^

+---------+------------------------+---------------------------+
|  Name   |          Type          |        Description        |
+=========+========================+===========================+
| gallery | :ref:`JaniceGallery`\* | A gallery object to free. |
+---------+------------------------+---------------------------+

Example 
^^^^^^^

::

    JaniceGallery gallery; // Where gallery is a valid gallery object created previously
    if (janice_free_gallery(&gallery) != JANICE_SUCCESS)
        // ERROR!


.. _janice_clear_template_ids:

janice\_clear\_template\_ids
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a of :ref:`JaniceTemplateIds` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_clear_template_ids(JaniceTemplateIds* ids);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+----------------------------+----------------------------------+
| Name |            Type            |           Description            |
+======+============================+==================================+
| ids  | :ref:`JaniceTemplateIds`\* | A template ids objects to clear. |
+------+----------------------------+----------------------------------+

.. _janice_clear_template_ids_group:

janice\_clear\_template\_ids\_group
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceTemplateIdsGroup` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_clear_template_ids_group(JaniceTemplateIdsGroup* group);

Parameters
^^^^^^^^^^

+-------+---------------------------------+--------------------------------+
| Name  |              Type               |          Description           |
+=======+=================================+================================+
| group | :ref:`JaniceTemplateIdsGroup`\* | A template ids group to clear. |
+-------+---------------------------------+--------------------------------+
