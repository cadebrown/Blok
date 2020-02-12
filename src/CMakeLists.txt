

file(GLOB BLOK_SRC ./*.cc ./render/*.cc)

add_executable (Blok  gl3w/gl3w.c  ${BLOK_SRC})

set(BLOK_LIBS "")

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Blok OpenMP::OpenMP_CXX)

endif()


target_link_libraries(Blok ${depLibs} pthread dl m GL GLU Xrandr Xext X11 zlibstatic IrrXML asound jack)



