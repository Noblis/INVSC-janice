## janice_train {: #JaniceTrain }

Train an implementation using new data.

#### Signature {: #JaniceTrainSignature }

```
JANICE_EXPORT JaniceError janice_train(const char* data_prefix,
                                       const char* data_list);
```

#### Thread Safety {: #JaniceTrainThreadSafety }

This function is thread unsafe.

#### Parameters {: #JaniceTrainParameters }

Name        | Type         | Description
----------- | ------------ | -----------
data_prefix | const char\* | A prefix path pointing to the location of training data
data_train  | const char\* | A list of training data and labels. The format is currently unspecified

#### Notes {: #JaniceTrainNotes }

This function is untested, unstable and most likely subject to breaking changes
in future releases.
