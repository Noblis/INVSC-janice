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

Enumerations
------------

.. _JaniceEnrollmentType:

JaniceEnrollmentType
~~~~~~~~~~~~~~~~~~~~

Often times, the templates produced by algorithms will require different
data for different use cases. The enrollment type indicates what the use
case for the created template will be, allowing implementors to
specialize their templates if they so desire. The use cases supported by
the API are:

+----------------------+-------------------------------------------------------+
| Type                 | Description                                           |
+======================+=======================================================+
| Janice11Reference    | The template will be used as a reference template for |
|                      | 1:1 verification.                                     |
+----------------------+-------------------------------------------------------+
| Janice11Verification | The template will be used for verification against a  |
|                      | reference template in 1:1 verification.               |
+----------------------+-------------------------------------------------------+
| Janice1NProbe        | The template will be used as a probe template in 1:N  |
|                      | search.                                               |
+----------------------+-------------------------------------------------------+
| Janice1NGallery      | The template will be enrolled into a gallery and      |
|                      | searched against in 1:N search.                       |
+----------------------+-------------------------------------------------------+
| JaniceCluster        | The template will be used for clustering.             |
+----------------------+-------------------------------------------------------+

Structs
-------

.. _JaniceTemplateType:

JaniceTemplateType 
~~~~~~~~~~~~~~~~~~

An opaque pointer to a struct that represents a template. 

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

.. _JaniceConstTemplate:

JaniceConstTemplate 
~~~~~~~~~~~~~~~~~~~

A pointer to a constant :ref:`JaniceTemplateType` object.

Signature 
^^^^^^^^^

::

    typedef const struct JaniceTemplateType* JaniceConstTemplate;

.. _JaniceTemplates:

JaniceTemplates 
~~~~~~~~~~~~~~~

An array of :ref:`JaniceTemplate` objects.

Signature
^^^^^^^^^

::

    typedef struct JaniceTemplate* JaniceTemplates;

.. _JaniceConstTemplates:

