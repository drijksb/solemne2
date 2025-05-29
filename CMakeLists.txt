cmake_minimum_required(VERSION 3.10)
project(RandomImageGenerator)

# OpenCV
find_package(OpenCV REQUIRED)
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please set OpenCV_DIR.")
endif()

# TurboJPEG
find_path(TURBOJPEG_INCLUDE_DIR NAMES turbojpeg.h)
find_library(TURBOJPEG_LIB NAMES turbojpeg)

if (NOT TURBOJPEG_INCLUDE_DIR OR NOT TURBOJPEG_LIB)
    message(FATAL_ERROR "TurboJPEG not found. Please install libturbojpeg-dev")
endif()

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${TURBOJPEG_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(random_image_generator 
    src/main.cpp 
    src/ImageGenerator.cpp 
    src/ImageWriter.cpp 
    src/ThreadSafeQueue.cpp 
    src/TurboJPEGWriter.cpp 
    src/Utils.cpp
)

target_link_libraries(random_image_generator 
    ${OpenCV_LIBS}
    ${TURBOJPEG_LIB}
)
