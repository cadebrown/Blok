
add_executable (Blocc Blocc.cc Blocc-Entity.cc Blocc-Render.cc log.cc gl3w/gl3w.c)

target_link_libraries(Blocc ${depLibs} pthread dl m GL GLU Xrandr Xext X11 asound jack)
