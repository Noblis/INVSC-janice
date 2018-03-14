.. _comparison:

Comparison / Search
=====================

Overview
--------

This API defines two possible types of comparisons, 1:1 and 1:N. These are
represented by the :ref:`janice_verify` and :ref:`janice_search` functions
respectively. The API quantifies the relationship between two templates as a
single number called a :ref:`similarity_score`.

Structs
-------

.. _JaniceSimilarity:

JaniceSimilarity
~~~~~~~~~~~~~~~~

A number representing the similarity between two templates. See :ref:`similarity_score` for more information.

Signature
^^^^^^^^^

::

    typedef double JaniceSimilarity

.. _JaniceSimilarities:

JaniceSimilarities
~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceSimilarity` objects.

Fields
^^^^^^

+--------------+---------------------------+-------------------------------------------------+
|     Name     |           Type            |                   Description                   |
+==============+===========================+=================================================+
| similarities | :ref:`JaniceSimilarity`\* | An array of similarity objects.                 |
+--------------+---------------------------+-------------------------------------------------+
| length       | size\_t                   | The number of elements in :code:`similarities`. |
+--------------+---------------------------+-------------------------------------------------+

.. _JaniceSimilaritiesGroup:

JaniceSimilaritiesGroup
~~~~~~~~~~~~~~~~~~~~~~~

A structure representing a list of :ref:`JaniceSimilarities` objects.

Fields
^^^^^^

+--------+-----------------------------+------------------------------------------+
|  Name  |            Type             |               Description                |
+========+=============================+==========================================+
| group  | :ref:`JaniceSimilarities`\* | An array of similarities objects.        |
+--------+-----------------------------+------------------------------------------+
| length | size\_t                     | The number of elements in :code:`group`. |
+--------+-----------------------------+------------------------------------------+

Functions
---------

.. _janice_verify:

janice\_verify
~~~~~~~~~~~~~~

Compare two templates with the difference expressed as a similarity
score.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_verify(JaniceConstTemplate reference,
                                            JaniceConstTemplate verification,
                                            JaniceSimilarity* similarity);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

.. _similarity_score:

Similarity Score
^^^^^^^^^^^^^^^^

This API expects that the comparison of two templates results in a
single value that quantifies the similarity between them. A similarity
score is constrained by the following requirements:

1. Higher scores indicate greater similarity
2. Scores can be asymmetric. This mean verify(a, b) does not necessarily equal verify(b, a)

Parameters
^^^^^^^^^^

+--------------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|     Name     |           Type            |                                                                                         Description                                                                                          |
+==============+===========================+==============================================================================================================================================================================================+
| reference    | :ref:`JaniceTemplate`     | A reference template. This template was created with the :code:`Janice11Reference` role.                                                                                                     |
+--------------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| verification | :ref:`JaniceTemplate`     | A verification template. This this template was created with the :code:`Janice11Verification` role.                                                                                          |
+--------------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| similarity   | :ref:`JaniceSimilarity`\* | A similarity score. See :ref:`similarity_score`. Memory for this object should be managed by the user. The implementation should assume this points to a valid object that it can overwrite. |
+--------------+---------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate reference; // Where reference is a valid template object created
                              // previously
    JaniceTemplate verification; // Where verification is a valid template object
                                 // created previously
    JaniceSimilarity similarity;
    if (janice_verify(reference, verification, &similarity) != JANICE_SUCCESS)
        // ERROR!

.. _janice_verify_batch:

janice\_verify\_batch
~~~~~~~~~~~~~~~~~~~~~

Compute a batch of reference templates with a batch of verification templates. 
The :code:`ith` in the reference batch is compared with the :code:`ith` template in the 
verification batch. Batch processing can often be more efficient than serial 
processing, particularly if a GPU or co-processor is being utilized.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_verify_batch(JaniceTemplates references,
                                                  JaniceTemplates verifications,
                                                  JaniceSimilarities* similarities);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------------+-----------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|     Name      |            Type             |                                                                                                                                                                      Description                                                                                                                                                                      |
+===============+=============================+=======================================================================================================================================================================================================================================================================================================================================================+
| references    | :ref:`JaniceTemplates`      | An array of reference templates. Each template was created with the :code:`Janice11Reference` role.                                                                                                                                                                                                                                                   |
+---------------+-----------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| verifications | :ref:`JaniceTemplates`      | An array of verification templates. Each template was created with the :code:`Janice11Verification` role. The number of elements in :code:`verifications` must equal the number of elements in :code:`references`.                                                                                                                                    |
+---------------+-----------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| similarities  | :ref:`JaniceSimilarities`\* | A struct to hold the output similarity scores. There must be the same number of similarity scores output as there are :code:`references` and :code:`verifications`. The implementor should allocate the internal members of this object during the call. The user is responsible for clearing the object by calling :ref:`janice_clear_similarities`. |
+---------------+-----------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_search:

janice\_search
~~~~~~~~~~~~~~

Compute 1-N search results between a query template object and a target gallery 
object. When running searches, users will often only want the top N results, or
will only want results above a predefined threshold. This function must respect
the :code:`threshold` and :code:`max_returns` fields of a :ref:`JaniceContext` object to
facilitate these use cases. Implementors must always respect the passed threshold
(i.e. a score below the given threshold should never be returned). If users would
not like to specify a threshold they can set the member to :code:`-DOUBLE_MAX`. If
the :code:`max_returns` member is non-zero implementors should respect both the threshold
and the number of desired returns (i.e. return the top K scores above the given
threshold). Users who would like to see all valid returns should set :code:`max_returns`
to 0.

This function allocates two structures with the same number of elements.
:code:`similarities` is a :ref:`JaniceSimilarities` object with an arra of 
:ref:`similarity_score`, sorted in descending order. The second is a
:ref:`JaniceTemplateIds` where the :code:`ith` template id gives the unique
identifier for the gallery template that produces the :code:`ith` similarity
score when compared with the probe. 

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                            JaniceConstGallery gallery,
                                            JaniceContext context,
                                            JaniceSimilarities* similarities,
                                            JaniceTemplateIds* ids);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+--------------+------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|     Name     |             Type             |                                                                                                                                                                       Description                                                                                                                                                                        |
+==============+==============================+==========================================================================================================================================================================================================================================================================================================================================================+
| probe        | :ref:`JaniceTemplate`        | A query template. The template was created with the :code:`Janice1NProbe` role.                                                                                                                                                                                                                                                                          |
+--------------+------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| gallery      | :ref:`JaniceGallery`         | A gallery object to search against.                                                                                                                                                                                                                                                                                                                      |
+--------------+------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context      | const :ref:`JaniceContext`\* | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                 |
+--------------+------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| similarities | :ref:`JaniceSimilarities`\*  | A structure to hold the output similarity scores, sorted in descending order. This structure should have the same number of elements as :code:`ids`. The implementor should allocate the internal members of this object during the call. The user is responsible for clearing the object by calling :ref:`janice_clear_similarities`.                   |
+--------------+------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ids          | :ref:`JaniceTemplateIds`\*   | A structure to hold the gallery template ids associated with the :code:`similarities`. This structure should have the same number of elements as :code:`similarities`. The implementor should allocate the internal members of this object during the call. The user is responsible for clearing the object by calling :ref:`janice_clear_template_ids`. |
+--------------+------------------------------+----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Example
^^^^^^^

::

    JaniceTemplate probe;  // Where probe is a valid template object created
                           // previously
    JaniceGallery gallery; // Where gallery is a valid gallery object created
                           // previously
    
    JaniceContext context = nullptr;
    if (janice_create_context(JaniceDetectAll, // detection policy, this shouldn't impact search
                              0, // min_object_size, this shouldn't impact search
                              Janice1NProbe, // enrollment type, this shouldn't impact search
                              0.7, // threshold, get all matches scoring above 0.7
                              50, // max_returns, get the top 50 matches scoring above the set threshold
                              0, // hint, this shouldn't impact search
                              &context) != JANICE_SUCCESS)
        // ERROR!

    JaniceSimilarities similarities;
    JaniceTemplateIds ids;

    // Run search
    if (janice_search(probe, gallery, context, &similarities, &ids) != JANICE_SUCCESS)
        // ERROR!

.. _janice_search_batch:

janice\_search\_batch
~~~~~~~~~~~~~~~~~~~~~

Compute 1-N search results between a batch of probe templates and a single
gallery. Given :code:`N` probe templates in a batch, this function should return
a single :ref:`JaniceSimilaritiesGroup` with N sublists and a single
:ref:`JaniceTemplateIdsGroup` with N sublists. Each sublist must conform to
the behavior defined in :ref:`janice\_search`. Batch processing can often be 
more efficient than serial processing, particularly if a GPU or co-processor 
is being utilized.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_search_batch(JaniceTemplates probes,
                                                  JaniceGallery gallery,
                                                  JaniceContext context,
                                                  JaniceSimilaritiesGroup* similarities,
                                                  JaniceTemplateIdsGroup* ids);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+--------------+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|     Name     |               Type               |                                                                                                                                                                                          Description                                                                                                                                                                                           |
+==============+==================================+================================================================================================================================================================================================================================================================================================================================================================================================+
| probes       | :ref:`JaniceTemplates`           | An array of probe templates to search with. Each template was created with the :code:`Janice1NProbe` role.                                                                                                                                                                                                                                                                                     |
+--------------+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| gallery      | :ref:`JaniceGallery`             | The gallery to search against.                                                                                                                                                                                                                                                                                                                                                                 |
+--------------+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| context      | const :ref:`JaniceContext`\*     | A context object with relevant hyperparameters set. Memory for the object should be managed by the user. The implementation should assume this points to a valid object.                                                                                                                                                                                                                       |
+--------------+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| similarities | :ref:`JaniceSimilaritiesGroup`\* | A structure to hold the output similarities. Given :code:`N` probes, there should be :code:`N` sublists in the output, where the :code:`ith` sublist gives the similarity scores of the :code:`ith` probe. Internal struct members should be initialized by the implementor as part of the call. The user is required to clear the struct by calling :ref:`janice_clear_similarities_group`.   |
+--------------+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ids          | :ref:`JaniceTemplateIdsGroup`\*  | A structure to hold the output template ids. Given :code:`N` probes, there should be :code`N` sublists in the output, where the :code:`ith` sublist gives the gallery template ids of the :code:`ith` probe. Internal struct members should be initialized by the implementor as part of the call. The user is required to clear the struct by calling :ref:`janice_clear_template_ids_group`. |
+--------------+----------------------------------+------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

.. _janice_clear_similarities:

janice\_clear\_similarities
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceSimilarities` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_clear_similarities(JaniceSimilarities* similarities);

Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+--------------+-----------------------------+----------------------------------+
|     Name     |            Type             |           Description            |
+==============+=============================+==================================+
| similarities | :ref:`JaniceSimilarities`\* | An similarities object to clear. |
+--------------+-----------------------------+----------------------------------+

.. _janice_clear_similarities_group:

janice\_clear\_similarities\_group
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceSimilaritiesGroup` object.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_clear_similarities_group(JaniceSimilaritiesGroup* group);

Parameters
^^^^^^^^^^

+-------+----------------------------------+--------------------------------+
| Name  |               Type               |          Description           |
+=======+==================================+================================+
| group | :ref:`JaniceSimilaritiesGroup`\* | A similarities group to clear. |
+-------+----------------------------------+--------------------------------+
