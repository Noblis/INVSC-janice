JanICE
=====

[HSI JanICE API](http://noblis.github.io/janice/)

# Build Instructions

    $ git clone https://github.com/Noblis/janice.git
    $ cd janice
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make

# Building PittPatt

    $ git clone https://github.com/Noblis/janice.git
    $ cd janice
    $ mkdir build
    $ cd build
    $ cmake -DJANICE_BUILD_PP5_WRAPPER=ON -DJANICE_BUILD_OPENCV_WRAPPER=ON -DJANICE_IMPLEMENTATION="pittpatt" -DJANICE_IO_IMPLEMENTATION="opencv_io" ..
    $ make -j8 install

**Note:** After the cmake command you may be prompted to manually enter the PittPatt directory if it cannot be found automatically. Using the cmake GUI is recommended here (ccmake ..) but you can specify an additional -DPP5_DIR=/path/to/pp5 to cmake from the command line. 
