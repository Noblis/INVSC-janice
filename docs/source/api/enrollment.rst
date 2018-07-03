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
predetermined constant value like :code:`-FLOAT_MAX`. Users can query a template to
see if it failed to enroll using the :ref:`janice_template_is_fte` function and
may choose to manually discard it if they desire.

Enumerations
------------

.. _JaniceFeatureVectorType:

JaniceFeatureVectorType
~~~~~~~~~~~~~~~~~~~~~~~

The data type of the feature vector returned by
:ref:`janice_template_get_feature_vector`. Supported data types are:

+--------------+-------------------------------------------------------------------------+
|  Data Type   |                               Description                               |
+==============+=========================================================================+
| JaniceInt8   | 8 bit signed integer. The associated *C* type is :code:`int8_t`         |
+--------------+-------------------------------------------------------------------------+
| JaniceInt16  | 16 bit signed integer. The associated *C* type is :code:`int16_t`       |
+--------------+-------------------------------------------------------------------------+
| JaniceInt32  | 32 bit signed integer. The associated *C* type is :code:`int32_t`       |
+--------------+-------------------------------------------------------------------------+
| JaniceInt64  | 64 bit signed integer. The associated *C* type is :code:`int64_t`       |
+--------------+-------------------------------------------------------------------------+
| JaniceUInt8  | 8 bit unsigned integer. The associated *C* type is :code:`uint8_t`      |
+--------------+-------------------------------------------------------------------------+
| JaniceUInt16 | 16 bit unsigned integer. The associated *C* type is :code:`uint16_t`    |
+--------------+-------------------------------------------------------------------------+
| JaniceUInt32 | 32 bit unsigned integer. The associated *C* type is :code:`uint32_t`    |
+--------------+-------------------------------------------------------------------------+
| JaniceUInt64 | 64 bit unsigned integer. The associated *C* type is :code:`uint64_t`    |
+--------------+-------------------------------------------------------------------------+
| JaniceFloat  | 32 bit floating point number. The associated *C* type is :code:`float`  |
+--------------+-------------------------------------------------------------------------+
| JaniceDouble | 64 bit floating point number. The associated *C* type is :code:`double` |
+--------------+-------------------------------------------------------------------------+

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

+--------+-------------------------+------------------------------------------+
|  Name  |          Type           |               Description                |
+========+=========================+==========================================+
| tmpls  | :ref:`JaniceTemplate`\* | An array of template objects.            |
+--------+-------------------------+------------------------------------------+
| length | size\_t                 | The number of elements in :code:`tmpls`. |
+--------+-------------------------+------------------------------------------+

.. _JaniceTemplatesGroup:

