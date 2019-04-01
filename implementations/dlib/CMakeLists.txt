find_package(BLAS REQUIRED)

option(JANICE_DLIB_FR_WITH_CUDA OFF)
set(DLIB_USE_CUDA ${JANICE_DLIB_FR_WITH_CUDA})
set(DLIB_NO_GUI_SUPPORT ON)
add_subdirectory(dlib)

set(SOURCES
  core.cpp
  detect.cpp
  enroll.cpp
  verify.cpp
  gallery.cpp
  search.cpp
  cluster.cpp
)

set(INCLUDE_DIRS
  dlib
  ${PROJECT_SOURCE_DIR}/api
  ${PROJECT_SOURCE_DIR}/implementations/dlib
  ${PROJECT_SOURCE_DIR}/implementations/opencv_io
  ${PROJECT_SOURCE_DIR}/implementations/dlib/test
)

add_library(janice_dlib ${SOURCES})
target_include_directories(janice_dlib PRIVATE ${INCLUDE_DIRS})
set_target_properties(janice_dlib PROPERTIES
                                       DEFINE_SYMBOL JANICE_LIBRARY
                                       VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                                       SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(janice_dlib ${BLAS_LIBRARIES} dlib)

install(TARGETS janice_dlib RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib)

enable_testing()

if (${BUILD_TESTING})
  add_subdirectory(test)
endif()
