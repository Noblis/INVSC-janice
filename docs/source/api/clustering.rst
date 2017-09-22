.. _clustering:

Clustering
==========

Overview
--------

This API defines clustering is the automatic and unsupervised combination of 
unlabelled templates into groups of like templates. What constitutes likeness is
heavily dependent on the use case and context in question. One example when 
dealing with faces is grouping based on identity, where all faces belonging to a 
single individual are placed in a cluster.

Structs
-------

.. _JaniceMediaClusterItem:

JaniceMediaClusterItem 
~~~~~~~~~~~~~~~~~~~~~~

A structure that connects an input media and location with an output cluster. 
Because multiple detections could occur in a single piece of media a 
:ref:`JaniceRect` and a frame index are included to identify which detection 
this item refers to.

Fields 
^^^^^^

+-------------+------------------------+---------------------------------------+
| Name        | Type                   | Description                           |
+=============+========================+=======================================+
| cluster\_id | :ref:`JaniceClusterId` | A unique identifier for the cluster   |
|             |                        | this item belongs to. Items belonging |
|             |                        | to the same cluster should have the   |
|             |                        | the id.                               |
+-------------+------------------------+---------------------------------------+
| media\_id   | :ref:`JaniceMediaId`   | A unique identifier for the media     |
|             |                        | object this item corresponds to.      |
+-------------+------------------------+---------------------------------------+
| confidence  | double                 | The confidence that this item belongs |
|             |                        | to this cluster.                      |
+-------------+------------------------+---------------------------------------+
| rect        | :ref:`JaniceRect`      | The location of the clustered object  |
|             |                        | in a frame or image.                  |
+-------------+------------------------+---------------------------------------+
| frame       | uint32\_t              | The frame index of the clustered      |
|             |                        | object if the media is a video,       |
|             |                        | otherwise 0.                          |
+-------------+------------------------+---------------------------------------+

.. _JaniceTemplateClusterItem:

JaniceTemplateClusterItem 
~~~~~~~~~~~~~~~~~~~~~~~~~

A structure that connects an input template with an output cluster.

Fields 
^^^^^^

+-----------+---------------------------+--------------------------------------+
| Name      | Type                      | Description                          |
+=============+=========================+======================================+
| cluster\_id | :ref:`JaniceClusterId`  | A unique identifier for the cluster  |
|             |                         | this item belongs to. Items belonging|
|             |                         | to the same cluster should have the  |
|             |                         | same id.                             |
+-------------+-------------------------+--------------------------------------+
| tmpl\_id    | :ref:`JaniceTemplateId` | A unique identifier for the template |
|             |                         | object this item corresponds to.     |
+-------------+-------------------------+--------------------------------------+
| confidence  | double                  | The confidence that this item belongs|
|             |                         | to this cluster.                     |
+-------------+-------------------------+--------------------------------------+

Typedefs
--------

.. _JaniceClusterId:

JaniceClusterId 
~~~~~~~~~~~~~~~

A unique identifier for a cluster

Signature 
^^^^^^^^^

::

    typedef uint32_t JaniceClusterId;

.. _JaniceMediaClusterItems:

