option(JANICE_WITH_OPENCV_IO "Use the default I/O routines built with OpenCV" OFF)
if (${JANICE_WITH_OPENCV_IO})
  add_subdirectory(opencv_io)
  set(JANICE_IO_IMPLEMENTATION "opencv_io" CACHE STRING "Use the provided OpenCV I/O Library" FORCE)
endif()

