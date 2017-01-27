## janice_error_to_string {: #JaniceErrorToString }

Convert an [error code](enums.md#JaniceError) into a string for printing.

### Signature {: #JaniceErrorToStringSignature }
```
JANICE_EXPORT const char* janice_error_to_string(JaniceError error);
```

### Thread Safety {: #JaniceErrorToStringThreadSafety }

This function is thread safe.

### Parameters {: #JaniceErrorToStringParameters }

Name  | Type                                | Description
----- | ----------------------------------- | -----------
error | [JaniceError](enums.md#JaniceError) | An error code

### Return Value {: #JaniceErrorToStringReturnValue }

This is the only function in the API that does not return
[JaniceError](enums.md#JaniceError). It returns <code>const char\*</code> which is a
null-terminated list of characters that describe the input error.
