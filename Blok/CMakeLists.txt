
# declare our main executable for our game, with all source codes
add_executable (Blok
    # in-tree dependencies/generated code for the OpenGL loader
    gl3w/gl3w.c 

    # actual Blok code
    Blok.cc Render.cc Server.cc Client.cc

    # rendering utility
    render/Texture.cc render/FontTexture.cc render/UIText.cc render/Mesh.cc render/ChunkMesh.cc render/Shader.cc render/Target.cc

    # audio utility
    audio/Buffer.cc audio/Engine.cc

    # world generation routines
    WG/Default.cc WG/Flat.cc
)

# link the libraries with all the dependency libraries
target_link_libraries(Blok 
    ${BLOK_LIBS}
)
