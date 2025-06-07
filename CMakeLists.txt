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

add_executable(fastcap
    src/main.cpp 
    src/ImageGenerator.cpp 
    src/ImageWriter.cpp 
    src/ThreadSafeQueue.cpp 
    src/TurboJPEGWriter.cpp 
    src/Utils.cpp
)

target_link_libraries(fastcap 
    ${OpenCV_LIBS}
    ${TURBOJPEG_LIB}
)

add_executable(tests
    tests/main.cpp
    src/TurboJPEGWriter.cpp 
)

target_link_libraries(tests 
    ${OpenCV_LIBS}
    ${TURBOJPEG_LIB}
)


