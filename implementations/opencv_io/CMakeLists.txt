# Build the opencv_io library. This library implements the interfaces in janice_io.h
# To use it simply link it into your project

find_package(OpenCV 3.0 REQUIRED)
set(OpenCV_LIBS opencv_core opencv_highgui opencv_videoio opencv_imgcodecs)

add_library(opencv_io SHARED opencv_io.cpp)
set_target_properties(opencv_io PROPERTIES
                      DEFINE_SYMBOL JANICE_LIBRARY
                      VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                      SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(opencv_io ${OpenCV_LIBS}) 

install(TARGETS opencv_io RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)

# Optionally, build unit tests
if (${BUILD_TESTING})
  add_subdirectory(test)
endif()
