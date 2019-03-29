project(janice LANGUAGES CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)

# Extract version info from header file
macro(janice_get_version VAR)
  file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/api/janice.h LINE
       REGEX "#define[ ]+${VAR}[ ]+[0-9]+")
  string(REGEX MATCH "[0-9]+" ${VAR} ${LINE})
endmacro(janice_get_version)

janice_get_version(JANICE_VERSION_MAJOR)
janice_get_version(JANICE_VERSION_MINOR)
janice_get_version(JANICE_VERSION_PATCH)
set(JANICE_VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH})

configure_file(api/janice.h ${CMAKE_CURRENT_BINARY_DIR}/refreshes_version_when_changed)

# The default implementations use CTest and this command needs to be in the
# root CMakeLists.txt file
include(CTest)

# Default initialize the library variables
set(JANICE_IMPLEMENTATION CACHE STRING "A path to a library implementing the JanICE API")
set(JANICE_IO_IMPLEMENTATION CACHE STRING "A path to a library or libraries implementating the JanICE I/O API")

add_library(janice INTERFACE)
target_include_directories(janice INTERFACE api)

install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/api/janice_error.h
          ${CMAKE_CURRENT_SOURCE_DIR}/api/janice_io.h
          ${CMAKE_CURRENT_SOURCE_DIR}/api/janice.h
        DESTINATION include/janice)

include_directories(api)

# Optionally include another language interface
add_subdirectory(interfaces)

# Optionally build one of the provided implementations
add_subdirectory(implementations)

# Optionally build the test harness
if(NOT JANICE_SKIP_HARNESS)
  add_subdirectory(harness)
endif()
