/* PackedBlockMesh.cc - implementation of the mesh class */

#include <Blok-Render.hh>

namespace Blok::Render {

void PackedBlockMesh::setup() {

    glBindVertexArray(glVAO);
    
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);

    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PackedBlockVertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * 3 * sizeof(int), &faces[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers

    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, sizeof(PackedBlockVertex), (void*)offsetof(PackedBlockVertex, pos));

    // vertex normal
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PackedBlockVertex), (void*)offsetof(PackedBlockVertex, normal));

    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PackedBlockVertex), (void*)offsetof(PackedBlockVertex, uv));

    glBindVertexArray(0);
}


// construct a mesh from a list of vertices and faces.
//   each face is a list of indexes into the vertices array, making up
//   triangles
PackedBlockMesh::PackedBlockMesh(const List<PackedBlockVertex>& vertices, const List<Face>& faces) {
    // just set them equal
    this->vertices = vertices;
    this->faces = faces;

    // create OpenGL handles for everything
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);

    setup();
}


        // from a normal mesh
PackedBlockMesh::PackedBlockMesh(Mesh* m) {
    for (Vertex vert : m->vertices) {
        this->vertices.push_back(PackedBlockVertex(vert.pos, vert.N, vert.uv));
    }

    for (Face face : m->faces) {
        this->faces.push_back(face);
    }

    // create OpenGL handles for everything
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);

    setup();

}


}
