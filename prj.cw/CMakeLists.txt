cmake_minimum_required(VERSION 3.10)
project(qr_detector)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(qr_detector
    src/main.cpp
    src/code_scanner.cpp
)

target_include_directories(qr_detector PRIVATE include)
target_link_libraries(qr_detector ${OpenCV_LIBS})