JaniceTemplatesGroup
~~~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceTemplates` objects.

Fields
^^^^^^

+--------+--------------------------+------------------------------------------+
|  Name  |           Type           |               Description                |
+========+==========================+==========================================+
| group  | :ref:`JaniceTemplates`\* | An array of templates objects.           |
+--------+--------------------------+------------------------------------------+
| length | size\_t                  | The number of elements in :code:`group`. |
+--------+--------------------------+------------------------------------------+

Callbacks
---------

.. _JaniceEnrollMediaCallback:

JaniceEnrollMediaCallback
~~~~~~~~~~~~~~~~~~~~~~~~~

A function prototype to process :ref:`JaniceTemplate` and :ref:`JaniceDetection`
objects as they are found.

Signature
^^^^^^^^^

::

    JaniceError (*JaniceEnrollMediaCallback)(const JaniceTemplate*, const JaniceDetection*, size_t, void*);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`thread_unsafe`.

Parameters
^^^^^^^^^^

+-----------+--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------+
|   Name    |              Type              |                                                                    Description                                                                     |
+===========+================================+====================================================================================================================================================+
| tmpl      | const :ref:`JaniceTemplate`\*  | A template object enrolled during the function                                                                                                     |
+-----------+--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------+
| detection | const :ref:`JaniceDetection`\* | A detection object containing the location of the enrolled template                                                                                |
+-----------+--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------+
| index     | size_t                         | The index of the media iterator the template was enrolled from.                                                                                    |
+-----------+--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------+
| user_data | void\*                         | User defined data that may assist in the processing of template. It is passed directly from the :code:`\*_with_callback` function to the callback. |
+-----------+--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------+

.. _JaniceEnrollDetectionsCallback:

JaniceEnrollDetectionsCallback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A function prototype to process :ref:`JaniceTemplate` objects as they are
created.

Signature
^^^^^^^^^

::

    JaniceError (*JaniceEnrollDetectionsCallback)(const JaniceTemplate*, size_t, void*);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`thread_unsafe`.

Parameters
^^^^^^^^^^

+-----------+-------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
|   Name    |             Type              |                                                                       Description                                                                       |
+===========+===============================+=========================================================================================================================================================+
| tmpl      | const :ref:`JaniceTemplate`\* | A template object enrolled during the function                                                                                                          |
+-----------+-------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| index     | size_t                        | The index of the media iterator group the template was enrolled from.                                                                                   |
+-----------+-------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+
| user_data | void\*                        | User defined data that may assist in the processing of the detection. It is passed directly from the :code:`\*_with_callback` function to the callback. |
+-----------+-------------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------+


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

    JANICE_EXPORT JaniceError janice_enroll_from_media(const JaniceMediaIterator* media,
                                                       const JaniceContext* context,
                                                       JaniceTemplates* tmpls,
                                                       JaniceDetections* detections);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |                Type                |                                                                                                                                                                                                         Description                                                                                                                                                                                                         |
+============+====================================+=============================================================================================================================================================================================================================================================================================================================================================================================================================+
| media      | const :ref:`JaniceMediaIterator`\* | The media to detect and enroll templates from. After the function call, the iterator will exist in an undefined state. A user should call :ref:`reset` on the iterator before reusing it.                                                                                                                                                                                                                                   |
+------------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | const :ref:`JaniceContext`\*       | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                                                                                    |
+------------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls      | :ref:`JaniceTemplates`\*           | A struct to hold the templates enrolled from the media. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is required to clear this object by calling :ref:`janice_clear_templates`                                                                                                               |
+------------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| detections | :ref:`JaniceDetections`\*          | A struct to hold the detection information for each of the templates enrolled from the media. This object should have the same number of elements as :code:`tmpls`. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is required to clear this object by calling :ref:`janice_clear_detections`. |
+------------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_media_with_callback:

janice\_enroll\_from\_media\_with\_callback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Run detection with a callback, which surfaces detections as they are made for
processing. The callback accepts user data as input. It is important to remember
that :code:`JaniceMediaIterator` may be stateful and should not be part of the
callback. The implementor is not responsible for ensuring that the state of
:code:`media` is not changed by the user during this call. The provided callback
may return an error. If an error is returned by the callback, the implementation
should abort and return that error as well. This function will always pass
:code:`0` to the index parameter of the callback.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_media_with_callback(const JaniceMediaIterator* media,
                                                                     const JaniceContext* context,
                                                                     JaniceEnrollMediaCallback callback,
                                                                     void* user_data);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+-----------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|   Name    |                Type                |                                                                                          Description                                                                                          |
+===========+====================================+===============================================================================================================================================================================================+
| media     | const :ref:`JaniceMediaIterator`\* | A media object to run detection and enrollment on. After the function call, the iterator will exist in an undefined state. A user should call :ref:`reset` on the iterator before reusing it. |
+-----------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context   | const :ref:`JaniceContext`\*       | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                      |
+-----------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| callback  | :ref:`JaniceEnrollMediaCallback`   | A pointer to a user defined callback function.                                                                                                                                                |
+-----------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| user_data | void\*                             | A pointer to user defined data. This is passed to the callback function on each invocation.                                                                                                   |
+-----------+------------------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_media_batch:

janice\_enroll\_from\_media\_batch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Detect and enroll templates from a batch of media objects. Batch processing can
often be more efficient then serial processing of a collection of data,
particularly if a GPU or co-processor is being utilized. This function reports
per-image error codes. Depending on the batch policy given, it will return one
of :code:`JANICE_SUCCESS` if no errors occured, or
:code:`JANICE_BATCH_ABORTED_EARLY` or :code:`JANICE_BATCH_FINISHED_WITH_ERRORS`
if errors occured within the batch. In either case, any computation marked
:code:`JANICE_SUCCESS` in the output should be considered valid output.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_media_batch(const JaniceMediaIterators* media,
                                                             const JaniceContext* context,
                                                             JaniceTemplatesGroup* tmpls,
                                                             JaniceDetectionsGroup* detections,
                                                             JaniceErrors* errors);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |                Type                 |                                                                                                                                                                                                                                                                                                                                                    Description                                                                                                                                                                                                                                                                                                                                                    |
+============+=====================================+===================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================+
| media      | const :ref:`JaniceMediaIterators`\* | An array of media iterators to enroll. After the function call, each iterator in the array will exist in an undefined state. A user should call :ref:`reset` on each iterator before reusing them.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
+------------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | const :ref:`JaniceContext`\*        | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
+------------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls      | :ref:`JaniceTemplatesGroup`\*       | A list of lists of template objects. Each input media iterator can contain 0 or more possible templates. This output structure should mirror the input such that the sublist at index :code:`i` should contain all of the templates enrolled from media iterator :code:`i`. If no templates are enrolled from a particular media object an entry must still be present in the top-level output list and the sublist should have a length of 0. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_templates_group`. |
+------------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| detections | :ref:`JaniceDetectionsGroup`\*      | A list of lists of track objects. The top level list should have the same number of elements as :code:`tmpls` and sublist :code:`i` should have the same number of elements as :code:`tmpls` sublist i. Each track in the sublist should provide the location information for where the corresponding template was enrolled from. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_detections_group`.                                                                                                             |
+------------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| errors     | :ref:`JaniceErrors`\*               | A struct to hold per-image error codes. There must be the same number of errors as there are :code:`media` unless the call aborted early, in which case there can be less. The :code:`ith` error code should give the status of detection on the :code:`ith` piece of media. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_errors`.                                                                                                                                                                            |
+------------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_media_batch_with_callback:

