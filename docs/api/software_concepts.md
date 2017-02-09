## Error Handling {: #ErrorHandling }

The API handles errors using return codes. Valid return codes are defined
[here](#JaniceError). In general, it is assumed that new memory is only
allocated if a function returns <code>JANICE_SUCCESS</code>. Therefore,
**implementors are REQUIRED to deallocate any memory allocated during a
function call if that function returns an error.**

## Memory Allocation {: #MemoryAllocation }

The API often passes unallocated pointers to functions for the
implementor to allocate appropriately. This is indicated if the type of a
function input is <code>JaniceObject\*\*</code>, or in the case of a utility
<code>typedef</code> <code>JaniceTypedef\*</code>. It is considered a best
practice for unallocated pointers to be initialized to <code>NULL</code> before
they are passed to a function, but this is not guaranteed. It is the
responsibility of the users of the API to ensure that pointers do not point to
valid data before they are passed to functions in which they are modified, as
this would cause memory leaks.

## Thread Safety {: #ThreadSafety }
All functions are marked one of:

Type          | Description
------------- | -----------
Thread safe   | Can be called simultaneously from multiple threads, even when the invocations use shared data.
Reentrant     | Can be called simultaneously from multiple threads, but only if each invocation uses its own data.
Thread unsafe | Can not be called simultaneously from multiple threads.

## Compiling {: #Compiling }
Define <code>JANICE_LIBRARY</code> during compilation to export JanICE symbols.

## Versioning {: #Versioning }

This API follows the [semantic versioning](http://semver.org) paradigm. Each
released iteration is tagged with a major.minor.patch version. A change in
the major version indicates a breaking change. A change in the minor version
indicates a backwards-compatible change. A change in the patch version
indicates a backwards-compatible bug fix.
