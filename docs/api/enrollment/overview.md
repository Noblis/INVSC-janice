This API defines feature extraction as the process of turning 1 or more
[detections](../io/overview.md) that refer to the same object into a single
representation. This representation is defined in the API using the
[JaniceTemplateType](structs.md#JaniceTemplateType) object. In some cases (e.g.
face recognition) this model of [multiple detections] -> [single
representation] contradicts the current paradigm of [single detection] ->
[single representation]. Implementors are free to implement whatever paradigm
they choose internally (i.e. a JanICE template could be a simple list of single
detection templates) provided the [comparison](../comparison/overview.md)
functions work appropriately.
