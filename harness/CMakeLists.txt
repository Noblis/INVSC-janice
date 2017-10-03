set(JANICE_WITH_EVAL_HARNESS ON CACHE BOOL "Compile evaluation library/utilities")

if (${JANICE_WITH_EVAL_HARNESS} AND (JANICE_IO_IMPLEMENTATION) AND (JANICE_IMPLEMENTATION))
  # Use C++11
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  # Include paths. By default we include the opencv_io implementation
  include_directories(../implementations/opencv_io)
  include_directories(include)

  # Detection
  add_executable(janice_detect janice_detect.cpp)
  target_link_libraries(janice_detect ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  # Enroll from media
  add_executable(janice_enroll_media janice_enroll_media.cpp)
  target_link_libraries(janice_enroll_media ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  # Enroll from detections
  add_executable(janice_enroll_detections janice_enroll_detections.cpp)
  target_link_libraries(janice_enroll_detections ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  # Search
  add_executable(janice_search janice_search.cpp)
  target_link_libraries(janice_search ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  # specific eval application targets
  #add_executable(janice_eval_cluster janice_eval_cluster.cpp)
  #target_link_libraries(janice_eval_cluster janice_harness ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  #add_executable(janice_batch_extract janice_batch_extract.cpp)
  #target_link_libraries(janice_batch_extract janice_harness ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})
endif()
