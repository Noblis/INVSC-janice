## janice_image_access {: #JaniceImageAccess }

Get a pixel value at a given row, column and channel.

#### Signature {: #JaniceImageAccessSignature }

```
inline uint8_t janice_image_access(JaniceConstImage image, uint32_t channel, uint32_t row, uint32_t col);
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

