add_library(pittpatt SHARED pittpatt.cpp ../janice_io.cpp)
set_target_properties(pittpatt PROPERTIES
                      DEFINE_SYMBOL JANICE_LIBRARY
                      VERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR}.${JANICE_VERSION_PATCH}
                      SOVERSION ${JANICE_VERSION_MAJOR}.${JANICE_VERSION_MINOR})
target_link_libraries(pittpatt ${PP5_LIBS} ${JANICE_IO_IMPLEMENTATION})
install(TARGETS pittpatt RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib)

# Add this to the list of implementations to test
set(JANICE_TEST_IMPLEMENTATION ${JANICE_TEST_IMPLEMENTATIONS} "pittpatt" PARENT_SCOPE)
set(JANICE_TEST_SDK_PATH ${JANICE_TEST_SDK_PATHS} ${PP5_DIR} PARENT_SCOPE)
