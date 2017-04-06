
option(JANICE_WTIH_EVAL_HARNESS "Compile evaluation library/utilities" ON)

if ((${JANICE_WITH_EVAL_HARNESS}) AND (JANICE_IO_IMPLEMENTATION) AND (JANICE_IMPLEMENTATION))
# target for the eval harness library
   add_library(janice_harness SHARED janice_harness_cluster.cpp janice_harness_extract.cpp)
   include_directories(${CMAKE_CURRENT_LIST_DIR})

   set_target_properties(janice_harness PROPERTIES DEFINE_SYMBOL JANICE_HARNESS_LIBRARY)

   target_link_libraries(janice_harness ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

   install(TARGETS janice_harness RUNTIME DESTINATION bin
                                  LIBRARY DESTINATION lib
                                  ARCHIVE DESTINATION lib)

  # specific eval application targets
  add_executable(janice_eval_cluster janice_eval_cluster.cpp)
  target_link_libraries(janice_eval_cluster janice_harness ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})

  add_executable(janice_batch_extract janice_batch_extract.cpp)
  target_link_libraries(janice_batch_extract janice_harness ${JANICE_IO_IMPLEMENTATION} ${JANICE_IMPLEMENTATION})
endif()
