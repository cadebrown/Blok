/* ChunkMesh.cc - implementation of the ChunkMesh class */

// include rendering library
#include <Blok/Render.hh>

namespace Blok::Render {


// add visible faces to the list
static void addBlock(List<ChunkMeshVertex>& vertices, List<Face>& faces, Chunk* chunk, int x, int y, int z) {

    // top, bottom, left, right, forward, and reverse faces
    bool doTop=false, doBot=false, doLef=false, doRig=false, doFor=false, doBac=false;

    // get the ID
    int id = chunk->get(x, y, z).id;

    // check top and bottom faces
    if (y == CHUNK_SIZE_Y - 1) {
        doTop = true;
    } else if (chunk->get(x, y+1, z).id == ID::AIR) {
        doTop = true;
    }

    if (y == 0) {
        doBot = true;
    } else if (chunk->get(x, y-1, z).id == ID::AIR) {
        doBot = true;
    }


    // check left & right faces
    if (x == CHUNK_SIZE_X-1) {
        if (chunk->rcache.cR == NULL || chunk->rcache.cR->get(0, y, z).id == ID::AIR) {
            doRig = true;
        }
    } else if (chunk->get(x+1, y, z).id == ID::AIR) {
        doRig = true;
    }

    if (x == 0) {
        if (chunk->rcache.cL == NULL || chunk->rcache.cL->get(CHUNK_SIZE_X-1, y, z).id == ID::AIR) {
            doLef = true;
        }
    } else if (chunk->get(x-1, y, z).id == ID::AIR) {
        doLef = true;
    }


    // check left & right faces
    if (z == CHUNK_SIZE_Z-1) {
        if (chunk->rcache.cT == NULL || chunk->rcache.cT->get(x, y, 0).id == ID::AIR) {
            doFor = true;
        }
    } else if (chunk->get(x, y, z+1).id == ID::AIR) {
        doFor = true;
    }

    if (z == 0) {
        if (chunk->rcache.cB == NULL || chunk->rcache.cB->get(x, y, CHUNK_SIZE_Z-1).id == ID::AIR) {
            doBac = true;
        }
    } else if (chunk->get(x, y, z-1).id == ID::AIR) {
        doBac = true;
    }




    /*} else if (x == 0) {
        if (chunk->rcache.cL == NULL || chunk->rcache.cL->get(CHUNK_SIZE_X-1, y, z).id == ID::AIR) {
            doRig = true;
        }
*/

    // render various faces

    if (doTop) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z),     vec2(0.0, 0.5), vec3(0, 1, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z+1),   vec2(0.0, 0.0), vec3(0, 1, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z),   vec2(0.5, 0.5), vec3(0, 1, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z+1), vec2(0.5, 0.0), vec3(0, 1, 0), id));

        faces.push_back({idx, idx+1, idx+2});
        faces.push_back({idx+1, idx+3, idx+2});
    }

    if (doBot) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        vertices.push_back(ChunkMeshVertex(vec3(x, y, z),       vec2(1.0, 0.5), vec3(0, -1, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z+1),     vec2(1.0, 0.0), vec3(0, -1, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z),     vec2(0.5, 0.5), vec3(0, -1, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z+1),   vec2(0.5, 0.0), vec3(0, -1, 0), id));

        faces.push_back({idx+1, idx, idx+2});
        faces.push_back({idx+1, idx+2, idx+3});
    }

    if (doRig) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z),     vec2(0.0, 1.0), vec3(1, 0, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z+1),   vec2(0.5, 1.0), vec3(1, 0, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z),   vec2(0.0, 0.5), vec3(1, 0, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z+1), vec2(0.5, 0.5), vec3(1, 0, 0), id));

        faces.push_back({idx, idx+2, idx+1});
        faces.push_back({idx+1, idx+2, idx+3});
    }

    if (doLef) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        vertices.push_back(ChunkMeshVertex(vec3(x, y, z),     vec2(0.5, 1.0), vec3(-1, 0, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z+1),   vec2(0.0, 1.0), vec3(-1, 0, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z),   vec2(0.5, 0.5), vec3(-1, 0, 0), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z+1), vec2(0.0, 0.5), vec3(-1, 0, 0), id));

        faces.push_back({idx, idx+1, idx+2});
        faces.push_back({idx+1, idx+3, idx+2});
    }


    if (doFor) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        vertices.push_back(ChunkMeshVertex(vec3(x, y, z+1),     vec2(0.5, 1.0), vec3(0, 0, 1), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z+1),   vec2(0.5, 0.5), vec3(0, 0, 1), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z+1),   vec2(0.0, 1.0), vec3(0, 0, 1), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z+1), vec2(0.0, 0.5), vec3(0, 0, 1), id));

        faces.push_back({idx, idx+2, idx+1});
        faces.push_back({idx+1, idx+2, idx+3});
    }

    if (doBac) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        vertices.push_back(ChunkMeshVertex(vec3(x, y, z),     vec2(0.0, 1.0), vec3(0, 0, -1), id));
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z),   vec2(0.0, 0.5), vec3(0, 0, -1), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z),   vec2(0.5, 1.0), vec3(0, 0, -1), id));
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z), vec2(0.5, 0.5), vec3(0, 0, -1), id));

        faces.push_back({idx, idx+1, idx+2});
        faces.push_back({idx+1, idx+3, idx+2});
    }


}
void ChunkMesh::update(Chunk* chunk) {

    // reset the variables here
    vertices = {};
    faces = {};

    // iterate through all non-empty blocks
    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                if (chunk->get(x, y, z).id != ID::AIR) {
                    addBlock(vertices, faces, chunk, x, y, z);
                }
            }
        }
    }

    // translate them
    for (int i = 0; i < vertices.size(); ++i) {
        vertices[i].pos += vec3(chunk->getWorldPos());
    }

    // now, store in the OpenGL objects
    glBindVertexArray(glVAO);

    // now, upload the data
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);

    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ChunkMeshVertex), &vertices[0], GL_STATIC_DRAW);  


    // add the faces to the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * 3 * sizeof(int), &faces[0], GL_STATIC_DRAW);


    glBindVertexArray(0);


}

ChunkMesh* ChunkMesh::fromChunk(Chunk* chunk) {

    ChunkMesh* newcm = new ChunkMesh();
    newcm->update(chunk);

    return newcm;

}

// construct a mesh from a list of vertices and faces.
//   each face is a list of indexes into the vertices array, making up
//   triangles
ChunkMesh::ChunkMesh() {
    
    // just set them equal
    //this->vertices = vertices;
    //this->faces = faces;

    // create OpenGL handles for everything
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);

    // now, innitialize the OpenGL information and send the vertex data to the GPU
    glBindVertexArray(glVAO);
    
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    // set the vertex attribute pointers

    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, pos));

    // vertex texture coords
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, uv));

    // vertex tangent
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, T));

    // vertex bitangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, B));

    // vertex normals
    glEnableVertexAttribArray(4);	
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, N));

    // vertex block ID
    glEnableVertexAttribArray(5);	
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, blockID));


    glBindVertexArray(0);
}

// deconstruct the mesh
ChunkMesh::~ChunkMesh() {
    // just delete our OpenGL handles's resourceses
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
}



}
