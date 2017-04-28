## janice_initialize {: #JaniceInitialize }

Initialize global or shared state for the implementation. This function should
be called once at the start of the application, before making any other calls
to the API.

#### Signature {: #JaniceInitializeSignature }

```
JANICE_EXPORT JaniceError janice_initialize(const char* sdk_path,
                                            const char* temp_path,
                                            const char* algorithm,
                                            const int num_threads,
                                            const int* gpus,
                                            const int num_gpus);
```

#### Thread Safety {: #JaniceInitializeThreadSafety }

This function is thread unsafe.

#### Parameters {: #JaniceInitializeParameters }

Name        | Type         | Description
----------- | ------------ | -----------
sdk_path    | const char\* | Path to a *read-only* directory containing the JanICE compliant SDK as specified by the implementor
temp_path   | const char\* | Path to an existing empty *read-write* directory for use as temporary file storage by the implementation. This path must be guaranteed until [janice_finalize](#JaniceFinalize).
algorithm   | const char\* | An empty string indicating the a default algorithm, or an implementation defined string containing an alternative configuration
num_threads | const int    | The number of threads the implementation is allowed to use. A value of '-1' indicates that the implementation can use all available hardware.
gpus        | const int\*  | A list of indices of GPUs available to the implementation. The length of the list is given by *num_gpus*. If implementors do not offer a GPU solution they can ignore this value.
num_gpus    | const int    | The length of the *gpus* array. If no GPUs are available this should be set to 0.

## janice_api_version {: #JaniceAPIVersion }

Query the implementation for the version of the JanICE API it was designed to
implement. See
[the section on software versioning](../software_concepts.md#Versioning) for
more information on the versioning process for this API.

#### Signature {: #JaniceAPIVersionSignature }

```
JANICE_EXPORT JaniceError janice_api_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);
```

#### Thread Safety {: #JaniceAPIVersionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceAPIVersionParameters }

Name  | Type       | Description
----- | ---------- | -----------
major | uint32_t\* | The supported major version of the API
minor | uint32_t\* | The supported minor version of the API
patch | uint32_t\* | The supported patch version of the API

## janice_sdk_version {: #JaniceSDKVersion }

Query the implementation for its SDK version.

#### Signature {: #JaniceSDKVersionSignature }

```
JANICE_EXPORT JaniceError janice_sdk_version(uint32_t* major,
                                             uint32_t* minor,
                                             uint32_t* patch);
```

#### Thread Safety {: #JaniceSDKVersionThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSDKVersionParameters }

Name  | Type       | Description
----- | ---------- | -----------
major | uint32_t\* | The major version of the SDK
minor | uint32_t\* | The minor version of the SDK
patch | uint32_t\* | The patch version of the SDK

## janice_finalize {: #JaniceFinalize }

Destroy any resources created by [janice_initialize](#JaniceInitialize) and
finalize the application. This should be called once after all other API calls.

#### Signature {: #JaniceFinalizeSignature }

```
JANICE_EXPORT JaniceError janice_finalize();
```

#### Thread Safety {: #JaniceFinalizeThreadSafety }

This function is thread unsafe.
