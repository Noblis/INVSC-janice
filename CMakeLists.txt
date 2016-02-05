project(janice)
cmake_minimum_required(VERSION 2.8.6)

set(JANICE_NAME "JanICE")
set(JANICE_DESCRIPTION "JanICE API")

# Extract version info from header file
macro(janice_get_version VAR)
  file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/include/janice.h LINE
       REGEX "#define[ ]+${VAR}[ ]+[0-9]+")
  string(REGEX MATCH "[0-9]+" ${VAR} ${LINE})
endmacro(janice_get_version)
janice_get_version(JANICE_VERSION_MAJOR)
janice_get_version(JANICE_VERSION_MINOR)
janice_get_version(JANICE_VERSION_PATCH)
configure_file(include/janice.h ${CMAKE_CURRENT_BINARY_DIR}/janice.h)
set(JANICE_VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH})

if(UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fvisibility=hidden")
endif()

# Janice API documentation
add_subdirectory(doxygen)

set(JANICE_IMPLEMENTATION_PATH ${JANICE_IMPLEMENTATION_PATH} "" CACHE STRING "Location of implementation of Janice functions")
set(JANICE_IO_IMPLEMENTATION_PATH ${JANICE_IO_IMPLEMENTATION_PATH} src/opencv_io CACHE STRING "Location of implementation of Janice io functions")
if (NOT ${JANICE_IMPLEMENTATIONS_PATH} STREQUAL "")
  add_subdirectory(${JANICE_IMPLEMENTATIONS_PATH} ${CMAKE_BINARY_DIR}/janice)
  add_subdirectory(${JANICE_IO_IMPLEMENTATION_PATH} ${CMAKE_BINARY_DIR}/janice_io)

  # Janice test harness 
  add_subdirectory(src/utils)
endif()