janice\_enroll\_from\_media\_batch\_with\_callback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Run batched detection and enrollment with a callback, which surfaces templates
and associated detections they are made for processing. Batch processing can
often be more efficient than serial processing, particularly if a GPU or
co-processor is being utilized. The callback accepts user data as input. It is
important to remember that :code:`JaniceMediaIterator` may be stateful and
should not be part of the callback. The implementor is not responsible for
ensuring that the state of :code:`media` is not changed by the user during this
call. The provided callback may return an error. If an error is returned by the
callback, it should be stored at the corresponding offset in :code:`errors` and
the implementation should stop processing that media. As a special case, the
callback may return :code:`JANICE_CALLBACK_EXIT_IMMEDIATELY`. In this case, the
parent function should set the corresponding error appropriately and then return
without finishing.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_media_batch_with_callback(const JaniceMediaIterators* media,
                                                                           const JaniceContext* context,
                                                                           JaniceEnrollMediaCallback callback,
                                                                           void* user_data,
                                                                           JaniceErrors* errors);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+-----------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|   Name    |                Type                 |                                                                                                                                                                                                                                                                      Description                                                                                                                                                                                                                                                                      |
+===========+=====================================+=======================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================+
| media     | const :ref:`JaniceMediaIterators`\* | A list of media objects to run detection and enrollment on. After the function call, each iterator will exist in an undefined state. A user should call :ref:`reset` on each iterator before reusing it.                                                                                                                                                                                                                                                                                                                                              |
+-----------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context   | const :ref:`JaniceContext`\*        | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                                                                                                                                                                                                              |
+-----------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| callback  | :ref:`JaniceEnrollMediaCallback`    | A pointer to a user defined callback function.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
+-----------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| user_data | void\*                              | A pointer to user defined data. This is passed to the callback function on each invocation.                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
+-----------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| errors    | :ref:`JaniceErrors`\*               | A struct to hold per-image error codes. There must be the same number of errors as there are :code:`media` unless the call aborted early, in which case there can be less. The :code:`ith` error code should give the status of detection and enrollment on the :code:`ith` piece of media. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_errors`. |
+-----------+-------------------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_detections:

janice\_enroll\_from\_detections
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a :ref:`JaniceTemplate` object from an array of detections.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_detections(const JaniceMediaIterators* media,
                                                            const JaniceDetections* detections,
                                                            const JaniceContext* context,
                                                            JaniceTemplate* tmpl);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+-------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |                Type                 |                                                                                                                                     Description                                                                                                                                      |
+============+=====================================+======================================================================================================================================================================================================================================================================================+
| media      | const :ref:`JaniceMediaIterators`\* | An array of media objects. The array should have the same length as :code:`detections`. After the function call, each iterator in the array will exist in an undefined state. A user should call :ref:`reset` on each iterator before reusing them.                                  |
+------------+-------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| detections | const :ref:`JaniceDetections`\*     | An array of detection objects. Each detection in the array should represent a unique sighting of the same object. The :code:`ith` detection in the array represents a sighting in the :code:`ith` element in :code:`media`. This array should have the same length as :code:`media`. |
+------------+-------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | const :ref:`JaniceContext`\*        | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                             |
+------------+-------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpl       | :ref:`JaniceTemplate`\*             | An uninitialized template object. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_template`.                                                                                       |
+------------+-------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_detections_batch:

