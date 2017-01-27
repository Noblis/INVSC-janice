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
confidence, and frame number. See [the detection overview](overview.md) for
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
confidence | double                    | A likelihood that the rectangle bounds an object of interest. See [this description](#JaniceDetectionInstanceConfidence)
frame      | uint32_t                  | A frame index indicating which frame the rectangle came from. If the rectangle comes from an image this should be set to 0

## JaniceDetectionType {: #JaniceDetectionType }

An opaque pointer to a struct that represents a detection. See
[the detection overview](overview.md) for more information.
