set(JANICE_WITH_EVAL_HARNESS ON CACHE BOOL "Compile evaluation library/utilities")

if (${JANICE_WITH_EVAL_HARNESS} AND (JANICE_IO_IMPLEMENTATION) AND (JANICE_IMPLEMENTATION))
  include_directories(include)

  # Detection
  add_executable(janice_detect janice_detect.cpp)
  target_link_libraries(janice_detect ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  # Enroll from media
  add_executable(janice_enroll_media janice_enroll_media.cpp)
  target_link_libraries(janice_enroll_media ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  # Cluster pre-done template
  add_executable(janice_cluster_templates janice_cluster_templates.cpp)
  target_link_libraries(janice_cluster_templates ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

endif()
