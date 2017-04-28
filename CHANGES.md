# 4.0 -> 4.1 Changes

* Added explicit support for the number of threads an implementation can use
* Added support for an implementation using multiple GPUs
* Changed the I/O interface-
    * Main I/O class is now JaniceMediaIterator
    * JaniceMediaIterator is now a fully defined class that relies on C-style function pointers. The advantage of being fully defined is that JANICE_IMPLEMENTATION does not need to link against JANICE_IO_IMPLEMENTATION.
    * OpenCV I/O has been refactored to work with the new interface
    * The memory layout for JaniceImage is now required to be row -> column -> channel.
* A new function janice_gallery_reserve to reserve space in a gallery. This can increase efficiency when adding batches of templates to a gallery.
* A new function janice_free_buffer to release memory allocated during object serialization
* Documentation updates to reflect the new changes
