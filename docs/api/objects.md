# Overview

Janice objects generally fall into two classes; utility objects that exist
to simplify function parameters or represent basic classes, and fundamental
objects that correspond to high level API concepts. For fundamental objects,
the API employs the [PIMPL idiom](http://en.cppreference.com/w/cpp/language/pimpl)
to abstract away the implementation from the end user. This has the added
advantage of not enforcing any paradigm on implementors to implement their
fundamental objects in a predefined manner.

## JaniceMediaType {: #JaniceMediaType }

An opaque pointer to a struct that represents an image or video. See [the
paragraph on media](#Media) for more information. The API deals exclusively
with pointers to this object and provides the following typedefs for convenience

Name             | Definition
---------------- | ----------
JaniceMedia      | <code>typedef struct JaniceMediaType\*</code>
JaniceConstMedia | <code>typedef const struct JaniceMediaType\*</code>

## JaniceRect {: #JaniceRect }

A simple struct that represents a rectangle

#### Fields {: #JaniceRectFields }

Name   | Type     | Description
------ | -------- | -----------
x      | uint32_t | The x offset of the rectangle in pixels
y      | uint32_t | The y offset of the rectangle in pixels
width  | uint32_t | The width of the rectangle in pixels
height | uint32_t | The height of the rectangle in pixels

## JaniceDetectionInstance {: #JaniceDetectionInstance }

A single detection in an image or video, represented as a rectangle,
confidence, and frame number. See [the paragraph on detection](#Detection) for
information on why this structure is necessary.

#### Confidence {: #JaniceDetectionInstanceConfidence }

The confidence value indicates a likelihood that the rectangle actually bounds
an object of interest. It is **NOT** required to be a probability and often
only has meaning relative to other confidence values from the same algorithm.
The only restriction is that a larger confidence value indicates a greater
likelihood that the rectangle bounds an object.

#### Fields {: #JaniceDetectionInstanceFields }

Name       | Type                      | Description
---------- | ------------------------- | -----------
rect       | [JaniceRect](#JaniceRect) | The rectangle that bounds the object of interest
confidence | double                    | A likelihood that the rectangle bounds an object of interest. See [this description](#Confidence)
frame      | uint32_t                  | A frame index indicating which frame the rectangle came from. If the rectangle comes from an image this should be set to 0

## JaniceDetectionType {: #JaniceDetectionType }

An opaque pointer to a struct that represents a detection. See [the paragraph
on detection](#Detection) for more information. The API deals exclusively with
pointers to this object and provides the following typedefs for convenience.

Name                 | Definition
-------------------- | ----------
JaniceDetection      | <code>typedef struct JaniceDetectionType\*</code>
JaniceConstDetection | <code>typedef const struct JaniceDetectionType\*</code>

## JaniceTemplateType {: #JaniceTemplateType }

An opaque pointer to a struct that represents a template. See [the paragraph on
feature extraction](#FeatureExtraction) for more information. The API deals
exclusively with pointers to this object and provides the following typedefs
for convienience.

Name                | Definition
------------------- | ----------
JaniceTemplate      | <code>typedef struct JaniceTemplateType\*</code>
JaniceConstTemplate | <code>typedef const struct JaniceTemplateType\*</code>

## JaniceGalleryType {: #JaniceGalleryType }

An opaque pointer to a struct that represents a gallery. See [the paragraph on
galleries](#Galleries) for more information. The API deals exclusively with
pointers to this object and provides the following typedefs for convenience.

Name               | Definition
------------------ | ----------
JaniceGallery      | <code>typedef struct JaniceGalleryType\*</code>
JaniceConstGallery | <code>typedef const struct JaniceGalleryType\*</code>

## JaniceClusterItem {: #JaniceClusterItem }

A utility structure that stores cluster information about an API object.

### Fields

Name | Type | Description
---- | ---- | -----------
cluster_id | uint32_t | The id of the cluster that this item belongs to. Two items with the same cluster id are members of the same cluster
source_id | uint32_t | The id of the source object this item refers to. Two items with the same source id came from the same source object.
confidence | double | The confidence that this item belongs in it's assigned cluster. The confidence could be used after the fact to "purify" clusters by removing the least confident members.
detection | [JaniceDetection](#JaniceDetectionType) | In the special case where the source object is a [JaniceMediaType](#JaniceMediaType) object, the source id is not enough to handle the case where multiple objects of interest exist within the media. The detection is used in this case to give the specific location of the object that this item refers to.
