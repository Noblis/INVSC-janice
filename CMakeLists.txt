project(janice)
cmake_minimum_required(VERSION 3.0)

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

# Include the API header files
include_directories(api)

# Optionally include another language interface
add_subdirectory(interfaces)

# Optionally build one of the provided implementations
add_subdirectory(implementations)

# Optionally build the test harness
add_subdirectory(harness)
