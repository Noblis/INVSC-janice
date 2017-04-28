## janice_create_gallery {: #JaniceCreateGallery }

Create a [JaniceGallery](typedefs.md#JaniceGallery) object from a list of
templates and unique ids.

#### Signature {: #JaniceCreateGallerySignature }

```
JANICE_EXPORT JaniceError janice_create_gallery(JaniceConstTemplates tmpls,
                                                const JaniceTemplateIds ids,
                                                uint32_t num_tmpls,
                                                JaniceGallery* gallery);
```

#### Thread Safety {: #JaniceCreateGalleryThreadSafety

This function is reentrant.

#### Parameters {: #JaniceCreateGalleryParameters }

Name      | Type                                                                   | Description
--------- | ---------------------------------------------------------------------- | -----------
tmpls     | [JaniceConstTemplates](../enrollment/typedefs.md#JaniceConstTemplates) | An array of templates to add to the gallery. This can be NULL, which should create an empty gallery.
ids       | const [JaniceTemplateIds](typedefs.md#JaniceTemplateIds)               | A set of unique identifiers to associate with the input templates. The *ith* id corresponds to the *ith* input template. This can be NULL, which should create an empty gallery.
num_tmpls | uint32_t                                                               | The number of templates in the array.
gallery   | [JaniceGallery\*](typedefs.md#JaniceGallery)                           | An uninitialized gallery object.

#### Example {: #JaniceCreateGalleryExample }

```
JaniceTemplates tmpls; // Where tmpls is a valid array of valid template
                       // objects created previously
JaniceTemplateIds ids; // Where ids is a valid array of unique unsigned integers that
                       // is the same length as tmpls
JaniceGallery gallery = NULL; // best practice to initialize to NULL

if (janice_create_gallery(tmpls, ids, &gallery) != JANICE_SUCCESS)
    // ERROR!
```

## janice_gallery_reserve {: #JaniceGalleryReserve }

Reserve space in a gallery for N templates.

#### Signature {: #JaniceGalleryReserveSignature }

```
JANICE_EXPORT JaniceError janice_gallery_reserve(size_t n);
```

#### Thread Safety {: #JaniceGalleryReserveThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceGalleryReserveParameters }

Name | Type   | Description
---- | ------ | -----------
n    | size_t | The number of templates to reserve space for


## janice_gallery_insert {: #JaniceGalleryInsert }

Insert a template into a gallery object. The template data should be copied
into the gallery as the template may be deleted after this function.

#### Signature {: #JaniceGalleryInsertSignature }

```
JANICE_EXPORT JaniceError janice_gallery_insert(JaniceGallery gallery,
                                                JaniceConstTemplate tmpl,
                                                JaniceTemplateId id);
```

#### Thread Safety {: #JaniceGalleryInsertThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceGalleryInsertParameters }

Name    | Type                                                                 | Description
------- | -------------------------------------------------------------------- | -----------
gallery | [JaniceGallery](typedefs.md#JaniceGallery)                           | A gallery object to insert the template into.
tmpl    | [JaniceConstTemplate](../enrollment/typedefs.md#JaniceConstTemplate) | A template object to insert into the gallery.
id      | [JaniceTemplateId](typedefs.md#JaniceTemplateId)                     | A unique id to associate with the input template

#### Example {: #JaniceGalleryInsertExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created
                     // previously
JaniceTemplateId id; // Where id is a unique integer to associate with tmpl. This
                     // integer should not exist in the gallery
JaniceGallery gallery; // Where gallery is a valid gallery object created
                       // previously

if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
    // ERROR!
```

## janice_gallery_remove {: #JaniceGalleryRemove }

Remove a template from a gallery object using its unique id.

#### Signature {: #JaniceGalleryRemoveSignature }

```
JANICE_EXPORT JaniceError janice_gallery_remove(JaniceGallery gallery,
                                                uint32_t id);
```

#### Thread Safety {: #JaniceGalleryRemoveThreadSafety }

This function is reentrant.

#### Parameters

Name    | Type                                             | Description
------- | ------------------------------------------------ | -----------
gallery | [JaniceGallery](typedefs.md#JaniceGallery)       | A gallery object to remove the template from
id      | [JaniceTemplateId](typedefs.md#JaniceTemplateId) | A unique id associated with a template in the gallery that indicates which template should be remove.

#### Example

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created
                     // previously
const JaniceTemplateId id = 0; // A unique integer id to associate with tmpl.

JaniceGallery gallery; // Where gallery is a valid gallery object created
                       // previously that does not have a template with id '0'
                       // already inserted in it.

// Insert the template with id 0
if (janice_gallery_insert(gallery, tmpl, id) != JANICE_SUCCESS)
    // ERROR!

// Now we can remove the template
if (janice_gallery_remove(gallery, id) != JANICE_SUCCESS)
    // ERROR!
```

## janice_gallery_prepare {: #JaniceGalleryPrepare }

Prepare a gallery for search. Implementors can use this function as an
opportunity to streamline gallery objects to accelerate the search process. The
calling convention for this function is **NOT** specified by the API, this
means that this function is not guaranteed to be called before
[janice_search](../comparison/functions.md#JaniceSearch). It also means that templates can be added
to a gallery before and after this function is called. Implementations should
handle all of these calling conventions. However, users should be aware that
this function may be computationally expensive. They should strive to call it
only at critical junctions before search and as few times as possible overall.

#### Signature {: #JaniceGalleryPrepareSignature }

```
JANICE_EXPORT JaniceError janice_gallery_prepare(JaniceGallery gallery);
```

#### Thread Safety {: #JaniceGalleryPrepareThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceGalleryPrepareParameters }

Name    | Type                                       | Description
------- | ------------------------------------------ | -----------
gallery | [JaniceGallery](typedefs.md#JaniceGallery) | A gallery object to prepare

#### Example {: #JaniceGalleryPrepareExample }

```
JaniceTemplate* tmpls; // Where tmpls is a valid array of valid template
                       // objects created previously
JaniceTemplateIds ids; // Where ids is a valid array of unique unsigned integers that
                       // is the same length as tmpls
JaniceTemplate tmpl; // Where tmpl is a valid template object created
                     // previously
JaniceTemplateId id; // Where id is a unique integer id to associate with tmpl.

JaniceGallery gallery = NULL; // best practice to initialize to NULL

if (janice_create_gallery(tmpls, ids, &gallery) != JANICE_SUCCESS)
    // ERROR!

// It is valid to run search without calling prepare
if (janice_search(tmpl, gallery ... ) != JANICE_SUCCESS)
    // ERROR!

// Prepare can be called after search
if (janice_gallery_prepare(gallery) != JANICE_SUCCESS)
    // ERROR!

// Search can be called again right after prepare
if (janice_search(tmpl, gallery ... ) != JANICE_SUCCESS)
    // ERROR!

// Insert another template into the gallery. This is valid after the gallery
// has been prepared
if (janice_gallery_insert(gallery, tmpl, 112) != JANICE_SUCCESS)
    // ERROR!

// Prepare the gallery again
if (janice_gallery_prepare(gallery) != JANICE_SUCCESS)
    // ERROR!
```

## janice_serialize_gallery {: #JaniceSerializeGallery }

Serialize a [JaniceGallery](typedefs.md#JaniceGallery) object to a flat buffer.

#### Signature {: #JaniceSerializeGallerySignature }

```
JANICE_EXPORT JaniceError janice_serialize_gallery(JaniceConstGallery gallery,
                                                   JaniceBuffer* data,
                                                   size_t* len);
```

#### Thread Safety {: #JaniceSerializeGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSerializeGalleryParameters }

Name    | Type                                                 | Description
------- | ---------------------------------------------------- | -----------
gallery | [JaniceConstGallery](typedefs.md#JaniceConstGallery) | A gallery object to serialize
data    | [JaniceBuffer](../io/typedefs.md#JaniceBuffer)       | An uninitialized buffer to hold the flattened data.
len     | size_t\*                                             | The length of the flat buffer after it is filled.

#### Example {: #JaniceSerializeGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery created
                       // previously.

JaniceBuffer buffer = NULL;
size_t buffer_len;
janice_serialize_gallery(gallery, &buffer, &buffer_len);
```

## janice_deserialize_gallery {: #JaniceDeserializeGallery }

Deserialize a [JaniceGallery](typedefs.md#JaniceGallery) object from a flat buffer.

#### Signature {: #JaniceDeserializeGallerySignature }

```
JANICE_EXPORT JaniceError janice_deserialize_gallery(const JaniceBuffer data,
                                                     size_t len,
                                                     JaniceGallery* gallery);
```

#### Thread Safety {: #JaniceDeserializeGalleryThreadSafety }

This function is reentrant.

#### Parameters

Name    | Type                                                 | Description
------- | ---------------------------------------------------- | ---------
data    | const [JaniceBuffer](../io/typedefs.md#JaniceBuffer) | A buffer containing data from a flattened gallery object.
len     | size_t                                               | The length of the flat buffer
gallery | [JaniceGallery\*](typedefs.md#JaniceGallery)         | An uninitialized gallery object.

#### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
unsigned char buffer[buffer_len];

FILE* file = fopen("serialized.gallery", "r");
fread(buffer, 1, buffer_len, file);

JaniceGallery gallery = NULL; // best practice to initialize to NULL
janice_deserialize_gallery(buffer, buffer_len, gallery);

fclose(file);
```

## janice_read_gallery {: #JaniceReadGallery }

Read a gallery from a file on disk. This method is functionally equivalent
to the following-

```
const size_t buffer_len = K; // Where K is the known length of the buffer
JaniceBuffer buffer[buffer_len];

FILE* file = fopen("serialized.gallery", "r");
fread(buffer, 1, buffer_len, file);

JaniceGallery gallery = NULL; // best practice to initialize to NULL
janice_deserialize_gallery(buffer, buffer_len, gallery);

fclose(file);
```

It is provided for memory efficiency and ease of use when reading from disk.

#### Signature {: #JaniceReadGallerySignature }

```
JANICE_EXPORT JaniceError janice_read_gallery(const char* filename,
                                              JaniceGallery* gallery);
```

#### Thread Safety {: #JaniceReadGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceReadGalleryParameters }

Name     | Type                                         | Description
-------- | -------------------------------------------- | -----------
filename | const char\*                                 | The path to a file on disk
gallery  | [JaniceGallery\*](typedefs.md#JaniceGallery) | An uninitialized gallery object.

#### Example {: #JaniceReadGalleryExample }

```
JaniceGallery gallery = NULL;
if (janice_read_gallery("example.gallery", &gallery) != JANICE_SUCCESS)
    // ERROR!
```

## janice_write_gallery {: #JaniceWriteGallery }

Write a gallery to a file on disk. This method is functionally equivalent
to the following-

```
JaniceGallery gallery; // Where gallery is a valid gallery created previously.

JaniceBuffer buffer = NULL;
size_t buffer_len;
janice_serialize_gallery(gallery, &buffer, &buffer_len);

FILE* file = fopen("serialized.gallery", "w+");
fwrite(buffer, 1, buffer_len, file);

fclose(file);
```

It is provided for memory efficiency and ease of use when writing to disk.

#### Signature {: #JaniceWriteGallerySignature }

```
JANICE_EXPORT JaniceError janice_write_gallery(JaniceConstGallery gallery,
                                               const char* filename);
```

#### ThreadSafety {: #JaniceWriteGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceWriteGalleryParameters }

Name     | Type                                                 | Description
-------- | ---------------------------------------------------- | -----------
gallery  | [JaniceConstGallery](typedefs.md#JaniceConstGallery) | The gallery object to write to disk
filename | const char\*                                         | The path to a file on disk

#### Example {: #JaniceWriteGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery created previously
if (janice_write_gallery(gallery, "example.gallery") != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_gallery {: #JaniceFreeGallery }

Free any memory associated with a [JaniceGalleryType](structs.md#JaniceGalleryType) object.

#### Signature {: #JaniceFreeGallerySignature }

```
JANICE_EXPORT JaniceError janice_free_gallery(JaniceGallery* gallery);
```

#### Thread Safety {: #JaniceFreeGalleryThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeGalleryParameters }

Name    | Type                                         | Description
------- | -------------------------------------------- | -----------
gallery | [JaniceGallery\*](typedefs.md#JaniceGallery) | A gallery object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeGalleryExample }

```
JaniceGallery gallery; // Where gallery is a valid gallery object created previously
if (janice_free_gallery(&gallery) != JANICE_SUCCESS)
    // ERROR!
```
