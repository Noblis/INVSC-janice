This is a reference implementation of the I/O functions defined in the API. It
is designed to load image information only as needed (i.e. lazily) and supports
most modern image and video codecs through the [OpenCV](http://opencv.org)
library. Note that because OpenCV is a C++ library this is a C++ header.

# JaniceMediaType

This implementation provides an object definition for the
[JaniceMediaType](../api/objects.md#JaniceMediaType) object.

## Fields {: #JaniceMediaTypeFields }

Name     | Type                                                             | Definition
-------- | ---------------------------------------------------------------- | ----------
filename | [std::string](http://www.cplusplus.com/reference/string/string/) | The file name of the media object
channels | uint32_t                                                         | The number of channels in the media. This will only ever be 1 (Grayscale) or 3 (Color)
rows     | uint32_t                                                         | The number of rows in the media
cols     | uint32_t                                                         | The number of columns in the media
frames   | uint32_t                                                         | The number of frames in the media. If the media is an image this will be 1

## Methods {: #JaniceMediaTypeMethods }

### next {: #JaniceMediaTypeNext}

Retrieve the image if the media is an image or the next frame from the video if
the media is a video.

#### Signature {: #JaniceMediaTypeNextSignature }
```
JANICE_EXPORT bool JaniceMediaType::next(cv::Mat& img);
```

#### Thread Safety {: #JaniceMediaTypeNextThreadSafety }

This function is thread unsafe.

#### Parameters {: #JaniceMediaTypeNextParameters }

Name | Type                                                                | Description
---- | ------------------------------------------------------------------- | -----------
img  | [cv::Mat](http://docs.opencv.org/3.1.0/d3/d63/classcv_1_1Mat.html)& | A reference to hold either the image or the next frame.

#### Return Value {: #JaniceMediaTypeNextReturnValue }

This function returns true if the media is an image or if the last frame was retrieved and false otherwise.

#### Notes
The current behavior of the implementation is to reset
the media automatically after the final call to next. If the media is an image
this means that successive calls to next() will store the same information in
the reference parameter and return true each time. If the media is a video,
calling next() again after the last frame is retrieved will cause the video to
loop back to the beginning and store the first frame in the reference parameter
and return false.

### seek {: #JaniceMediaTypeSeek }

Seek through a video to a particular location. If the media is an image this function can be considered a no-op.

#### Signature {: #JaniceMediaTypeSeekSignature }
```
JANICE_EXPORT bool JaniceMediaType::seek(uint32_t frame);
```

#### Thread Safety {: #JaniceMediaTypeSeekThreadSafety }

This function is thread unsafe.

#### Parameters

Name  | Type     | Description
----- | -------- | -----------
frame | uint32_t | The frame number to seek to

#### Return Value

This function returns true if the media is a video and the frame is valid (i.e.
within the bounds of the video) and false if the frame is invalid or the media
is an image.