JaniceConstTemplates 
~~~~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceConstTemplate` objects.

Signature
^^^^^^^^^

::

    typedef struct JaniceConstTemplate* JaniceConstTemplates;

Functions
---------

.. _janice\_create\_template:

janice\_create\_template
~~~~~~~~~~~~~~~~~~~~~~~~

Create a :ref:`JaniceTemplate` object from an array of detections.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_create_template(JaniceConstDetections detections,
                                                     uint32_t num_detections,
                                                     JaniceEnrollmentType role,
                                                     JaniceTemplate* tmpl);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------------------+------------------------------+--------------------------------+
| Name              | Type                         | Description                    |
+===================+==============================+================================+
| detections        | :ref:`JaniceConstDetections` | An array of detection objects. | 
+-------------------+------------------------------+--------------------------------+
| num\_detections   | uint32\_t                    | The number of input detections.|
+-------------------+------------------------------+--------------------------------+
| role              | :ref:`JaniceEnrollmentType`  | The use case for the template  | 
+-------------------+------------------------------+--------------------------------+
| tmpl              | :ref:`JaniceTemplate` \*     | An uninitialized template      |
|                   |                              | object. The implementor should |
|                   |                              | allocate this object during the|
|                   |                              | function call. The user is     |
|                   |                              | responsible for freeing the    |
|                   |                              | object by calling              |
|                   |                              | :ref:`janice\_free\_template`. |
+-------------------+------------------------------+--------------------------------+

Example
^^^^^^^

::

    JaniceDetections detections; // Where detections is a valid array of valid
                                 // detection objects created previously
    const uint32_t num_detections = K; // Where K is the number of detections in
                                       // the detections array
    JaniceEnrollmentType role = Janice1NProbe; // This template will be used as a
                                               // probe in 1-N search
    JaniceTemplate tmpl = NULL; // Best practice to initialize to NULL

    if (janice_create_template(detections, num_detections, rolw, &tmpl) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_template\_get\_attribute:

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

    JANICE_EXPORT JaniceError janice_template_get_attribute(JaniceConstTemplate tmpl,
                                                            const char* attribute,
                                                            char** value);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-----------+----------------------------+------------------------------------+
| Name      | Type                       | Description                        |
+===========+============================+====================================+
| tmpl      | :ref:`JaniceConstTemplate` | A template object to query the     |
|           |                            | attribute from.                    |
+-----------+----------------------------+------------------------------------+
| attribute | const char\*               | The name of the attribute to query.|
+-----------+----------------------------+------------------------------------+
| value     | char\*\*                   | An uninitialized pointer to hold   |
|           |                            | the attribute value. The           |
|           |                            | implementor should allocate this   |
|           |                            | object during the function call.   |
|           |                            | The returned value must be a null  |
|           |                            | terminated string.                 |
+-----------+----------------------------+------------------------------------+

.. _janice\_serialize\_template:

janice\_serialize\_template 
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Serialize a :ref:`JaniceTemplate` object to a flat buffer.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_serialize_template(JaniceConstTemplate tmpl,
                                                        JaniceBuffer* data,
                                                        size_t* len);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+--------+----------------------------+----------------------------------------+
| Name   | Type                       | Description                            |
+========+============================+========================================+
| tmpl   | :ref:`JaniceConstTemplate` | A template object to serialize         |
+--------+----------------------------+----------------------------------------+
| data   | :ref:`JaniceBuffer` \*     | An uninitialized buffer to hold the    |
|        |                            | flattened data. The implementor should |
|        |                            | allocate this object during the        |
|        |                            | function call. The user is responsible |
|        |                            | for freeing the object by calling      |
|        |                            | :ref:`janice\_free\_buffer`            |
+--------+----------------------------+----------------------------------------+
| len    | size\_t\*                  | The length of the flat buffer.         |
+--------+----------------------------+----------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template created
                         // previously.

    JaniceBuffer buffer = NULL;
    size_t buffer_len;
    janice_serialize_template(tmpl, &buffer, &buffer_len);

.. _janice\_deserialize\_template:

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

This function is reentrant.

Parameters
^^^^^^^^^^

+-------+---------------------------+------------------------------------------+
| Name  | Type                      | Description                              |
+=======+===========================+==========================================+
| data  | const :ref:`JaniceBuffer` | A buffer containing data from a flattened|
|       |                           | template object.                         |
+-------+---------------------------+------------------------------------------+
| len   | size\_t                   | The length of the flat buffer.           |
+-------+---------------------------+------------------------------------------+
| tmpl  | :ref:`JaniceTemplate` \*  | An uninitialized template object. The    |
|       |                           | implementor should allocate this object  |
|       |                           | during the function call. The user is    |
|       |                           | responsible for freeing the object by    |
|       |                           | calling :ref:`janice\_free\_template`    |
+-------+---------------------------+------------------------------------------+

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

.. _janice\_read\_template:

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

This function is reentrant.

Parameters
^^^^^^^^^^

+------------+--------------------------+--------------------------------------+
| Name       | Type                     | Description                          |
+============+==========================+======================================+
| filename   | const char \*            | The path to a file on disk           |
+------------+--------------------------+--------------------------------------+
| tmpl       | :ref:`JaniceTemplate` \* | An uninitialized template object. The|
|            |                          | implementor should allocate this     |
|            |                          | object during the function call. The |
|            |                          | user is responsible for freeing the  |
|            |                          | object by calling                    |
|            |                          | :ref:`janice\_free\_template`        |
+------------+--------------------------+--------------------------------------+

Example 
^^^^^^^

::

    JaniceTemplate tmpl = NULL;
    if (janice_read_template("example.template", &tmpl) != JANICE_SUCCESS)
        // ERROR!

.. _janice\_write\_template:

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

    JANICE_EXPORT JaniceError janice_write_template(JaniceConstTemplate tmpl,
                                                    const char* filename);

ThreadSafety
^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+------------+----------------------------+------------------------------------+
| Name       | Type                       | Description                        |
+============+============================+====================================+
| tmpl       | :ref:`JaniceConstTemplate` | The template object to write to    |
|            |                            | disk.                              |
+------------+----------------------------+------------------------------------+
| filename   | const char\*               | The path to a file on disk.        |
+------------+----------------------------+------------------------------------+

Example 
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template created
                         // previously
    if (janice_write_template(tmpl, "example.template") != JANICE_SUCCESS)
        // ERROR!

.. _janice\_free\_template:

janice\_free\_template
~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceTemplate` object.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters
^^^^^^^^^^

+-------+-----------------------+----------------------------------------------+
| Name  | Type                  | Description                                  |
+=======+=======================+==============================================+
| tmpl  | :ref:`JaniceTemplate` | A template object to free. Best practice     |
|       |                       | dictates the pointer should be set to *NULL* |
|       |                       | after it is free.                            |
+-------+-----------------------+----------------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate tmpl; // Where tmpl is a valid template object created previously
    if (janice_free_template(&tmpl) != JANICE_SUCCESS)
        // ERROR!
