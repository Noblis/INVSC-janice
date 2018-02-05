.. _enrollment:

Enrollment
==========

Overview
--------

This API defines feature extraction as the process of turning 1 or more
:ref:`detection` API objects that refer to the same object of interest into a
single representation. This representation is defined in the API using
the :ref:`JaniceTemplateType` object. In some cases (e.g. face recognition)
this model of [multiple detections] -> [single representation] contradicts the
current paradigm of [single detection] -> [single representation]. Implementors
are free to implement whatever paradigm they choose internally (i.e. a JanICE
template could be a simple list of single detection templates) provided
the :ref:`comparison` functions work appropriately.

.. _fte:

Failure To Enroll
~~~~~~~~~~~~~~~~~

For computer vision use cases, it is common to implement quality checks that
can cause a template to fail during enrollment if it is missing certain
characteristics. In this API templates should fail to enroll (FTE) quietly.
This means that successive operations using an FTE template should still work
without error. For example, calling :ref:`janice_verify` with an FTE template
and a successful template should still return a score, even if that score is a
predetermined constant value like **-FLOAT_MAX**. Users can query a template to
see if it failed to enroll using the :ref:`janice_template_is_fte` function and
may choose to manually discard it if they desire.

Structs
-------

.. _JaniceTemplateType:

JaniceTemplateType
~~~~~~~~~~~~~~~~~~

A struct that represents a template.

Typedefs
--------

.. _JaniceTemplate:

JaniceTemplate
~~~~~~~~~~~~~~

A pointer to a :ref:`JaniceTemplateType` object.

Signature
^^^^^^^^^

::

    typedef struct JaniceTemplateType* JaniceTemplate;

.. _JaniceTemplates:

JaniceTemplates
~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceTemplate` objects.

Fields
^^^^^^

+--------+-------------------------+------------------------------------+
|  Name  |          Type           |            Description             |
+========+=========================+====================================+
| tmpls  | :ref:`JaniceTemplate`\* | An array of template objects.      |
+--------+-------------------------+------------------------------------+
| length | size\_t                 | The number of elements in *tmpls*. |
+--------+-------------------------+------------------------------------+

.. _JaniceTemplatesGroup:

JaniceTemplatesGroup
~~~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceTemplates` objects.

Fields
^^^^^^

+--------+--------------------------+------------------------------------+
|  Name  |           Type           |            Description             |
+========+==========================+====================================+
| group  | :ref:`JaniceTemplates`\* | An array of templates objects.     |
+--------+--------------------------+------------------------------------+
| length | size\_t                  | The number of elements in *group*. |
+--------+--------------------------+------------------------------------+


Functions
---------

.. _janice_enroll_from_media:

janice\_enroll\_from\_media
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Detect and enroll templates from a single media file. Detection should respect
the provided minimum object size and detection policy. This function may
produce 0 or more templates, depending on the number of objects found in the
media.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_media(JaniceMediaIterator media,
                                                       JaniceContext context,
                                                       JaniceTemplates* tmpls,
                                                       JaniceTracks* tracks);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |            Type            |                                                                                                                                                                Description                                                                                                                                                                 |
+=========+============================+============================================================================================================================================================================================================================================================================================================================================+
| media   | :ref:`JaniceMediaIterator` | The media to detect and enroll templates from.                                                                                                                                                                                                                                                                                             |
+---------+----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context | :ref:`JaniceContext`       | A context object with relevant hyperparameters set.                                                                                                                                                                                                                                                                                        |
+---------+----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls   | :ref:`JaniceTemplates`\*   | A struct to hold the templates enrolled from the media. The internal members of this object should be allocated by the implementor during the call. The user is required to clear this object by calling :ref:`janice_clear_templates`                                                                                                     |
+---------+----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tracks  | :ref:`JaniceTracks`\*      | A struct to hold the detection information for each of the templates enrolled from the media. This object should have the same number of elements as *tmpls*. The internal members of this object should be allocated by the implementor during the call. The user is required to clear this object by calling :ref:`janice_clear_tracks`. |
+---------+----------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_media_batch:

janice\_enroll\_from\_media\_batch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Detect and enroll templates from a batch of media objects. Batch processing can
often be more efficient then serial processing of a collection of data,
particularly if a GPU or co-processor is being utilized.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_media_batch(JaniceMediaIterators media,
                                                             JaniceContext context,
                                                             JaniceTemplatesGroup* tmpls,
                                                             JaniceTracksGroup* tracks);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+-------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |             Type              |                                                                                                                                                                                                                                                                                                       Description                                                                                                                                                                                                                                                                                                       |
