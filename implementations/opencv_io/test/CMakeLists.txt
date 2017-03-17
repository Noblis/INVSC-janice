# We require C++11 for testing
if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()


# Gather all the tests and then iterate over them
file(GLOB TESTS *.cpp)

foreach(TEST ${TESTS})
  # Get the name of the file without the extension
  get_filename_component(TEST_NAME ${TEST} NAME_WE)

  # Create an executable for the test
  add_executable(${TEST_NAME} ${TEST})

  # Register the test with CMake
  add_test(NAME ${TEST_NAME}
           COMMAND ${TEST_NAME}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  # Link and install the executable against the opencv_io library
  target_link_libraries(${TEST_NAME} opencv_io)
  install(TARGETS ${TEST_NAME}
          RUNTIME DESTINATION bin)
endforeach()
