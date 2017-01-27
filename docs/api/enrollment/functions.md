## janice_create_template {: #JaniceCreateTemplate }

Create a [JaniceTemplate](typedefs.md#JaniceTemplate) object from an array of detections.

#### Signature {: #JaniceCreateTemplateSignature }

```
JANICE_EXPORT JaniceError janice_create_template(JaniceConstDetections detections,
                                                 uint32_t num_detections,
                                                 JaniceEnrollmentType role,
                                                 JaniceTemplate* tmpl);
```

#### Thread Safety {: #JaniceCreateTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceCreateTemplateParameters }

Name           | Type                                                                    | Description
-------------- | ----------------------------------------------------------------------- | -----------
detections     | [JaniceConstDetections](../detection/typedefs.md#JaniceConstDetections) | An array of detection objects
num_detections | uint32_t                                                                | The number of input detections
role           | [JaniceEnrollmentType](enums.md#JaniceEnrollmentType)                   | The use case for the template
tmpl           | [JaniceTemplate\*](typedefs.md#JaniceTemplate)                          | An uninitialized template object.

#### Example {: #JaniceCreateTemplateExample }

```
JaniceDetections detections; // Where detections is a valid array of valid
                             // detection objects created previously
const uint32_t num_detections = K; // Where K is the number of detections in
                                   // the detections array
JaniceEnrollmentType role = Janice1NProbe; // This template will be used as a
                                           // probe in 1-N search
JaniceTemplate tmpl = NULL; // Best practice to initialize to NULL

if (janice_create_template(detections, num_detections, rolw, &tmpl) != JANICE_SUCCESS)
    // ERROR!
```

## janice_template_get_attribute {: #JaniceTemplateGetAttribute }

Get a metadata value from a template using a key string. The valid set of keys
is determined by the implementation and must be included in their delivered
documentation. The possible return values for a valid key are also
implementation specific. Invalid keys should return an error.

#### Signature {: #JaniceTemplateGetAttributeSignature }

```
JANICE_EXPORT JaniceError janice_template_get_attribute(JaniceConstTemplate tmpl,
                                                        const char* attribute,
                                                        char** value);
```                                                     

#### Thread Safety {: #JaniceTemplateGetAttributeThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceTemplateGetAttributeParameters }

Name      | Type                                                   | Description
--------- | ------------------------------------------------------ | -----------
tmpl      | [JaniceConstTemplate](typedefs.md#JaniceConstTemplate) | A template object to query the attribute from
attribute | const char\*                                           | The name of the attribute to query
value     | char\*\*                                               | An uninitialized pointer to hold the attribute value. The return value must be null-terminated.

## janice_serialize_template {: #JaniceSerializeTemplate }

Serialize a [JaniceTemplate](typedefs.md#JaniceTemplate) object to a flat buffer.

#### Signature {: #JaniceSerializeTemplateSignature }

```
JANICE_EXPORT JaniceError janice_serialize_template(JaniceConstTemplate tmpl,
                                                    JaniceBuffer* data,
                                                    size_t* len);
```

#### Thread Safety {: #JaniceSerializeTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSerializeTemplateParameters }

Name | Type                                                   | Description
---- | ------------------------------------------------------ | -----------
tmpl | [JaniceConstTemplate](typedefs.md#JaniceConstTemplate) | A template object to serialize
data | [JaniceBuffer\*](../io/typedefs.md#JaniceBuffer)       | An uninitialized buffer to hold the flattened data.
len  | size_t\*                                               | The length of the flat buffer after it is filled.

#### Example {: #JaniceSerializeTemplateExample }

```

JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously.

JaniceBuffer buffer = NULL;
size_t buffer_len;
janice_serialize_template(tmpl, &buffer, &buffer_len);
```

## janice_deserialize_template {: #JaniceDeserializeTemplate }

Deserialize a [JaniceTemplate](typedefs.md#JaniceTemplate) object from a flat buffer.

#### Signature {: #JaniceDeserializeTemplateSignature }

```
JANICE_EXPORT JaniceError janice_deserialize_template(const JaniceBuffer data,
                                                      size_t len,
                                                      JaniceTemplate* tmpl);
```

#### Thread Safety {: #JaniceDeserializeTemplateThreadSafety }

This function is reentrant.

#### Parameters

Name | Type                                                 | Description
---- | ---------------------------------------------------- | -----------
data | const [JaniceBuffer](../io/typedefs.md#JaniceBuffer) | A buffer containing data from a flattened template object.
len  | size_t                                               | The length of the flat buffer
tmpl | [JaniceTemplate\*](typedefs.md#JaniceTemplate)       | An uninitialized template object.

#### Example

```
const size_t buffer_len = K; // Where K is the known length of the buffer
JaniceBuffer buffer[buffer_len];

FILE* file = fopen("serialized.template", "r");
fread(buffer, 1, buffer_len, file);

JaniceTemplate tmpl = NULL; // best practice to initialize to NULL
janice_deserialize_template(buffer, buffer_len, tmpl);

fclose(file);
```

## janice_read_template {: #JaniceReadTemplate }

Read a template from a file on disk. This method is functionally equivalent
to the following-

```
const size_t buffer_len = K; // Where K is the known length of the buffer
JaniceBuffer buffer[buffer_len];

FILE* file = fopen("serialized.template", "r");
fread(buffer, 1, buffer_len, file);

JaniceTemplate tmpl = nullptr;
janice_deserialize_template(buffer, buffer_len, tmpl);

fclose(file);
```

It is provided for memory efficiency and ease of use when reading from disk.

#### Signature {: #JaniceReadTemplateSignature }

```
JANICE_EXPORT JaniceError janice_read_template(const char* filename,
                                               JaniceTemplate* tmpl);
```

#### Thread Safety {: #JaniceReadTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceReadTemplateParameters }

Name     | Type                                           | Description
-------- | ---------------------------------------------- | -----------
filename | const char\*                                   | The path to a file on disk
tmpl     | [JaniceTemplate\*](typedefs.md#JaniceTemplate) | An uninitialized template object.

#### Example {: #JaniceReadTemplateExample }

```
JaniceTemplate tmpl = NULL;
if (janice_read_template("example.template", &tmpl) != JANICE_SUCCESS)
    // ERROR!
```

## janice_write_template {: #JaniceWriteTemplate }

Write a template to a file on disk. This method is functionally equivalent
to the following-

```

JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously.

JaniceBuffer buffer = NULL;
size_t buffer_len;
janice_serialize_template(tmpl, &buffer, &buffer_len);

FILE* file = fopen("serialized.template", "w+");
fwrite(buffer, 1, buffer_len, file);

fclose(file);
```

It is provided for memory efficiency and ease of use when writing to disk.

#### Signature {: #JaniceWriteTemplateSignature }

```
JANICE_EXPORT JaniceError janice_write_template(JaniceConstTemplate tmpl,
                                                const char* filename);
```

#### ThreadSafety {: #JaniceWriteTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceWriteTemplateParameters }

Name     | Type                                                   | Description
-------- | ------------------------------------------------------ | -----------
tmpl     | [JaniceConstTemplate](typedefs.md#JaniceConstTemplate) | The template object to write to disk
filename | const char\*                                           | The path to a file on disk

#### Example {: #JaniceWriteTemplateExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template created
                     // previously
if (janice_write_template(tmpl, "example.template") != JANICE_SUCCESS)
    // ERROR!
```

## janice_free_template {: #JaniceFreeTemplate }

Free any memory associated with a [JaniceTemplateType](structs.md#JaniceTemplateType) object.

#### Signature {: #JaniceFreeTemplateSignature }

```
JANICE_EXPORT JaniceError janice_free_template(JaniceTemplate* tmpl);
```

#### Thread Safety {: #JaniceFreeTemplateThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeTemplateParameters }

Name | Type                                              | Description
---- | ------------------------------------------------- | -----------
tmpl | [JaniceTemplate\*](structs.md#JaniceTemplateType) | A template object to free. Best practice dictates the pointer should be set to <code>NULL</code> after it is freed.

#### Example {: #JaniceFreeTemplateExample }

```
JaniceTemplate tmpl; // Where tmpl is a valid template object created previously
if (janice_free_template(&tmpl) != JANICE_SUCCESS)
    // ERROR!
```
