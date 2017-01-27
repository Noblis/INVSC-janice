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

## JaniceMediaIteratorType {: #JaniceMediaIteratorType }

An opaque object that iterates over the frames in a media object. The object
can contain any implementation specific variables or state required to
implement the iteration interface described [here](functions.md).

## JaniceMediaType {: #JaniceMediaType }

An interface representing an image or video

#### Fields {: #JaniceMediaTypeFields }

Name      | Type     | Description
--------- | -------- | -----------
filename  | char\*   | A null-terminated string that contains the location of the media on disk.
channels  | uint32_t | The number of channels in the media
rows      | uint32_t | The number of rows in the media
cols      | uint32_t | The number of columns in the media
frames    | uint32_t | The number of frames in the media
