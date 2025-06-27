cmake_minimum_required(VERSION 3.26)
project(qr_detector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(include)

# Создаем исполняемый файл со всеми источниками
add_executable(qr_detector 
    src/main.cpp
    src/code_scanner.cpp
    src/image_processor.cpp
)

target_link_libraries(qr_detector ${OpenCV_LIBS})
