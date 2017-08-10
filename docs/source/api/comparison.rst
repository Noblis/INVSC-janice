.. _comparison:

Comparison
==========

Overview
--------

This API defines two possible types of comparisons, 1:1 and 1:N. These are 
represented by the :ref:`janice\_verify` and :ref:`janice\_search` functions
respectively. The API quantifies the relationship between two templates as a 
single number called a :ref:`comparison\_sim\_score`.

Typedefs
--------

.. _JaniceSimilarity:

JaniceSimilarity 
~~~~~~~~~~~~~~~~

A number representing the similarity between two templates. See
[functions.md#JaniceVerifySimilarityScore] for more information.

Signature 
^^^^^^^^^

::

    typedef double JaniceSimilarity

.. _JaniceSimilarities:

JaniceSimilarities 
~~~~~~~~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceSimilarity` objects.

Signature 
^^^^^^^^^

::

    typedef JaniceSimilarity* JaniceSimilarities;

.. _JaniceSearchTemplateIds:

JaniceSearchTemplateIds 
~~~~~~~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceTemplateId` objects.

Signature 
^^^^^^^^^

::

    typedef JaniceTemplateId* JaniceSearchTemplateIds;

Functions
---------

.. _janice\_verify:

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

This function is reentrant.

.. _comparison_sim_score:

Similarity Score 
^^^^^^^^^^^^^^^^

This API expects that the comparison of two templates results in a
single value that quantifies the similarity between them. A similarity
score is constrained by the following requirements:

::

    1. Higher scores indicate greater similarity
    2. Scores can be asymmetric. This mean verify(a, b) does not necessarily
       equal verify(b, a)

Parameters 
^^^^^^^^^^

+--------------+----------------------------+----------------------------------+
| Name         | Type                       | Description                      |
+==============+============================+==================================+
| reference    | :ref:`JaniceConstTemplate` | A reference template. This       |
|              |                            | template was created with the    |
|              |                            | *Janice11Reference* role.        |
+--------------+----------------------------+----------------------------------+
| verification | :ref:`JaniceConstTemplate` | A verification template. This    |
|              |                            | this template was created with   |
|              |                            | the *Janice11Verification* role. |
+--------------+----------------------------+----------------------------------+
| similarity   | :ref:`JaniceSimilarity` \* | A similarity score. See          |
|              |                            | :ref:`comparison_sim_score`.     |
+--------------+----------------------------+----------------------------------+

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

.. _janice\_search:

janice\_search 
~~~~~~~~~~~~~~

Compute 1-N search results between a query template object and a target
gallery object. The function allocates two arrays of equal size, one containing 
:ref:`comparison_sim_score` and the other containing the unique id of the 
template the score was computed with (along with the query). Often it is 
desirable (and perhaps computationally efficient) to only see the top K scores 
out of N possible templates. The option to set a K is provided to the user as
part of the function parameters.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                            JaniceConstGallery gallery,
                                            uint32_t num_requested,
                                            JaniceSimilarities* similarities,
                                            JaniceSearchTemplateIds* ids,
                                            uint32_t* num_returned);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+----------------+-----------------------------------+------------------------------------+
| Name           | Type                              | Description                        |
+================+===================================+====================================+
| probe          | :ref:`JaniceConstTemplate`        | A template to use as a query. The  |
|                |                                   | template was created with the      |
|                |                                   | Janice1NProbe role.                |
+----------------+-----------------------------------+------------------------------------+
| gallery        | :ref:`JaniceConstGallery`         | A gallery object to search against.|
+----------------+-----------------------------------+------------------------------------+
| num\_requested | uint32\_t                         | The number of requested number of  |
|                |                                   | returns. If the user would like as |
|                |                                   | many returns as there are templates|
|                |                                   | in the gallery they can set this to|
|                |                                   | 0.                                 |
+----------------+-----------------------------------+------------------------------------+
| similarities   | :ref`JaniceSimilarities` \*       | An uninitialized array of          |
|                |                                   | similarity scores. The scores must |
|                |                                   | be in descending order (i.e. the   |
|                |                                   | highest score is stored at index   |
|                |                                   | 0). The implementor should allocate|
|                |                                   | this object during the function    |
|                |                                   | call. The user is responsible for  |
|                |                                   | freeing the object with            |
|                |                                   | :ref:`janice\_free\_similarities`. |
+----------------+-----------------------------------+------------------------------------+
| ids            | :ref:`JaniceSearchTemplateIds` \* | An uninitialized array of unique   |
|                |                                   | ids identifying the target         |
|                |                                   | templates associated with each     |
|                |                                   | score in *similarities*. This array|
|                |                                   | must be the same size as           |
|                |                                   | *similarities*. The *ith* id in    |
|                |                                   | this array corresponds with the    |
|                |                                   | *ith* similarity in *similarities*.|
|                |                                   | The implementor should allocate    |
|                |                                   | this object during the function    |
|                |                                   | call. The user is responsible for  |
|                |                                   | freeing the object by calling      |
|                |                                   | :ref:`janice\_free\_search\_ids`.  |
+----------------+-----------------------------------+------------------------------------+
| num\_returned  | uint32\_t\*                       | The number of elements in the      |
|                |                                   | *similarities* and *ids* arrays.   |
|                |                                   | This number can be different from  |
|                |                                   | *num\_requested*.                  |
+----------------+-----------------------------------+------------------------------------+

Example 
^^^^^^^

::

    JaniceTemplate probe;  // Where probe is a valid template object created
                           // previously
    JaniceGallery gallery; // Where gallery is a valid gallery object created
                           // previously
    const uint32_t num_requested = 50; // Request the top 50 matches

    JaniceSimilarities similarities = NULL;
    JaniceSearchTemplateIds ids = NULL;
    uint32_t num_returned;

    // Run search
    if (janice_search(probe, gallery, num_requested, &similarities, &ids, &num_returned) != JANICE_SUCCESS)
        // ERROR!

    num_requested == num_returned; // This might not be true!

.. _janice\_free\_similarities:

janice\_free\_similarities 
~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceSimilarities` object.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_similarities(JaniceSimilarities* similarities);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+----------------+------------------------------+------------------------------+
| Name           | Type                         | Description                  |
+================+==============================+==============================+
| similarities   | :ref:`JaniceSimilarities` \* | An array of similarities to  |
|                |                              | free.                        |
+----------------+------------------------------+------------------------------+

.. _janice\_free\_search\_ids:

janice\_free\_search\_ids
~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceSearchTemplateIds` object.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_search_ids(JaniceSearchTemplateIds* ids);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+--------+-----------------------------------+--------------------------+
| Name   | Type                              | Description              |
+========+===================================+==========================+
| ids    | :ref:`JaniceSearchTemplateIds` \* | An array of ids to free. |
+--------+-----------------------------------+--------------------------+
