.. _context:

Context
=======

A context is a single object for managing the various hyperparameters parameters
required by JanICE functions. 

Enumerations
------------

.. _JaniceDetectionPolicy:

JaniceDetectionPolicy
~~~~~~~~~~~~~~~~~~~~~

A policy that controls the types of objects that should be detected by a call
to :ref:`janice_detect`. Supported policies are:

+---------------------+-------------------------------------------------------+
|       Policy        |                      Description                      |
+=====================+=======================================================+
| JaniceDetectAll     | Detect all objects present in the media.              |
+---------------------+-------------------------------------------------------+
| JaniceDetectLargest | Detect the largest object present in the media.       |
|                     | Running detection with this policy should produce at  |
|                     | most one detection.                                   |
+---------------------+-------------------------------------------------------+
| JaniceDetectBest    | Detect the best object present in the media. The      |
|                     | implementor is responsible for defining what "best"   |
|                     | entails in the context of their algorithm. Running    |
|                     | detection with this policy should produce at most one |
|                     | detection.                                            |
+---------------------+-------------------------------------------------------+

.. _JaniceEnrollmentType:

JaniceEnrollmentType
~~~~~~~~~~~~~~~~~~~~

Often times, the templates produced by algorithms will require different
data for different use cases. The enrollment type indicates what the use
case for the created template will be, allowing implementors to
specialize their templates if they so desire. The use cases supported by
the API are:

+----------------------+-------------------------------------------------------+
|         Role         |                      Description                      |
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

.. _JaniceContext:

JaniceContext
~~~~~~~~~~~~~

A structure to hold hyperparameters. These hyperparameters may be set by the
user to control execution of the implementation algorithms. Users should consult
the relevant documentation for accepted values or ranges for these hyperparameters.
Implementors should ensure the provided values are acceptable before using them.

.. _detection_min_object_size:

Minimum Object Size
^^^^^^^^^^^^^^^^^^^

This function specifies a minimum object size as one of its parameters.
This value indicates the minimum size of objects that the user would
like to see detected. Often, increasing the minimum size can improve
runtime of algorithms. The size is in pixels and corresponds to the
length of the smaller side of the rectangle. This means a detection will
be returned if and only if its smaller side is larger than the value
specified. If the user does not wish to specify a minimum width 0 can be
provided.

.. _clustering_hint:

Hint 
^^^^

Clustering is generally considered to be an ill-defined problem, and
most algorithms require some help determining the appropriate number of
clusters. The hint parameter helps influence the number of clusters,
though the implementation is free to ignore it. The goal of the hint is
to provide user input for two use cases:

1. If the hint is between 0 - 1 it should be regarded as a purity
   requirement for the algorithm. A 1 indicates the user wants perfectly
   pure clusters, even if that means more clusters are returned. A 0
   indicates that the user wants very few clusters returned and accepts
   there may be some errors.
2. If the hint is > 1 it represents an estimated upper bound on the
   number of object types in the set.

Fields
^^^^^^

+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+
|       Name        |             Type             |                                               Description                                               |
+===================+==============================+=========================================================================================================+
| policy            | :ref:`JaniceDetectionPolicy` | The detection policy                                                                                    |
+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+
| min\_object\_size | uint32\_t                    | The minumum object size of a detection. See :ref:`detection_min_object_size` for additional information |
+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+
| role              | :ref:`JaniceEnrollmentType`  | The enrollment type for a template                                                                      |
+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+
| threshold         | double                       | The minimum acceptable score for a search result.                                                       |
+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+
| max\_returns      | uint32\_t                    | The maximum number of results a single search should return                                             |
+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+
| hint              | double                       | A hint to a clustering algorithm. See :ref:`clustering_hint` for additional information                 |
+-------------------+------------------------------+---------------------------------------------------------------------------------------------------------+

Functions
---------

.. _janice_init_default_context:

janice\_init\_default\_context
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Initialize hyperparameters of a context object with sensible defaults.
The context object should be created by the user prior to calling this function.

Signature
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_init_default_context(JaniceContext* context);
                                        
Thread Safety
^^^^^^^^^^^^^

This function is :ref:`reentrant`.

Parameters
^^^^^^^^^^

+---------+------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------+
|  Name   |          Type          |                                                                   Description                                                                   |
+=========+========================+=================================================================================================================================================+
| context | :ref:`JaniceContext`\* | The context to initialize. Memory for the object should be managed by the user. The implementation should assume this points to a valid object. |
+---------+------------------------+-------------------------------------------------------------------------------------------------------------------------------------------------+
