

# declare our main executable
add_executable (Blok
    # in-tree dependencies/generated code
    gl3w/gl3w.c 

    # actual Blok code
    Blok.cc Render.cc Client.cc

    # rendering utility
    render/Texture.cc render/Mesh.cc render/Shader.cc render/Target.cc

    # world generation routines
    WG/Default.cc
)

# link the libraries with all the dependency libraries
target_link_libraries(Blok ${blok_deps} pthread dl m GL GLU Xrandr Xext X11 zlibstatic IrrXML asound jack)



