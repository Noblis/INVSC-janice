As a computer vision API it is a requirement that images and videos are loaded
into a common structure that can be processed by the rest of the API. In this
case, we strive to isolate the I/O functions from the rest of the API. This
serves three purposes:

1. It allows implementations to be agnostic to the method and type of image storage, compression techniques and other factors
2. It keeps implementations from having to worry about licenses, patents and other factors that can arise from distributing proprietary image formats
3. It allows implementations to be "future-proof" with regards to future developments of image or video formats

To accomplish this goal the API defines a simple interface of two structures,
[JaniceImageType](structs.md#JaniceImage) and 
[JaniceMediaType](structs.md#JaniceMedia) which correspond to
a single image or frame and an entire video respectively. These interfaces
allow pixel-level access for implementations and can be changed independently
to work with new formats.
