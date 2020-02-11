

file(GLOB BLOK_SRC ./*.cc ./render/*.cc)

add_executable (Blok  gl3w/gl3w.c  ${BLOK_SRC})

target_link_libraries(Blok ${depLibs} pthread dl m GL GLU Xrandr Xext X11 zlibstatic IrrXML asound jack)
