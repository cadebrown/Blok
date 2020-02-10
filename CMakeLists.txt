cmake_minimum_required (VERSION 3.1)
project (Blok)

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

