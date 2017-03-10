# Janus Changelog

This document highlights notable differences between the Janus C++ API (version 0.5.1)
and the JanICE API (version 4.0.0)

### Initialization

No significant changes

### Versioning

* The JanICE API includes 2 functions for querying version information from the implementation
    * `janice_api_version` gets the supported API version
    * `janice_sdk_version` gets the current SDK version

### Training

* The JanICE API includes a training function. This function was copied from the Phase 1 Janus API.

### Media I/O

* Janus implements media as an in-memory buffer. JanICE implements media as a lazy iterator
* Note if Janus PR #43 is merged the two implementations will closely mirror each other.

### Detection

* Janus defines two structures, `janus_attributes` and `janus_track` which hold metadata for a single detection and a collection of detections respectively. Tracks are paired with media in the `janus_association` object.
* JanICE defines an opaque object, `JaniceDetection`, which manages all detection information, including any image data.
* A `JaniceDetectionIterator` object iterates over a `JaniceDetection` to extract rectangles and confidences at (possibly sparse) frame indices.
* JanICE defines I/O functions for the `JaniceDetection` object because it is opaque
    * `janice_serialize_detection` and `janice_deserialize_detection` serialize and deserialize the detection to a flat buffer respectively.
    * `janice_read_detection` and `janice_write_detection` write the detection directly to a C `FILE` object.

### Enrollment

* Template creation is basically the same in both APIs; an array of detections is used to create a single template. Both APIs define 5 different roles a template can fulfill.
* Janus provides an overload on `janus_create_template` that creates multiple templates directly from a piece of media
* Janus serializes templates to C++ stream objects
* JanICE follows the same serialization pattern as detection- `janice_serialize_template`, `janice_deserialize_template`, `janice_read_template`, `janice_write_template`.
* JanICE has a function to query arbitrary metadata (age, gender, etc.) from a template.
    * `janice_template_get_attribute` provides a `const char*` key and expects a `char*` value (or an error if the key is invalid).

### Verification

No significant changes

### Galleries

* Both APIs offer the ability to create a gallery from a list of templates and a list of unique identifiers
* Both APIs define functions to insert and remove templates from galleries based on the unique id
* Janus serializes galleries to C++ stream objects
* JanICE follows the same serialization pattern as detections and templates- `janice_serialize_gallery`, `janice_deserialize_gallery`, `janice_read_gallery`, `janice_write_gallery`

### Search

No significant differences

### Clustering

* Janus has a single function, `janus_cluster`, which clusters a list of templates.
* JanICE has 2 functions, `janice_cluster_media` and `janice_cluster_templates` which cluster a list of media and a list of templates respectively.
* Note if Janus PR #43 is merged the two implementations will closely mirror each other.

### Finalization

No significant differences
