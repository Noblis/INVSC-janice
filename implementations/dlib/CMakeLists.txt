include(ExternalProject)

ExternalProject_Add(dlib_src
                    GIT_REPOSITORY "https://github.com/davisking/dlib.git"
                    GIT_TAG "master"
                    PREFIX ${CMAKE_BINARY_DIR}/dlib
                    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/dlib_install -DCMAKE_BUILD_TYPE:STRING=Release -DDLIB_USE_CUDA:BOOL=OFF -DDLIB_NO_GUI_SUPPORT:BOOL=ON
                    BUILD_COMMAND make
                    INSTALL_COMMAND make install
                    LOG_DOWNLOAD ON)

find_package(BLAS REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCES
  core.cpp
  detect.cpp
  enroll.cpp
  verify.cpp
  gallery.cpp
  search.cpp
  cluster.cpp
)

add_library(janice_dlib ${SOURCES})
target_include_directories(janice_dlib PRIVATE ${CMAKE_BINARY_DIR}/dlib_install/include ../../api/ ../)
add_dependencies(janice_dlib dlib_src)
set_target_properties(janice_dlib PROPERTIES
                                       DEFINE_SYMBOL JANICE_LIBRARY
                                       VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                                       SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(janice_dlib ${BLAS_LIBRARIES} ${CMAKE_BINARY_DIR}/dlib_install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}dlib${CMAKE_STATIC_LIBRARY_SUFFIX})

install(TARGETS janice_dlib RUNTIME DESTINATION bin
                            LIBRARY DESTINATION lib
                            ARCHIVE DESTINATION lib)

enable_testing()

if (${BUILD_TESTING})
  add_subdirectory(test)
endif()
