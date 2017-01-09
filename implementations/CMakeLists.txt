option(JANICE_WITH_OPENCV_IO "Use the default I/O routines built with OpenCV" OFF)
if (${JANICE_WITH_OPENCV_IO})
  include(opencv_io/opencv_io.cmake)
endif()

