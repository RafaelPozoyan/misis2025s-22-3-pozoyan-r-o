add_executable(dither_test "main.cpp")
add_subdirectory("lib")
target_link_libraries(dither_test PRIVATE dither)