# CMakeLists.txt - main build generator file for the Blok project,
#   allows configuration for multiple machines, hopefully eventually it will
#   be working for Linux, MacOS, and Windows

cmake_minimum_required (VERSION 3.1)
project (Blok)

# Set C++11, standards compliant, no extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to release, if not specified
if (NOT CMAKE_BUILD_TYPE)
    #set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# set C++ flags to always check for errors
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# debug releases should be compiled with -g
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# default release to -O3 for optimization, and -s for stripping the binary
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")

# set variables that are needed
set(BUILD_STATIC_LIB ON)

# requirement: OpenGL library
find_package(OpenGL REQUIRED)
#find_package(PortAudio REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)

#find_package(glfw3 3.3 REQUIRED PATHS ${CMAKE_CURRENT_BINARY_DIR}/out)
# just debug it out
message("OPENGL_LIBRARIES: " "${OPENGL_LIBRARIES}")

find_package(assimp REQUIRED)
#message("PORTAUDIO_LIBRARIES: " "${PORTAUDIO_LIBRARIES}")

# add these directories to search for
link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/out/lib
)

# include this like with -I
include_directories(
    # for blok headers
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Blok/include

    # for our built binaries
    ${CMAKE_CURRENT_BINARY_DIR}/out/include
    ${CMAKE_CURRENT_BINARY_DIR}/out/include/freetype2
    /opt/homebrew/Cellar/freetype/2.13.2/include/freetype2
    /opt/homebrew/include

    # include wherever OpenGL is installed at
    ${OPENGL_INCLUDE_DIRS}
    ${ASSIMP_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

# record all the libraries we need to link with for our application
# TODO: Do I need Xrandr Xext ? 
set(BLOK_LIBS 
    # graphics libraries
    ${OPENGL_LIBRARIES} 
    glfw
    
    # other dependencies we build
    #assimp portaudio freetype zlibstatic IrrXML
    ${PORTAUDIO_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${ASSIMP_LIBRARIES}
    # audio backends
    #asound jack 
    
    # system dependencies
    #pthread dl m
)
# add our source directory
add_subdirectory(Blok)