janice\_enroll\_from\_detections\_batch
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a set of :ref:`JaniceTemplate` objects from an array of detections. Batch
processing can often be more efficient then serial processing of a collection of
data, particularly if a GPU or co-processor is being utilized. This function
reports per media error codes. Depending on the batch policy given, it
will return one of :code:`JANICE_SUCCESS` if no errors occured, or
:code:`JANICE_BATCH_ABORTED_EARLY` or :code:`JANICE_BATCH_FINISHED_WITH_ERRORS`
if errors occured within the batch. In either case, any computation marked
:code:`JANICE_SUCCESS` in the output should be considered valid output.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_detections_batch(const JaniceMediaIteratorsGroup* media,
                                                                  const JaniceDetectionsGroup* detections,
                                                                  const JaniceContext* context,
                                                                  JaniceTemplates* tmpls,
                                                                  JaniceErrors* errors);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |                   Type                   |                                                                                                                                                                                                                                                                     Description                                                                                                                                                                                                                                                                      |
+============+==========================================+======================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================+
| media      | const :ref:`JaniceMediaIteratorsGroup`\* | A list of lists of media objects. Each sublist in this object should contain all of the media corresponding to unique sightings of an object of interest. The :code:`ith` sublist should  be the same length at the :code:`ith` sublist of :code:`detections`. The number of sublists should match the number of sublists in :code:`detections`. After the function call, each iterator in each sublist of the group will exist in an undefined state. A user should call :ref:`reset` on each iterator before reusing them.                         |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| detections | const :ref:`JaniceDetectionsGroup`\*     | A list of lists of detection objects. Multiple detections can be enrolled into a single template, for example if detections correspond to multiple views of the object of interest. Each sublist in this object should contain all detections that should be enrolled into a single template. The :code:`jth` element in the :code:`ith` sublist should represent a sighting in the :code:`jth` element in the :code:`ith` sublist of :code:`media`.                                                                                                 |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | const :ref:`JaniceContext`\*             | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                                                                                                                                                                                                             |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpls      | :ref:`JaniceTemplates`\*                 | A structure to hold the enrolled templates. This should have the same number of elements as :code:`detections`. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_templates`.                                                                                                                                                                         |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| errors     | :ref:`JaniceErrors`\*                    | A struct to hold per media group error codes. There must be the same number of errors as there are :code:`media` groups unless the call aborted early, in which case there can be less. The :code:`ith` error code should give the status of enrollment on the :code:`ith` group of media. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_errors`. |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_enroll_from_detections_batch_with_callback:

