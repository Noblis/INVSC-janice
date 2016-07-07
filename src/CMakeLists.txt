include_directories(BEFORE ../include)
file(GLOB JANICE_HEADERS ../include/*.h)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(JANICE_BUILD_OPENCV_WRAPPER "Build JanICE I/O implementation using OpenCV" OFF)
if(${JANICE_BUILD_OPENCV_WRAPPER})
  find_package(OpenCV)
  if(OpenCV_FOUND)
    set(${JANICE_IO_IMPLEMENTATION} opencv_io)
    add_subdirectory(opencv_io)
  endif()
endif()

# JanICE API documentation
add_subdirectory(doxygen)

# JanICE command line utilities
add_subdirectory(utils)
