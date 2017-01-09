## Objects {: #Objects }

This API is built around opaque object pointers that represent high-level
computer vision concepts. To provide the most flexibility to implementors
of this API these objects are not defined in the API and should instead
be defined in the implementation. Access to these object is regulated
through defined accessor functions that implementors are either required
or optional as indicated in their documentation below.

## Media {: #Media }

This API seeks to define a common representation for both images and videos,
which we call media. Implementors of this API are free to define media however
they like using the [JaniceMediaType](objects.md#JaniceMediaType) structure. However, it is
strongly recommended that the implementation is lazy when loading videos into
memory. It is expected that this API will load multiple videos at once for
processing, for example during a call to
[janice_cluster_media](functions.md#JaniceClusterMedia), which can overload system memory
if all frames are loaded at once. An example implementation has been provided
[here](https://github.com/Noblis/janice/implementations/opencv_io/opencv_io.h) that uses the popular OpenCV
library.

## Detection {: #Detection }

In the context of this API, detection is used to refer to the identification of
objects of interest within a [media](#Media) object. Detections are represented
using the [JaniceDetectionType](objects.md#JaniceDetectionType) object which implementors
are free to define however they would like. For images, a detection is defined
as a rectangle that bounds an object of interest and an associated confidence
value. For video, a single object can exist in multiple frames. A rectangle and
confidence are only relevant in a single frame. In this case, we define a
detection as a list of (rectangle, confidence) pairs that track a single object
through a video. It is not required that this list be dense however (i.e.
frames can be skipped). To support this, we extend our representation of a
detection to a (rectangle,confidence,frame) tuple where frame gives the index
of the frame the rectangle was found in.

## Feature Extraction {: #FeatureExtraction }

This API defines feature extraction as the process of turning 1 or more
[detections](#Detection) that refer to the same object into a single
representation. This representation is defined in the API using the
[JaniceTemplateType(#JaniceTemplateType) object. In some cases (e.g. face
recognition) this model of [multiple detections] -> [single representation]
contradicts the current paradigm of [single detection] -> [single
representation]. Implementors are free to implement whatever paradigm they
choose internally (i.e. a JanICE template could be a simple list of single
detection templates) provided the [comparison](#Comparison) functions work
appropriately.

## Galleries {: #Galleries }

This API defines a gallery object that represents a collection of templates.
Galleries are useful in the 1-N use case (see
[the section on comparison](#Comparison)) when a user would like to query an
unknown probe template against a set of known identities. A naive
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

## Comparison {: #Comparison }

This API defines two possible types of comparisons, 1-1 and 1-many or 1-N.
These are represented by the [janice_verify](functions.md#JaniceVerify) and
[janice_search](functions.md#JaniceSearch) functions respectively. The API
quantified the relationship between two templates as a single number called a
similarity score. The definition of a similarity score in the context of this
API is discussed [here](functions.md#JaniceVerifySimilarityScore).

## Clustering {: #Clustering }

This API defines clustering is the automatic and unsupervised combination of unlabelled templates into groups of like templates. What constitutes
likeness is heavily dependent on the use case and context in question. One
example when dealing with faces is grouping based on identity, where all faces
belonging to a single individual are placed in a cluster.
