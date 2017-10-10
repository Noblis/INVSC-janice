# JanICE API

## Overview

The JanICE API is a *C* API that provides a common interface between computer 
vision algorithms and agencies and entities that would like to use them.

## Building

The API is header only by design and therefore cannot be "built". However, a
build protocol designed around the [CMake](https://cmake.org) program is provided
as a default way to build implementations. Additionally, a small test harness
and associated command line utilties are included that provide examples to
users of best practices for using the API.

The protocol is quite simple. 2 variables give the paths to the implementation and
I/O implementation libraries. The implementation library is specified by
<code>JANICE_IMPLEMENTATION</code> and the I/O library by <code>JANICE_IO_IMPLEMENTATION</code>.
Each variable should give the full path to the respective library and can be specified
to cmake using the -D option. For example, an implementation that has all functionality in
a library <code>fake_janice_implementation.so</code> and all I/O functionality in a library
<code>fake_janice_io_implementation.so</code> could be linked against using
the following command:

```
$cmake -DJANICE_IMPLEMENTATION=/path/to/fake_janice_implementation.so \
       -DJANICE_IO_IMPLEMENTATION=/path/to/fake_janice_io_implementation.so ..
```

The .. at the end gives a relative path to the root of this repository. The test
harness and command line executables automatically check if 
<code>JANICE_IMPLEMENTATION</code> is defined and will build if it is not an 
empty string.

## License

This project is licensed under the MIT license and is free for academic and 
commercial use.

## Documentation

The documentation for the API is hosted [here](https://noblis.github.io/janice/).
The complete documentation for the API is available in "docs" folder. A PDF file
is provided for convienience. Alternatively, you can build the docs yourself with
the [Sphinx documentation generator](https://sphinx-doc.org).

## Contributing

Contributions are generally welcome. Please open a pull request.
