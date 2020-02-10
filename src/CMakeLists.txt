
add_executable (Blok  gl3w/gl3w.c  log.cc  Blok-Entity.cc Blok-Render.cc   Blok.cc)

target_link_libraries(Blok ${depLibs} pthread dl m GL GLU Xrandr Xext X11 asound jack)
