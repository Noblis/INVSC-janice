## JaniceMediaClusterItem {: #JaniceMediaClusterItem }

A structure that connects an input media and location with an output cluster.
Because multiple detections could occur in a single piece of media a
[JaniceRect](../detection/structs.md#JaniceRect) and a frame index are included
to identify which detection this item refers to.

#### Fields {: #JaniceMediaClusterItemFields }

Name       | Type                                             | Description
---------- | ------------------------------------------------ | -----------
cluster_id | [JaniceClusterId](typedefs.md#JaniceClusterId)   | A unique identifier for the cluster this item belongs to. Items belonging to the same cluster should have the same *cluster_id*
media_id   | [JaniceMediaId](typedefs.md#JaniceMediaId)       | A unique identifier for the media object this item corresponds to.
confidence | double | The confidence that this item belongs to this cluster.
rect       | [JaniceRect](../detection/structs.md#JaniceRect) | The location of the clustered object in a frame or image.
frame      | uint32_t                                         | The frame index of the clustered object if the media is a video, otherwise 0.

## JaniceTemplateClusterItem {: #JaniceTemplateClusterItem }

A structure that connects an input template with an output cluster.

#### Fields {: #JaniceTemplateClusterItemFields }

Name       | Type                                                        | Description
---------- | ----------------------------------------------------------- | -----------
cluster_id | [JaniceClusterId](typedefs.md#JaniceClusterId)              | A unique identifier for the cluster this item belongs to. Items belonging to the same cluster should have the same *cluster_id*
tmpl_id    | [JaniceTemplateId](../gallery/typedefs.md#JaniceTemplateId) | A unique identifier for the template object this item corresponds to.
confidence | double                                                      | The confidence that this item belongs to this cluster.
