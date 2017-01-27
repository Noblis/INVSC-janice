## janice_create_image {: #JaniceCreateImage }

Create an image with the provided dimensions. This function should allocate and
own its data buffer.

#### Signature {: #JaniceCreateImageSignature }

```
JANICE_EXPORT JaniceError janice_create_image(uint32_t channels,
                                              uint32_t rows,
                                              uint32_t cols,
                                              JaniceImage* image);
```

#### Thread Safety {: #JaniceCreateImageThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceCreateImageParameters }

Name     | Type                                    | Description
-------- | --------------------------------------- | -----------
channels | uint32_t                                | The number of channels the image should have
rows     | uint32_t                                | The number of rows the image should have
cols     | uint32_t                                | The number of columns the image should have
image    | [JaniceImage\*](typedefs.md#JaniceImage) | An uninitialized pointer to an image object

## janice_create_image_from_buffer {: #JaniceCreateImageFromBuffer }

Create an image with the provided dimensions and data. The image should borrow
the data not own it.

#### Signature {: #JaniceCreateImageFromBufferSignature }

```
JANICE_EXPORT JaniceError janice_create_image_from_buffer(uint32_t channels,
                                                          uint32_t rows,
                                                          uint32_t cols,
                                                          const JaniceBuffer data,
                                                          JaniceImage* image);
```

#### Thread Safety {: #JaniceCreateImageFromBufferThreadSafety }

This function is reentrant

#### Parameters {: #JaniceCreateImageFromBufferParameters }

Name     | Type                                     | Description
-------- | ---------------------------------------- | -----------
channels | uint32_t                                 | The number of channels the image should have
rows     | uint32_t                                 | The number of rows the image should have
cols     | uint32_t                                 | The number of columns the image should have
data     | [JaniceBuffer](typedefs.md#JaniceBuffer) | An array of data the image should use. The array must have length equal to channels * rows * cols and be contiguous and describe image data in row-major order
image    | [JaniceImage](typedefs.md#JaniceImage)   | An uninitialized pointer to an image object.

## janice_image_access {: #JaniceImageAccess }

Get a pixel value at a given row, column and channel.

#### Signature {: #JaniceImageAccessSignature }

```
JANICE_EXPORT JaniceError janice_image_access(JaniceConstImage image,
                                              uint32_t channel,
                                              uint32_t row,
                                              uint32_t col,
                                              uint8_t* value);
```

#### Thread Safety {: #JaniceImageAccessThreadSafety }

This function is reentrant

#### Parameters {: #JaniceImageAccessParameters }

Name    | Type                                            | Description
------- | ----------------------------------------------- | -----------
image   | [JaniceConstImage](typedefs.md#JaniceConstImage) | An image object
channel | uint32_t                                        | The channel to access. Must be less than image->channels.
row     | uint32_t                                        | The row to access. Must be less than image->rows.
col     | uint32_t                                        | The column to access. Must be less that image->cols.
value   | uint8_t\*                                       | A container for the value at the given index.

## janice_free_image {: #JaniceFreeImage }

Free any memory associated with a [JaniceImage](typedefs.md#JaniceImage).

#### Signature {: #JaniceFreeImageSignature }

```
JANICE_EXPORT JaniceError janice_free_image(JaniceImage* image);
```

#### Thread Safety {: #JaniceFreeImageThreadSafety }

This function is reentrant

#### Parameters {: #JaniceFreeImageParameters }

Name  | Type                                     | Description
----- | ---------------------------------------- | -----------
image | [JaniceImage\*](typedefs.md#JaniceImage) | An image object to free

## janice_media_it_next {: #JaniceMediaItNext }

Get the next frame or load the image from a media iterator.

#### Signature {: #JaniceMediaItNextSignature }

```
JANICE_EXPORT JaniceError janice_media_it_next(JaniceMediaIterator it,
                                               JaniceImage* image);
```

#### Thread Safety {: #JaniceMediaItNextThreadSafety }

This function is reentrant

#### Parameters {: #JaniceMediaItNextParameters }

Name  | Type                                                   | Description
----- | ------------------------------------------------------ | -----------
it    | [JaniceMediaIterator](typedefs.md#JaniceMediaIterator) | A media iterator object to query
image | [JaniceImage\*](typedefs.md#JaniceImage)               | An uninitialized pointer to an image object to hold the next frame or image

## janice_media_it_seek {: #JaniceMediaItSeek }

Seek to a provided frame. Seek should work such that a successive call to
[next](#JaniceMediaItNext) returns the frame at the index provided to seek.

#### Signature {: #JaniceMediaItSeekSignature }

```
JANICE_EXPORT JaniceError janice_media_it_seek(JaniceMediaIterator it,
                                               uint32_t frame);
```

#### Thread Safety {: #JaniceMediaItSeekThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceMediaItSeekParameters }

Name  | Type                                                   | Description
----- | ------------------------------------------------------ | -----------
it    | [JaniceMediaIterator](typedefs.md#JaniceMediaIterator) | A media iterator object to seek in
frame | uint32_t                                               | The frame to seek to. It must be less than the total number of frames in the video.

## janice_media_it_get {: #JaniceMediaItGet }

Get a specified frame from a media object. This method should be functionally
equivalent to calling <code>janice_media_it_seek(frame)</code> followed by
<code>janice_media_it_next(image)</code>.

#### Signature {: #JaniceMediaItGetSignature }

```
JANICE_EXPORT JaniceError janice_media_it_get(JaniceMediaIterator it,
                                              JaniceImage* image,
                                              uint32_t frame);
```

#### Thread Safety {: #JaniceMediaItGetThreadSafety }

This function is reentrant

#### Parameters {: #JaniceMediaItGetParameters }

Name  | Type                                                   | Description
----- | ------------------------------------------------------ | -----------
it    | [JaniceMediaIterator](typedefs.md#JaniceMediaIterator) | A media iterator object to get a frame from
image | [JaniceImage\*](typedefs.md#JaniceImage)               | An uninitialized image object to hold the frame or image
frame | uint32_t                                               | The index of the frame to get. This must be less than the number of frames in the video.

## janice_media_it_tell {: #JaniceMediaItTell }

Get the current frame position of a media object.

#### Signature {: #JaniceMediaItTellSignature }

```
JANICE_EXPORT JaniceError janice_media_it_tell(JaniceMediaIterator it,
                                               uint32_t* frame);
```

#### Thread Safety {: #JaniceMediaItTellThreadSafety }

This function is reentrant

#### Parameters {: #JaniceMediaItTellParameters }

Name  | Type                                                   | Description
----- | ------------------------------------------------------ | -----------
it    | [JaniceMediaIterator](typedefs.md#JaniceMediaIterator) | A media object to get the current frame position from
frame | uint32_t\*                                             | A container to hold the queried frame value

## janice_free_media_iterator {: #JaniceFreeMediaIterator }

Free any memory associated with a [JaniceMediaIterator](typedefs.md#JaniceMediaIterator).

#### Signature {: #JaniceFreeMediaIteratorSignature }

```
JANICE_EXPORT JaniceError janice_free_media_iterator(JaniceMediaIterator* it);
```

#### Thread Safety {: #JaniceFreeMediaIteratorThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeMediaIteratorParameters }

Name | Type                                                     | Description
---- | -------------------------------------------------------- | -----------
it   | [JaniceMediaIterator\*](typedefs.md#JaniceMediaIterator) | An iterator object to free

## janice_create_media {: #JaniceCreateMedia }

Create a media object from a file

#### Signature {: #JaniceCreateMediaSignature }

```
JANICE_EXPORT JaniceError janice_create_media(const char* filename,
                                              JaniceMedia* media);
```

#### Thread Safety {: #JaniceCreateMediaThreadSafety }

This function is reentrant

#### Parameters {: #JaniceCreateMediaParameters }

Name     | Type                                     | Description
-------- | ---------------------------------------- | -----------
filename | const char\*                             | A path to a file on disk
media    | [JaniceMedia\*](typedefs.md#JaniceMedia) | An uninitialized pointer to a media object

## janice_media_get_iterator {: #JaniceMediaGetIterator }

Get an iterator from a media object.

#### Signature {: #JaniceMediaGetIteratorSignature }

```
JANICE_EXPORT JaniceError janice_media_get_iterator(JaniceConstMedia media,
                                                    JaniceMediaIterator* it);
```

#### Thread Safety {: #JaniceMediaGetIteratorThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceMediaGetIteratorParameters }

Name  | Type                                                     | Description
----- | -------------------------------------------------------- | -----------
media | [JaniceConstMedia](typedefs.md#JaniceConstMemo)          | A media object to get an iterator from
it    | [JaniceMediaIterator\*](typedefs.md#JaniceMediaIterator) | An uninitialized iterator object.

## janice_free_media {: #JaniceFreeMedia }

Free any memory associated with a [JaniceMedia](typedefs.md#JaniceMedia).

#### Signature {: #JaniceFreeMediaSignature}

```
JANICE_EXPORT JaniceError janice_free_media(JaniceMedia* media);
```

#### Thread Safety {: #JaniceFreeMediaThreadSafety }

This function is reentrant

#### Parameters {: #JaniceFreeMediaParameters }

Name  | Type                                     | Description
----- | ---------------------------------------- | -----------
media | [JaniceMedia\*](typedefs.md#JaniceMedia) | A media object to free