janice\_enroll\_from\_detections\_batch\_with\_callback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create templates from a batch of sightings. Batch processing can often be more
efficient than serial processing, particularly if a GPU or co-processor is being
utilized. The callback accepts user data as input. It is important to remember
that :code:`JaniceMediaIterator` may be stateful and should not be part of the
callback. The implementor is not responsible for ensuring that the state of
:code:`media` is not changed by the user during this call. The provided callback
may return an error. If an error is returned by the callback, it should be
stored at the corresponding offset in :code:`errors` and the implementation
should stop processing that media. As a special case, the callback may return
:code:`JANICE_CALLBACK_EXIT_IMMEDIATELY`. In this case, the parent function
should set the corresponding error appropriately and then return without
finishing.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_enroll_from_detections_batch_with_callback(const JaniceMediaIteratorsGroup* media, 
                                                                                const JaniceDetectionsGroup* detections,
                                                                                const JaniceContext* context,
                                                                                JaniceEnrollDetectionsCallback callback,
                                                                                void* user_data,
                                                                                JaniceErrors* errors);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|    Name    |                   Type                   |                                                                                                                                                                                                                                                                     Description                                                                                                                                                                                                                                                                      |
+============+==========================================+======================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================+
| media      | const :ref:`JaniceMediaIteratorsGroup`\* | A list of lists of media objects. Each sublist in this object should contain all of the media corresponding to unique sightings of an object of interest. The :code:`ith` sublist should  be the same length at the :code:`ith` sublist of :code:`detections`. The number of sublists should match the number of sublists in :code:`detections`. After the function call, each iterator in each sublist of the group will exist in an undefined state. A user should call :ref:`reset` on each iterator before reusing them.                         |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| detections | const :ref:`JaniceDetectionsGroup`\*     | A list of lists of detection objects. Multiple detections can be enrolled into a single template, for example if detections correspond to multiple views of the object of interest. Each sublist in this object should contain all detections that should be enrolled into a single template. The :code:`jth` element in the :code:`ith` sublist should represent a sighting in the :code:`jth` element in the :code:`ith` sublist of :code:`media`.                                                                                                 |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context    | const :ref:`JaniceContext`\*             | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                                                                                                                                                                                                             |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| callback   | :ref:`JaniceEnrollDetectionsCallback`    | A pointer to a user defined callback function.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| user_data  | void\*                                   | A pointer to user defined data. This is passed to the callback function on each invocation.                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| errors     | :ref:`JaniceErrors`\*                    | A struct to hold per media group error codes. There must be the same number of errors as there are :code:`media` groups unless the call aborted early, in which case there can be less. The :code:`ith` error code should give the status of enrollment on the :code:`ith` group of media. The user is responsible for allocating memory for the struct before the function call. The implementor is responsbile for allocating and filling internal members. The user is responsible for clearing the object by calling :ref:`janice_clear_errors`. |
+------------+------------------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_template_is_fte:

janice\_template\_is\_fte
~~~~~~~~~~~~~~~~~~~~~~~~~

Query to see if a template has failed to enroll. See :ref:`fte` for additional information.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_template_is_fte(const JaniceTemplate tmpl,
                                                     int* fte);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name |            Type             |                                                                                         Description                                                                                          |
+======+=============================+==============================================================================================================================================================================================+
| tmpl | const :ref:`JaniceTemplate` | The template object to query.                                                                                                                                                                |
+------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| fte  | int\*                       | FTE flag. If the template has not failed to enroll this should equal 0. Memory for the object should be managed by the user. The implementation should assume this points to a valid object. |
+------+-----------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

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

    JANICE_EXPORT JaniceError janice_template_get_attribute(const JaniceTemplate tmpl,
                                                            const char* key,
                                                            char** value);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+-------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name  |            Type             |                                                                                                                          Description                                                                                                                          |
