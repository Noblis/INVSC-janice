# Build the janice_io_opencv library. This library implements methods to create media iterators
# To use it simply link it into your project

if (NOT OpenCV_FOUND) 
  find_package(OpenCV 3 REQUIRED)
endif(NOT OpenCV_FOUND)

set(OpenCV_LIBS opencv_core opencv_highgui opencv_videoio opencv_imgcodecs)

include_directories(.)
include_directories(../../api/)

message("Got libs ${OpenCV_LIBS}")
message("Got path ${OpenCV_DIR}")


add_library(janice_io_opencv SHARED janice_io_opencv.cpp)
set_target_properties(janice_io_opencv PROPERTIES
                                       DEFINE_SYMBOL JANICE_LIBRARY
                                       VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                                       SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(janice_io_opencv ${OpenCV_LIBS})

install(TARGETS janice_io_opencv RUNTIME DESTINATION bin
                                 LIBRARY DESTINATION lib
                                 ARCHIVE DESTINATION lib)

install(FILES janice_io_opencv.h DESTINATION include/janice)

# Optionally, build unit tests
if (${BUILD_TESTING})
  add_subdirectory(test)
endif()
