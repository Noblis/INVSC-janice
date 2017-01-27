In the context of this API, detection is used to refer to the identification of
objects of interest within a [media](../io/overview.md) object. Detections are
represented using the [JaniceDetectionType](structs.md#JaniceDetectionType)
object which implementors are free to define however they would like. For
images, a detection is defined as a rectangle that bounds an object of interest
and an associated confidence value. For video, a single object can exist in
multiple frames. A rectangle and confidence are only relevant in a single
frame. In this case, we define a detection as a list of (rectangle, confidence)
pairs that track a single object through a video. It is not required that this
list be dense however (i.e. frames can be skipped). To support this, we extend
our representation of a detection to a (rectangle,confidence,frame) tuple where
frame gives the index of the frame the rectangle was found in.
