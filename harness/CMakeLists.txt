set(JANICE_WITH_EVAL_HARNESS ON CACHE BOOL "Compile evaluation library/utilities")

if (${JANICE_WITH_EVAL_HARNESS} AND (JANICE_IO_IMPLEMENTATION) AND (JANICE_IMPLEMENTATION))
  # Use C++11
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  # Include paths. By default we include the opencv_io implementation
  include_directories(../implementations/opencv_io)
  include_directories(include)

  set(HARNESS_SOURCES
      janice_create_gallery.cpp
      janice_detect.cpp
      janice_enroll_media.cpp
      janice_enroll_detections.cpp
      janice_search.cpp
      #janice_cluster_templates.cpp
      #janice_cluster_media.cpp
      janice_verify.cpp
      )

  # Loop over the utils, rather than having a separate rule for each.
  # If caller has defined JANICE_EVAL_HARNESS_INSTALL, add an install step for each one.
  foreach(HARNESS ${HARNESS_SOURCES})
    # get the name to use for the executable
    get_filename_component(UTIL_NAME ${HARNESS} NAME_WE)
    add_executable(${UTIL_NAME} ${HARNESS})
    target_link_libraries(${UTIL_NAME} ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})
    # If this is defined, it's an installation path.
    if (DEFINED JANICE_EVAL_HARNESS_INSTALL)
      install(TARGETS ${UTIL_NAME} RUNTIME DESTINATION ${JANICE_EVAL_HARNESS_INSTALL})
    endif()
  endforeach()
else()
  message("Skipping JanICE harness executables: JANICE_WITH_EVAL_HARNESS=${JANICE_WITH_EVAL_HARNESS}; JANICE_IO_IMPLEMENTATION=${JANICE_IO_IMPLEMENTATION}; JANICE_IMPLEMENTATION=${JANICE_IMPLEMENTATION}")
endif()