+=========+===============================+=========================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================+
| media   | :ref:`JaniceMediaIterators`   | An array of media iterators to enroll.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
+---------+-------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context | :ref:`JaniceContext`          | A context object with relevant hyperparameters set.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
+---------+-------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls   | :ref:`JaniceTemplatesGroup`\* | A list of lists of template objects. Each input media iterator can contain 0 or more possible templates. This output structure should mirror the input such that the sublist at index *i* should contain all of the templates enrolled from media iterator *i*. If no templates are enrolled from a particular media object an entry must still be present in the top-level output list and the sublist should have a length of 0. The implementor should allocate the internal members of this object during the call. The user is responsible for clearing the object by calling :ref:`janice_clear_templates_group`. |
+---------+-------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tracks  | :ref:`JaniceTracksGroup`\*    | A list of lists of track objects. The top level list should have the same number of elements as *tmpls* and sublist *i* should have the same number of elements as *tmpls* sublist i. Each track in the sublist should provide the location information for where the corresponding template was enrolled from. The implementor should allocate the internal members of this object during the call. The user is responsible for clearing the object by calling :ref:`janice_clear_tracks_group`.                                                                                                                       |
+---------+-------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_detections:

janice\_enroll\_from\_detections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a :ref:`JaniceTemplate` object from an array of detections.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_detections(JaniceDetections detections,
                                                            JaniceContext context,
                                                            JaniceTemplate* tmpl);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |          Type           |                                                                                          Description                                                                                           |
+============+=========================+================================================================================================================================================================================================+
| detections | :ref:`JaniceDetections` | An array of detection objects.                                                                                                                                                                 |
+------------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | :ref:`JaniceContext`    | A context object with relevant hyperparameters set.                                                                                                                                            |
+------------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpl       | :ref:`JaniceTemplate`\* | An uninitialized template object. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_template`. |
+------------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_detections_batch:

janice\_enroll\_from\_detections\_batch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a set of :ref:`JaniceTemplate` objects from an array of detections. Batch 
processing can often be more efficient then serial processing of a collection of 
data, particularly if a GPU or co-processor is being utilized.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_detections_batch(JaniceDetectionsGroup detections,
                                                                  JaniceContext context,
                                                                  JaniceTemplates* tmpls);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |             Type             |                                                                                                                                          Description                                                                                                                                          |
+============+==============================+===============================================================================================================================================================================================================================================================================================+
| detections | :ref:`JaniceDetectionsGroup` | A list of lists of detection objects. Multiple detections can be enrolled into a single template, for example if detections correspond to multiple views of the object of interest. Each sublist in this object should contain all detections that should be enrolled into a single template. |
+------------+------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | :ref:`JaniceContext`         | A context object with relevant hyperparameters set.                                                                                                                                                                                                                                           |
+------------+------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls      | :ref:`JaniceTemplates`\*     | A structure to hold the enrolled templates. This should have the same number of elements as *detections*. The implementor should allocate the internal members of this object during the call. The user is responsible for clearing the object by calling :ref:`janice_clear_templates`.      |
+------------+------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_template_is_fte:

janice\_template\_is\_fte
~~~~~~~~~~~~~~~~~~~~~~~~~

Query to see if a template has failed to enroll. See :ref:`fte` for additional information.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_template_is_fte(JaniceTemplate tmpl,
                                                     int* fte);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+-----------------------+-------------------------------------------------------------------------+
| Name |         Type          |                               Description                               |
+======+=======================+=========================================================================+
| tmpl | :ref:`JaniceTemplate` | The template object to query.                                           |
+------+-----------------------+-------------------------------------------------------------------------+
| fte  | int\*                 | FTE flag. If the template has not failed to enroll this should equal 0. |
+------+-----------------------+-------------------------------------------------------------------------+

.. _janice_template_get_attribute:

janice\_template\_get\_attribute
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Get a metadata value from a template using a key string. The valid set
of keys is determined by the implementation and must be included in
their delivered documentation. The possible return values for a valid
key are also implementation specific. Invalid keys should return an
error.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_template_get_attribute(JaniceTemplate tmpl,
                                                            const char* key,
                                                            JaniceAttribute* value);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+-------+--------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name  |           Type           |                                                                                                       Description                                                                                                        |
+=======+==========================+==========================================================================================================================================================================================================================+
| tmpl  | :ref:`JaniceTemplate`    | A template object to query the attribute from.                                                                                                                                                                           |
+-------+--------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| key   | const char\*             | A key to look up a specific attribute. Valid keys must be defined and documented by the implementor.                                                                                                                     |
+-------+--------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| value | :ref:`JaniceAttribute`\* | An uninitialized char\* to hold the value of the attribute. This object should be allocated by the implementor during the function call. The user is responsible for the object by calling :ref:`janice_free_attribute`. |
+-------+--------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_serialize_template:

janice\_serialize\_template
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Serialize a :ref:`JaniceTemplate` object to a flat buffer.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_serialize_template(JaniceTemplate tmpl,
                                                        JaniceBuffer* data,
                                                        size_t* len);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+-----------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name |         Type          |                                                                                                  Description                                                                                                  |
+======+=======================+===============================================================================================================================================================================================================+
| tmpl | :ref:`JaniceTemplate` | A template object to serialize                                                                                                                                                                                |
+------+-----------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| data | :ref:`JaniceBuffer`\* | An uninitialized buffer to hold the flattened data. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_buffer` |
+------+-----------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| len  | size\_t\*             | The length of the flat buffer.                                                                                                                                                                                |
+------+-----------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template created
                         // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_template(tmpl, &buffer, &buffer_len);

