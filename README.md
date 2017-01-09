# JanICE API

## Welcome

The JanICE API is a *C* API that provides a common interface between computer 
vision algorithms and agencies and entities that would like to use them.

## Building

The API is header only by design and therefore cannot be "built". However, a
build protocol built around the [CMake](https://cmake.org) program is provided
as a default way to build implementations. Additionally, a small test harness
and associated command line utilties are included that provide examples to
users of best practices for using the API.

The protocol is quite simple. A single variable <code>JANICE_IMPLEMENTATION</code>
that gives the full path to the implementation library should be passed to cmake
using the -D option. For example, an implementation that has all functionality in
a library <code>fake_janice_implementation.so</code> could be linked against using
the following command:

```
$cmake -DJANICE_IMPLEMENTATION=/path/to/fake_janice_implementation.so ..
```

The .. at the end gives a relative path to the root of this repository. The test
harness and command line executables automatically check if 
<code>JANICE_IMPLEMENTATION</code> is defined and will build if it is not an 
empty string.

## License

This project is licensed under the MIT license and is free for academic and 
commercial use.

## Documentation

The complete documentation for the API is available in "docs" folder. It is
designed to be built with the open source [mkdocs](http://www.mkdocs.org)
project. Alternatively, the documentation can be viewed in any viewer that can
parse [Github Flavored Markdown](https://guides.github.com/features/mastering-markdown/).

## Contributing

Contributions are generally welcome. Please open a pull request.
