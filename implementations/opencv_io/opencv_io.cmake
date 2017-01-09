# Add the OpenCV source and required libraries
# 
# Implementation wishing to use the OpenCV I/O routines should define
# ${JANICE_WITH_OPENCV_IO} in their cmake command, add ${JANICE_EXTRA_SRC} to
# to their library, include <opencv_io.hpp>, and link their library against 
# ${JANICE_EXTRA_LIBS}.

include_directories(opencv_io)

set(JANICE_EXTRA_SRC ${JANICE_EXTRA_SRC} opencv_io/opencv_io.cpp)
set(JANICE_EXTRA_LIBS ${JANICE_EXTRA_LIBS} opencv_core
                                           opencv_highgui
                                           opencv_imgcodecs
                                           opencv_videoio) 