.. _janice_deserialize_template:

janice\_deserialize\_template
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Deserialize a :ref:`JaniceTemplate` object from a flat buffer.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_deserialize_template(const JaniceBuffer data,
                                                          size_t len,
                                                          JaniceTemplate* tmpl);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+---------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name |           Type            |                                                                                          Description                                                                                           |
+======+===========================+================================================================================================================================================================================================+
| data | const :ref:`JaniceBuffer` | A buffer containing data from a flattened template object.                                                                                                                                     |
+------+---------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| len  | size\_t                   | The length of the flat buffer.                                                                                                                                                                 |
+------+---------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpl | :ref:`JaniceTemplate`\*   | An uninitialized template object. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_template`. |
+------+---------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    JaniceBuffer buffer[buffer_len];

    FILE* file = fopen("serialized.template", "r");
    fread(buffer, 1, buffer_len, file);

    JaniceTemplate tmpl = NULL; // best practice to initialize to NULL
    janice_deserialize_template(buffer, buffer_len, tmpl);

    fclose(file);

.. _janice_read_template:

janice\_read\_template
~~~~~~~~~~~~~~~~~~~~~~

Read a template from a file on disk. This method is functionally
equivalent to the following-

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    JaniceBuffer buffer[buffer_len];

    FILE* file = fopen("serialized.template", "r");
    fread(buffer, 1, buffer_len, file);

    JaniceTemplate tmpl = nullptr;
    janice_deserialize_template(buffer, buffer_len, tmpl);

    fclose(file);

It is provided for memory efficiency and ease of use when reading from
disk.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                                   JaniceTemplate* tmpl);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+----------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|   Name   |          Type           |                                                                                          Description                                                                                           |
+==========+=========================+================================================================================================================================================================================================+
| filename | const char\*            | The path to a file on disk                                                                                                                                                                     |
+----------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpl     | :ref:`JaniceTemplate`\* | An uninitialized template object. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_template`. |
+----------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl = NULL;
    if (janice_read_template("example.template", &tmpl) != JANICE_SUCCESS)
        // ERROR!

.. _janice_write_template:

janice\_write\_template
~~~~~~~~~~~~~~~~~~~~~~~

Write a template to a file on disk. This method is functionally
equivalent to the following-

::

    JaniceTemplate tmpl; // Where tmpl is a valid template created
                         // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_template(tmpl, &buffer, &buffer_len);

    FILE* file = fopen("serialized.template", "w+");
    fwrite(buffer, 1, buffer_len, file);

    fclose(file);

It is provided for memory efficiency and ease of use when writing to
disk.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_write_template(JaniceTemplate tmpl,
                                                    const char* filename);

ThreadSafety
^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+----------+-----------------------+---------------------------------------+
|   Name   |         Type          |              Description              |
+==========+=======================+=======================================+
| tmpl     | :ref:`JaniceTemplate` | The template object to write to disk. |
+----------+-----------------------+---------------------------------------+
| filename | const char\*          | The path to a file on disk.           |
+----------+-----------------------+---------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template created
                         // previously
    if (janice_write_template(tmpl, "example.template") != JANICE_SUCCESS)
        // ERROR!

.. _janice_free_template:

janice\_free\_template
~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceTemplate` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+-----------------------+----------------------------+
| Name |         Type          |        Description         |
+======+=======================+============================+
| tmpl | :ref:`JaniceTemplate` | A template object to free. |
+------+-----------------------+----------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template object created previously
    if (janice_free_template(&tmpl) != JANICE_SUCCESS)
        // ERROR!

.. _janice_clear_templates:

janice\_clear\_templates
~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a of :ref:`JaniceTemplates` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_clear_templates(JaniceTemplates* templates);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+-------+--------------------------+-------------------------------+
| Name  |           Type           |          Description          |
+=======+==========================+===============================+
| tmpls | :ref:`JaniceTemplates`\* | A templates objects to clear. |
+-------+--------------------------+-------------------------------+

.. _janice_clear_templates_group:

janice\_clear\_templates\_group
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceTemplatesGroup` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_clear_templates_group(JaniceTemplatesGroup* group);

Parameters
^^^^^^^^^^

+-------+-------------------------------+-----------------------------+
| Name  |             Type              |         Description         |
+=======+===============================+=============================+
| group | :ref:`JaniceTemplatesGroup`\* | A templates group to clear. |
+-------+-------------------------------+-----------------------------+
