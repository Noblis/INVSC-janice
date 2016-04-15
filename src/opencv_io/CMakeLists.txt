add_library(opencv_io SHARED opencv_io.cpp)
set_target_properties(opencv_io PROPERTIES
                      DEFINE_SYMBOL JANICE_LIBRARY
                      VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                      SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(opencv_io opencv_core opencv_highgui)
install(TARGETS opencv_io RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)

# Add this to the list of implementations to test
option(JANICE_TEST_OPENCV_IO "Test the OpenCV implementation of the Janice I/O API" ON)
if(${JANICE_TEST_OPENCV_IO})
  set(JANICE_IO_TEST_IMPLEMENTATIONS ${JANICE_IO_TEST_IMPLEMENTATIONS} "opencv_io" PARENT_SCOPE)
endif()
