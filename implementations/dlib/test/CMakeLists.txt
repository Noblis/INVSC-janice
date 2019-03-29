if (NOT ${JANICE_WITH_OPENCV_IO})
  message(FATAL_ERROR "DLib tests require JANICE_WITH_OPENCV_IO=ON")
endif()

include_directories(tests)

set(TEST_SOURCES
    tests/misc.cpp
    tests/detection.cpp
    tests/enrollment.cpp
    tests/gallery.cpp
    tests/verify.cpp
    tests/search.cpp
    tests/cluster.cpp
)

set(TEST_INCLUDE_DIRS
  ${CMAKE_BINARY_DIR}/dlib_install/include
  ${PROJECT_SOURCE_DIR}/api
  ${PROJECT_SOURCE_DIR}/implementations/dlib
  ${PROJECT_SOURCE_DIR}/implementations/opencv_io
  ${PROJECT_SOURCE_DIR}/implementations/dlib/test
)

# Build the command line executable
add_executable(janice_dlib_tests main.cpp ${TEST_SOURCES})
target_include_directories(janice_dlib_tests PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(janice_dlib_tests janice_dlib janice_io_opencv)
target_compile_definitions(janice_dlib_tests PRIVATE -DDLIB_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

# Install the executable
install(TARGETS janice_dlib_tests RUNTIME DESTINATION bin)

