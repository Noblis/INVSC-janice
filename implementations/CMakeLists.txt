option(JANICE_WITH_OPENCV_IO "Use the default I/O routines built with OpenCV" OFF)
if (${JANICE_WITH_OPENCV_IO})
  add_subdirectory(opencv_io)
  set(JANICE_IO_IMPLEMENTATION ${JANICE_IO_IMPLEMENTATION} janice_io_opencv)
  #set(JANICE_IO_IMPLEMENTATION "janice_io_opencv" CACHE STRING "Use the provided OpenCV I/O Library" FORCE)
endif()

option(JANICE_WITH_MEMORY_IO "Use the default I/O routines for in-memory operations" OFF)
if (${JANICE_WITH_MEMORY_IO})
  add_subdirectory(memory_io)
  set(JANICE_IO_IMPLEMENTATION ${JANICE_IO_IMPLEMENTATION} janice_io_memory)
  #set(JANICE_IO_IMPLEMENTATION "janice_io_opencv" CACHE STRING "Use the provided OpenCV I/O Library" FORCE)
endif()

