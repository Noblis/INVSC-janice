include_directories(BEFORE ../include)
file(GLOB JANICE_HEADERS ../include/*.h)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(JANICE_BUILD_PP5_WRAPPER "Build JanICE implementation using PittPatt 5" OFF)
if(${JANICE_BUILD_PP5_WRAPPER})
  find_package(PP5 REQUIRED)
  add_subdirectory(pittpatt)
endif()

option(JANICE_BUILD_OPENCV_WRAPPER "Build JanICE I/O implementation using OpenCV" OFF)
if(${JANICE_BUILD_OPENCV_WRAPPER})
  #if (${JANICE_BUILD_PP5_WRAPPER})
  #  message(FATAL_ERROR "The OpenCV and PP5 wrappers cannot be built together.")
  #endif()

  find_package(OpenCV)
  if(OpenCV_FOUND)
    add_subdirectory(opencv_io)
  endif()
endif()

# JanICE API documentation
add_subdirectory(doxygen)

# JanICE command line utilities
add_subdirectory(utils)
