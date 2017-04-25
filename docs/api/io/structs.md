## JaniceImageType {: #JaniceImageType }

An interface representing a single frame or an image

#### Fields {: #JaniceImageTypeFields }

Name     | Type                          | Description
-------- | ----------------------------- | -----------
channels | uint32_t                      | The number of channels in the image
rows     | uint32_t                      | The number of rows in the image
cols     | uint32_t                      | The number of columns in the image
data     | [JaniceBuffer](#JaniceBuffer) | A contiguous, row-major array containing pixel data
owner    | bool                          | True if the image owns its data and should delete it, false otherwise.


## JaniceMediaIteratorState {: #JaniceMediaIteratorState }

A void pointer to a user-defined structure that contains state required for a
[JaniceMediaIteratorType](#JaniceMediaIteratorType).


## JaniceMediaIteratorType {: #JaniceMediaIteratorType }

An interface representing a single image or video. JaniceMediaIteratorType
implements an iterator interface on media to enable lazy loading via function
pointers.

#### Fields {: #JaniceMediaIteratorTypeFields }

Name | Type | Description
---- | ---- | -----------
next | JaniceError(JaniceMediaIteratorType\*, [JaniceImage\*](#JaniceImageType)) | A function pointer that advances the iterator one frame. The next frame or image should be stored in the [JaniceImage\*](#JaniceImageType) parameter.
seek | JaniceError(JaniceMediaIteratorType\*, uint32_t) | A function pointer that advances the iterator to a specific frame. This function is not applicable to images.
get  | JaniceError(JaniceMediaIteratorType\*, [JaniceImage\*](#JaniceImageType), uint32_t) | A function pointer that advances the iterator to a specific frame and retrieves that frame. This function is not applicable to images.
tell | JaniceError(JaniceMediaIteratorType\*, uint32_t\*) | A function pointer to report the current position of the iterator. This function is not applicable to images.
free_image | JaniceError([JaniceImage\*](#JaniceImageType) | A function pointer to free an image allocated by *next* or *get*.

