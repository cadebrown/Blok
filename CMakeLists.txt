cmake_minimum_required (VERSION 3.1)
project (Blok)

# Set C++11, standards compliant, no extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to release, if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# set C++ flags to always check for errors
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# debug releases should be compiled with -g
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# default release to -O3 for optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# set variables that are needed
set(BUILD_STATIC_LIB ON)

# requirement: OpenGL configuration
find_package(OpenGL REQUIRED)


# run ./build_libs.sh to build these requirements
set(GLFW_DIR ${CMAKE_CURRENT_BINARY_DIR}/glfw-3.3 CACHE STRING "where is glfw located?")
set(ASSIMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/assimp-4.1.0 CACHE STRING "where is assimp located?")
set(PORTAUDIO_DIR ${CMAKE_CURRENT_BINARY_DIR}/portaudio CACHE STRING "where is portaudio located?")
set(FREETYPE_DIR ${CMAKE_CURRENT_BINARY_DIR}/freetype-2.9 CACHE STRING "where is freetype located?")

# output where these are located
message("GLFW_DIR: " ${GLFW_DIR})
message("ASSIMP_DIR: " ${ASSIMP_DIR})
message("PORTAUDIO_DIR: " ${PORTAUDIO_DIR})
message("FREETYPE_DIR: " ${FREETYPE_DIR})

# set the variable 
#set(blok_deps ${GLFW_DIR}/src/libglfw3.a ${ASSIMP_DIR}/lib/libassimp.a ${PORTAUDIO_DIR}/lib/.libs/libportaudio.a ${FREETYPE_DIR}/objs/.libs/libfreetype.a)
set(blok_deps glfw3 assimp portaudio freetype dl m GL GLU Xrandr Xext X11 zlibstatic IrrXML asound jack pthread)

# add these directories to search for
link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/prefix/lib

    #${GLFW_DIR}/src
    #${ASSIMP_DIR}/lib
    #${PORTAUDIO_DIR}/lib/.libs
    #${FREETYPE_DIR}/objs/.libs
)

# include this like with -I
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}

    ${CMAKE_CURRENT_BINARY_DIR}/prefix/include
    ${CMAKE_CURRENT_BINARY_DIR}/prefix/include/freetype2

    #${GLFW_DIR}/include
    #${ASSIMP_DIR}/include
    #${PORTAUDIO_DIR}/include
    #${FREETYPE_DIR}/include
)

# build the executable
add_subdirectory(Blok)

