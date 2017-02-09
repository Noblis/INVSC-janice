## JaniceEnrollmentType {: #JaniceEnrollmentType }
Often times, the templates produced by algorithms will require different data for different use cases. The enrollment type indicates what the use case for the created template will be, allowing implementors to specialize their templates if they so desire. The use cases supported by the API are:

Type                              | Description
--------------------------------- | -----------
<code>Janice11Reference</code>    | The template will be used as a reference template for 1 to 1 verification.
<code>Janice11Verification</code> | The template will be used for verification against a reference template in 1 to 1 verification.
<code>Janice1NProbe</code>        | The template will be used as a probe in 1 to N search.
<code>Janice1NGallery</code>      | The template will be enrolled into a gallery and searched against for 1 to N search.
<code>JaniceCluster</code>        | The template will be used for clustering