+=======+=============================+===============================================================================================================================================================================================================================================================+
| tmpl  | const :ref:`JaniceTemplate` | A template object to query the attribute from.                                                                                                                                                                                                                |
+-------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| key   | const char\*                | A null-terminated key to look up a specific attribute. Valid keys must be defined and documented by the implementor.                                                                                                                                          |
+-------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| value | char\*\*                    | An uninitialized char\* to hold the value of the attribute. This object should be allocated by the implementor during the function call. This object must be null-terminated. The user is responsible for the object by calling :ref:`janice_free_attribute`. |
+-------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_template_get_feature_vector:

janice\_template\_get\_feature\_vector
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Extract a feature vector from a template. The requirements of the feature vector are still being defined.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_template_get_feature_vector(const JaniceTemplate tmpl,
                                                                 const JaniceFeatureVectorType feature_vector_type,
                                                                 void** feature_vector,
                                                                 size_t* length);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------------------+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|        Name         |                 Type                 |                                                                            Description                                                                             |
+=====================+======================================+====================================================================================================================================================================+
| tmpl                | const :ref:`JaniceTemplate`          | A template object to query the feature vector from.                                                                                                                |
+---------------------+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| feature_vector_type | const :ref:`JaniceFeatureVectorType` | The data type of the returned feature vector. It should be possible to interpret :code:`feature_vector` as a :code:`size` length array of the feature vector type. |
+---------------------+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| feature_vector      | void\*\*                             | A one-dimensional array containing the feature vector data. The user is responsible for the object by calling :ref:`janice_free_feature_vector`.                   |
+---------------------+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| size                | size\_t\*                            | The length of :code:`feature_vector`.                                                                                                                              |
+---------------------+--------------------------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_serialize_template:

janice\_serialize\_template
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Serialize a :ref:`JaniceTemplate` object to a flat buffer.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_serialize_template(const JaniceTemplate tmpl,
                                                        uint8_t** data,
                                                        size_t* len);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name |            Type             |                                                                                                  Description                                                                                                  |
+======+=============================+===============================================================================================================================================================================================================+
| tmpl | const :ref:`JaniceTemplate` | A template object to serialize                                                                                                                                                                                |
+------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| data | uint8_t\*\*                 | An uninitialized buffer to hold the flattened data. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_buffer` |
+------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| len  | size\_t\*                   | The length of the flat buffer. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                           |
+------+-----------------------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

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

    JANICE_EXPORT JaniceError janice_deserialize_template(const uint8_t* data,
                                                          size_t len,
                                                          JaniceTemplate* tmpl);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Name |          Type           |                                                                                          Description                                                                                           |
+======+=========================+================================================================================================================================================================================================+
| data | const uint8\_t\*        | A buffer containing data from a flattened template object.                                                                                                                                     |
+------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| len  | size\_t                 | The length of the flat buffer.                                                                                                                                                                 |
+------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| tmpl | :ref:`JaniceTemplate`\* | An uninitialized template object. The implementor should allocate this object during the function call. The user is responsible for freeing the object by calling :ref:`janice_free_template`. |
+------+-------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    const size_t buffer_len = K; // Where K is the known length of the buffer
    uint8_t buffer[buffer_len];

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
    uint8_t buffer[buffer_len];

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

Free any memory associated with a :ref:`JaniceTemplates` object.

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

.. _janice_free_feature_vector:

janice\_free\_feature\_vector
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free a feature vector returned by :ref:`janice_template_get_feature_vector`

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_feature_vector(void** feature_vector);

Parameters
^^^^^^^^^^

+----------------+----------+---------------------------+
|      Name      |   Type   |        Description        |
+================+==========+===========================+
| feature_vector | void\*\* | A feature vector to free. |
+----------------+----------+---------------------------+
