## JaniceRect {: #JaniceRect }

A simple struct that represents a rectangle

#### Fields {: #JaniceRectFields }

Name   | Type     | Description
------ | -------- | -----------
x      | uint32_t | The x offset of the rectangle in pixels
y      | uint32_t | The y offset of the rectangle in pixels
width  | uint32_t | The width of the rectangle in pixels
height | uint32_t | The height of the rectangle in pixels

## JaniceDetectionIteratorType {: #JaniceDetectionIteratorType }

An opaque pointer to an iterator class through a detection. If the detection
was computed from an image, the iterator should only move over a single value.
If the detection was computed from a video, the iterator should move over an
array of elements, the length of which is less than or equal to the number of
frames in the video, and might be sparse (i.e. frames can be skipped).

## JaniceDetectionType {: #JaniceDetectionType }

An opaque pointer to a struct that represents a detection. See
[the detection overview](overview.md) for more information.
