# Build the janice_io_memory library. This library implements methods to create media iterators
# from in-memory buffers

include_directories(.)
include_directories(../../api/)

add_library(janice_io_memory SHARED janice_io_memory.cpp janice_io_memory_sparse.cpp)
set_target_properties(janice_io_memory PROPERTIES
                                       DEFINE_SYMBOL JANICE_LIBRARY
                                       VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                                       SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(janice_io_memory ${OpenCV_LIBS})

install(TARGETS janice_io_memory RUNTIME DESTINATION bin
                                 LIBRARY DESTINATION lib
                                 ARCHIVE DESTINATION lib)

install(FILES janice_io_memory.h DESTINATION include/janice)
