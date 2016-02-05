find_package(OpenCV REQUIRED)

add_library(opencv_io SHARED opencv_io.cpp ../janice_io.cpp)
set_target_properties(opencv_io PROPERTIES
                      DEFINE_SYMBOL JANICE_LIBRARY
                      VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                      SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(opencv_io opencv_core opencv_highgui)
install(TARGETS opencv_io RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib)

set(JANICE_IO_IMPLEMENTATION_LIBS ${JANICE_IO_IMPLEMENTION_LIBS} "opencv_io" PARENT_SCOPE)