JaniceMediaClusterItems 
~~~~~~~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceMediaClusterItem` objects.

Signature 
^^^^^^^^^

::

    typedef struct JaniceMediaClusterItem* JaniceMediaClusterItems;

.. _JaniceTemplateClusterItems:

JaniceTemplateClusterItems 
~~~~~~~~~~~~~~~~~~~~~~~~~~

An array of :ref:`JaniceTemplateClusterItem` objects.

Signature 
^^^^^^^^^

::

    typedef struct JaniceTemplateClusterItem* JaniceTemplateClusterItems;

Function
--------

.. _janice\_cluster\_media:

janice\_cluster\_media
~~~~~~~~~~~~~~~~~~~~~~

Cluster a collection of media objects into groups. Each media object may
contain 0 or more objects of interest. To distinguish between these
objects the output cluster contains an object location.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_cluster_media(JaniceConstMedias input,
                                                   const JaniceMediaIds input_ids,
                                                   uint32_t num_inputs,
                                                   uint32_t hint,
                                                   JaniceMediaClusterItems* clusters,
                                                   uint32_t* num_clusters);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

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

Parameters 
^^^^^^^^^^

+---------------+-----------------------------------+---------------------------------------------+
| Name          | Type                              | Description                                 |
+===============+===================================+=============================================+
| input         | :ref:`JaniceMediaIterators`       | An array of media objects to cluster.       |
+---------------+-----------------------------------+---------------------------------------------+
| input\_ids    | const :ref:`JaniceMediaIds`       | An array of unique indentifiers for the     |
|               |                                   | input objects. This array must be the same  |
|               |                                   | size as *input*. The *ith* id should        |
|               |                                   | correspond to the *ith* media object in     |
|               |                                   | *input*.                                    |
+---------------+-----------------------------------+---------------------------------------------+
| num\_inputs   | uint32\_t                         | The size of the *input* and *input\_ids*    |
|               |                                   | arrays.                                     |
+---------------+-----------------------------------+---------------------------------------------+
| hint          | uint32\_t                         | A :ref:`clustering_hint` to the clustering  |
|               |                                   | algorithm.                                  |
+---------------+-----------------------------------+---------------------------------------------+
| clusters      | :ref:`JaniceMediaClusterItems` \* | An uninitialized array of cluster items to  |
|               |                                   | store the result of clustering. The         |
|               |                                   | implementor should allocate this object     |
|               |                                   | during the function call. The user is       |
|               |                                   | responsible for freeing the object by       |
|               |                                   | calling                                     |
|               |                                   | :ref:`janice\_free\_media\_cluster\_items`. |
+---------------+-----------------------------------+---------------------------------------------+
| num\_clusters | uint32\_t\*                       | The size of the *clusters* array.           |
+---------------+-----------------------------------+---------------------------------------------+

.. _janice\_cluster\_templates:

janice\_cluster\_templates 
~~~~~~~~~~~~~~~~~~~~~~~~~~

Cluster a collection of template objects into groups.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_cluster_templates(JaniceConstTemplates input,
                                                       const JaniceTemplateIds input_ids,
                                                       uint32_t num_inputs,
                                                       uint32_t hint,
                                                       JaniceTemplateClusterItems* clusters,
                                                       uint32_t* num_clusters);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+---------------+--------------------------------------+------------------------------------------------+
| Name          | Type                                 | Description                                    |
+===============+======================================+================================================+
| input         | :ref:`JaniceConstTemplates`          | An array of template objects to cluster.       |
+---------------+--------------------------------------+------------------------------------------------+
| input\_ids    | const :ref:`JaniceTemplateIds`       | An array of unique indentifiers for the input  |
|               |                                      | objects. This array must be the same size as   |
|               |                                      | *input*. The *ith* id should correspond to the |
|               |                                      | *ith* media object in *input*.                 |
+---------------+--------------------------------------+------------------------------------------------+
| num\_inputs   | uint32\_t                            | The size of the *input* and *input\_ids*       |
|               |                                      | arrays.                                        |
+---------------+--------------------------------------+------------------------------------------------+
| hint          | uint32\_t                            | A :ref:`clustering_hint` to the algorithm. The |
|               |                                      | implementor may ignore this value if they      |
|               |                                      | choose.                                        |
+---------------+--------------------------------------+------------------------------------------------+
| clusters      | :ref:`JaniceTemplateClusterItems` \* | An uninitialized array to hold the cluster     |
|               |                                      | output. The implementor should allocate this   |
|               |                                      | object during the function call. The user is   |
|               |                                      | for freeing the object by calling              |
|               |                                      | :ref:`janice\_free\_template\_cluster\_items`. |
+---------------+--------------------------------------+------------------------------------------------+
| num\_clusters | uint32\_t\*                          | The size of the *clusters* array.              |
+---------------+--------------------------------------+------------------------------------------------+

.. _janice\_free\_media\_cluster\_items:

janice\_free\_media\_cluster\_items 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceMediaClusterItems` object.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_media_cluster_items(JaniceMediaClusterItems* clusters);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+------------+-----------------------------------+-----------------------------+
| Name       | Type                              | Description                 |
+============+===================================+=============================+
| clusters   | :ref:`JaniceMediaClusterItems` \* | The media cluster object to |
|            |                                   | free.                       |
+------------+-----------------------------------+-----------------------------+

.. _janice\_free\_template\_cluster\_items:

janice\_free\_template\_cluster\_items 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free any memory associated with a :ref:`JaniceTemplateClusterItems` object.

Signature 
^^^^^^^^^

::

    JANICE_EXPORT JaniceError janice_free_template_cluster_items(JaniceTemplateClusterItems* clusters);

Thread Safety 
^^^^^^^^^^^^^

This function is reentrant.

Parameters 
^^^^^^^^^^

+------------+--------------------------------------+--------------------------+
| Name       | Type                                 | Description              |
+============+======================================+==========================+
| clusters   | :ref:`JaniceTemplateClusterItems` \* | The template cluster     |
|            |                                      | object to free.          |
+------------+--------------------------------------+--------------------------+
