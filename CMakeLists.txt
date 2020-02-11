cmake_minimum_required (VERSION 3.1)
project (Blok)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(OpenGL REQUIRED)

set(GLFW_DIR ${CMAKE_CURRENT_BINARY_DIR}/glfw-3.3 CACHE STRING "where is glfw located?")
set(ASSIMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/assimp-4.1.0 CACHE STRING "where is assimp located?")
set(PORTAUDIO_DIR ${CMAKE_CURRENT_BINARY_DIR}/portaudio CACHE STRING "where is portaudio located?")

message("GLFW_DIR: " ${GLFW_DIR})
message("ASSIMP_DIR: " ${ASSIMP_DIR})
message("PORTAUDIO_DIR: " ${PORTAUDIO_DIR})

set(depLibs ${GLFW_DIR}/src/libglfw3.a ${ASSIMP_DIR}/lib/libassimp.a ${PORTAUDIO_DIR}/lib/.libs/libportaudio.a)

link_directories(
    ${ASSIMP_DIR}/lib
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLFW_DIR}/include
    ${ASSIMP_DIR}/include
    ${PORTAUDIO_DIR}/include
)


# set variables that are needed
set(BUILD_STATIC_LIB ON)

add_subdirectory(src)
#add_subdirectory(examples)

