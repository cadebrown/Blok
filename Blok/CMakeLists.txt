
# declare our main executable for our game, with all source codes
add_executable (Blok
    # in-tree dependencies/generated code
    gl3w/gl3w.c 

    # actual Blok code
    Blok.cc Render.cc Server.cc Client.cc Chunk.cc

    # rendering utility
    render/Texture.cc render/FontTexture.cc render/UIText.cc render/Mesh.cc render/ChunkMesh.cc render/Shader.cc render/Target.cc

    # world generation routines
    WG/Default.cc
)

# link the libraries with all the dependency libraries
target_link_libraries(Blok ${blok_deps})
