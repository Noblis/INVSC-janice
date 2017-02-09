## janice_cluster_media

Cluster a collection of media objects into groups. Each media object may
contain 0 or more objects of interest. To distinguish between these objects
the output cluster contains an object location.

#### Signature {: #JaniceClusterMediaSignature }

```
JANICE_EXPORT JaniceError janice_cluster_media(JaniceConstMedias input,
                                               const JaniceMediaIds input_ids,
                                               uint32_t num_inputs,
                                               uint32_t hint,
                                               JaniceMediaClusterItems* clusters,
                                               uint32_t* num_clusters);
```

#### Thread Safety {: #JaniceClusterMediaThreadSafety }

This function is reentrant.

#### Hint {: #JaniceClusterMediaHint }

Clustering is generally considered to be an ill-defined problem, and most
algorithms require some help determining the appropriate number of clusters.
The hint parameter helps influence the number of clusters, though the
implementation is free to ignore it. The goal of the hint is to provide user
input for two use cases:

1. If the hint is between 0 - 1 it should be regarded as a purity requirement for the algorithm. A 1 indicates the user wants perfectly pure clusters, even if that means more clusters are returned. A 0 indicates that the user wants very few clusters returned and accepts there may be some errors.
2. If the hint is > 1 it represents an estimated upper bound on the number of object types in the set.

#### Parameters {: #JaniceClusterMediaParameters }

Name         | Type                                                             | Description
------------ | ---------------------------------------------------------------- | -----------
input        | [JaniceConstMedias](../io/typedefs.md#JaniceConstMedias)         | An array of media objects to cluster
input_ids    | const [JaniceMediaIds](typedefs.md#JaniceMediaIds)               | An array of unique identifiers for the input objects. This array must be the same size as *input* and the *ith* id corresponds to the *ith* media object in the input.
num_inputs   | uint32_t                                                         | The size of both the input and input_ids arrays.
hint         | uint32_t                                                         | A [hint](#JaniceClusterMediaHint) to the clustering algorithm.
clusters     | [JaniceMediaClusterItems\*](typedefs.md#JaniceMediaClusterItems) | An uninitialized array to hold the cluster output.
num_clusters | uint32_t\*                                                       | The size of the *clusters* array.

## janice_cluster_templates {: #JaniceClusterTemplates }

Cluster a collection of template objects into groups.

#### Signature {: #JaniceClusterTemplatesSignature }

```
JANICE_EXPORT JaniceError janice_cluster_templates(JaniceConstTemplates input,
                                                   const JaniceTemplateIds input_ids,
                                                   uint32_t num_inputs,
                                                   uint32_t hint,
                                                   JaniceTemplateClusterItems* clusters,
                                                   uint32_t* num_clusters);
```

#### Thread Safety {: #JaniceClusterTemplatesThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceClusterTemplatesParameters }

Name         | Type                                                                   | Description
------------ | ---------------------------------------------------------------------- | -----------
input        | [JaniceConstTemplates](../enrollment/typedefs.md#JaniceConstTemplates) | An array of template objects to cluster
input_ids    | const [JaniceTemplateIds](../gallery/typedefs.md#JaniceMediaIds)       | An array of unique identifiers for the input objects. This array must be the same size as *input* and the *ith* id corresponds to the *ith* media object in the input.
num_inputs   | uint32_t                                                               | The size of both the input and input_ids arrays.
hint         | uint32_t                                                               | A [hint](#JaniceClusterMediaHint) to the clustering algorithm.
clusters     | [JaniceTemplateClusterItems\*](typedefs.md#JaniceTemplateClusterItems) | An uninitialized array to hold the cluster output.
num_clusters | uint32_t\*                                                             | The size of the *clusters* array.

## janice_free_media_cluster_items {: #JaniceFreeMediaClusterItems }

Free any memory associated with a [JaniceMediaClusterItems](typedefs.md#JaniceMediaClusterItems) object.

#### Signature {: #JaniceFreeMediaClusterItemsSignature }

```
JANICE_EXPORT JaniceError janice_free_media_cluster_items(JaniceMediaClusterItems* clusters);
```

#### Thread Safety {: #JaniceFreeMediaClusterItemsThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeMediaClusterItemsParameters }

Name     | Type                                                             | Description
-------- | ---------------------------------------------------------------- | -----------
clusters | [JaniceMediaClusterItems\*](typedefs.md#JaniceMediaClusterItems) | The object to free

## janice_free_template_cluster_items {: #JaniceFreeTemplateClusterItems }

Free any memory associated with a [JaniceTemplateClusterItems](typedefs.md#JaniceTemplateClusterItems) object.

#### Signature {: #JaniceFreeTemplateClusterItemsSignature }

```
JANICE_EXPORT JaniceError janice_free_template_cluster_items(JaniceTemplateClusterItems* clusters);
```

#### Thread Safety {: #JaniceFreeTemplateClusterItemsThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeTemplateClusterItemsParameters }

Name     | Type                                                                   | Description
-------- | ---------------------------------------------------------------------- | -----------
clusters | [JaniceTemplateClusterItems\*](typedefs.md#JaniceTemplateClusterItems) | The object to free

