add_library(dither "dither.cpp" "include/dither/dither.hpp")

target_link_libraries(dither PUBLIC opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui)