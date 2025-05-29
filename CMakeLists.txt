cmake_minimum_required(VERSION 3.10)
project(concurrent_image_recorder)

# Requerir C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar paquetes
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

# Incluir directorios
include_directories(${OpenCV_INCLUDE_DIRS})

# Agregar ejecutable
add_executable(concurrent_image_recorder concurrent_image_recorder.cpp)

# Vincular bibliotecas
target_link_libraries(concurrent_image_recorder ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})

# Agregar flags de optimización para Release
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Verificar que estamos trabajando con C++17 o superior (para filesystem)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
        message(STATUS "GCC versión < 8.0, agregando -lstdc++fs")
        target_link_libraries(concurrent_image_recorder stdc++fs)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        message(STATUS "Clang versión < 7.0, agregando -lstdc++fs")
        target_link_libraries(concurrent_image_recorder stdc++fs)
    endif()
endif()