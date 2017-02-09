## janice_verify {: #JaniceVerify }

Compare two templates with the difference expressed as a similarity score.

#### Signature {: #JaniceVerifySignature }

```
JANICE_EXPORT JaniceError janice_verify(JaniceConstTemplate reference,
                                        JaniceConstTemplate verification,
                                        JaniceSimilarity* similarity);
```

#### Thread Safety {: #JaniceVerifyThreadSafety }

This function is reentrant.

#### Similarity Score {: #JaniceVerifySimilarityScore }

This API expects that the comparison of two templates results in a single value
that quantifies the similarity between them. A similarity score is constrained
by the following requirements:

    1. Higher scores indicate greater similarity
    2. Scores can be asymmetric. This mean verify(a, b) does not necessarily
       equal verify(b, a)

#### Parameters {: #JaniceVerifyParameters }

Name         | Type                                                 | Description
------------ | ---------------------------------------------------- | -----------
reference    | [JaniceConstTemplate](../enrollment/typedefs.md#JaniceConstTemplate) | A reference template. This template was created with the [Janice11Reference](../enrollment/enums.md#JaniceEnrollmentType) role.
verification | [JaniceConstTemplate](../enrollment/typedefs.md#JaniceConstTemplate) | A verification template. This template was created with the [Janice11Verification](../enrollment/enums.md#JaniceEnrollmentType) role.
similarity   | [JaniceSimilarity](typedefs.md#JaniceSimilarity)\*                   | A similarity score. See [this section](#JaniceVerifySimilarityScore) for more information.

#### Example {: #JaniceVerifyExample }

```
JaniceTemplate reference; // Where reference is a valid template object created
                          // previously
JaniceTemplate verification; // Where verification is a valid template object
                             // created previously
JaniceSimilarity similarity;
if (janice_verify(reference, verification, &similarity) != JANICE_SUCCESS)
    // ERROR!
```

## janice_search {: #JaniceSearch }

Compute 1-N search results between a query template object and a target gallery
object. The function allocates two arrays of equal size, one containing
[similarity scores](#JaniceVerifySimilarityScore) and the other containing the
unique id of the template the score was computed with (along with the query).
Often it is desirable (and perhaps computationally efficient) to only see the
top K scores out of N possible templates. The option to set a K is provided to
the user as part of the function parameters.

#### Signature {: #JaniceSearchSignature }

```
JANICE_EXPORT JaniceError janice_search(JaniceConstTemplate probe,
                                        JaniceConstGallery gallery,
                                        uint32_t num_requested,
                                        JaniceSimilarities* similarities,
                                        JaniceSearchTemplateIds* ids,
                                        uint32_t* num_returned);
```

#### Thread Safety {: #JaniceSearchThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceSearchParameters }

Name          | Type                                                                 | Description
------------- | -------------------------------------------------------------------- | -----------
probe         | [JaniceConstTemplate](../enrollment/typedefs.md#JaniceConstTemplate) | A template object to use as a query
gallery       | [JaniceConstGallery](../gallery/typedefs.md#JaniceConstGallery)      | A gallery object to search against
num_requested | uint32_t                                                             | The number of requested returns. If the user would like as many returns as there are templates in the gallery they can pass this parameter with the value 0.
similarities  | [JaniceSimilarities\*](typedefs.md#JaniceSimilarities)               | An array of [similarity scores](#JaniceVerifySimilarityScore). The scores are expected to be sorted in descending order (i.e. the highest scores is stored at index 0).
ids           | [JaniceSearchTemplateIds\*](typedefs.md#JaniceSearchTemplateIds)     | An array of unique ids identifying the target templates associated with the similarity scores. This array must be the same size as the similarities array. The *ith* id corresponds with the *ith* similarity.
num_returned  | uint32_t\*                                                           | The number of elements in the similarities and ids arrays. This number can be different from **num_requested**.

#### Example {: #JaniceSearchExample }

```
JaniceTemplate probe;  // Where probe is a valid template object created
                       // previously
JaniceGallery gallery; // Where gallery is a valid gallery object created
                       // previously
const uint32_t num_requested = 50; // Request the top 50 matches

JaniceSimilarities similarities = NULL;
JaniceSearchTemplateIds ids = NULL;
uint32_t num_returned;

// Run search
if (janice_search(probe, gallery, num_requested, &similarities, &ids, &num_returned) != JANICE_SUCCESS)
    // ERROR!

num_requested == num_returned; // This might not be true!
```

## janice_free_similarities {: #JaniceFreeSimilarities }

Free any memory associated with a [JaniceSimilarities](typedefs.md#JaniceSimilarities) object.

#### Signature {: #JaniceFreeSimilaritiesSignature }

```
JANICE_EXPORT JaniceError janice_free_similarities(JaniceSimilarities* similarities);
```

#### Thread Safety {: #JaniceFreeSimilaritiesThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeSimilaritiesParameters }

Name         | Type                                                   | Description
------------ | ------------------------------------------------------ | -----------
similarities | [JaniceSimilarities\*](typedefs.md#JaniceSimilarities) | An array of similarities to free.

## janice_free_search_ids {: #JaniceFreeSearchIds }

Free any memory associated with a [JaniceSearchTemplateIds](typedefs.md#JaniceSearchTemplateIds) object.

#### Signature {: #JaniceFreeSearchIdsSignature }

```
JANICE_EXPORT JaniceError janice_free_search_ids(JaniceSearchTemplateIds* ids);
```

#### Thread Safety {: #JaniceFreeSearchIdsThreadSafety }

This function is reentrant.

#### Parameters {: #JaniceFreeSearchIdsParamters }

Name | Type                                                             | Description
---- | ---------------------------------------------------------------- | -----------
ids  | [JaniceSearchTemplateIds\*](typedefs.md#JaniceSearchTemplateIds) | An array of ids to free.
