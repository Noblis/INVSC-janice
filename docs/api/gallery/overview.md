This API defines a gallery object that represents a collection of templates.
Galleries are useful in the 1-N use case (see
[the section on comparison](../comparison/overview.md)) when a user would like
to query an unknown probe template against a set of known identities. A naive
implementation of a gallery might be a simple array of templates. Often
however, implementations have optimized algorithms or data structures that can
lead to more efficient search times. It is recommended that advanced data
structures be implemented as part of a gallery. Please note however the rules
on gallery modification:

1. Gallery objects may be modified (templates inserted or removed) at any time.
2. It is understood that some preprocessing might need to be done between
   gallery modification and efficient search. A function
   [janice_prepare_gallery](functions.md#JanicePrepareGallery) exists for this
   purpose. The calling of this function however is optional. Please see the
   [documentation](functions.md#JanicePrepareGallery) for more information.
