/* ChunkMesh.cc - implementation of the ChunkMesh class
 *
 * Essentially, this is a subset of meshes that can be generated from a chunk
 * 
 */

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
        if (chunk->rcache.cR != NULL && chunk->rcache.cR->get(0, y, z).id == ID::AIR) {
            doRig = true;
        }
    } else if (chunk->get(x+1, y, z).id == ID::AIR) {
        doRig = true;
    }

    if (x == 0) {
        if (chunk->rcache.cL != NULL && chunk->rcache.cL->get(CHUNK_SIZE_X-1, y, z).id == ID::AIR) {
            doLef = true;
        }
    } else if (chunk->get(x-1, y, z).id == ID::AIR) {
        doLef = true;
    }


    // check forward and back faces
    if (z == CHUNK_SIZE_Z-1) {
        if (chunk->rcache.cT != NULL && chunk->rcache.cT->get(x, y, 0).id == ID::AIR) {
            doFor = true;
        }
    } else if (chunk->get(x, y, z+1).id == ID::AIR) {
        doFor = true;
    }

    if (z == 0) {
        if (chunk->rcache.cB != NULL && chunk->rcache.cB->get(x, y, CHUNK_SIZE_Z-1).id == ID::AIR) {
            doBac = true;
        }
    } else if (chunk->get(x, y, z-1).id == ID::AIR) {
        doBac = true;
    }

    // now, add them to the mesh, if they are visible
    // keep a array of all blocks around the origin
    //    +---+---+---+
    // Y /           /|
    //  /    ...    / |
    // +---+---+---+  |  Z
    // | 2 |11 |20 |  |
    // | 1 |10 |19 |  /
    // | 0 | 9 |18 | /
    // +---+---+---+/ X
    //
    BlockData surround[27];

    // get surrounding sample
    #define GET_S(_x, _y, _z) (surround[(_x) * 9 + (_z) * 3 + (_y)])


    // basically a monte carlo sample
    float raycastFac = 1.0;

    if (doTop || doBot || doRig || doLef || doFor || doBac) {
        // fill up the surrounds
        for (int lx = -1; lx <= 1; ++lx) {
            for (int lz = -1; lz <= 1; ++lz) {
                int nx = x+lx, nz = z+lz;

                // source for the chunk
                Chunk* src = chunk;
                if (nx < 0) {
                    src = src->rcache.cL;
                    nx += CHUNK_SIZE_X;
                } else if (nx >= CHUNK_SIZE_X) {
                    src = src->rcache.cR;
                    nx -= CHUNK_SIZE_X;
                }
                if (src == NULL) continue;

                if (nz < 0) {
                    src = src->rcache.cB;
                    nz += CHUNK_SIZE_Z;
                } else if (nz >= CHUNK_SIZE_Z) {
                    src = src->rcache.cT;
                    nz -= CHUNK_SIZE_Z;
                }
                if (src == NULL) continue;

                for (int ly = -1; ly <= 1; ++ly) {
                    // sample inner cube
                    if (y+ly >= 0 && y+ly < CHUNK_SIZE_Y) {

                        GET_S(lx+1, ly+1, lz+1) = src->get(nx, y+ly, nz);
                    }
                    //surround[lx * 9 + lz * 3 + ly] = chunk->get(x+lx, y+ly, z+lz);
                }
            }            
        }
    }

    if (doTop) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        int last0 = (GET_S(0, 2, 0).id != ID::AIR ? 1 : 0) + (GET_S(1, 2, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 2, 1).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z),     vec2(0.0, 0.5), vec3(0, 1, 0), id, (3 - last0) / 3.0));

        int last1 = (GET_S(0, 2, 2).id != ID::AIR ? 1 : 0) + (GET_S(0, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(1, 2, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z+1),   vec2(0.0, 0.0), vec3(0, 1, 0), id, (3 - last1) / 3.0));

        int last2 = (GET_S(2, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(1, 2, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z),   vec2(0.5, 0.5), vec3(0, 1, 0), id, (3 - last2) / 3.0));

        int last3 = (GET_S(1, 2, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z+1), vec2(0.5, 0.0), vec3(0, 1, 0), id, (3 - last3) / 3.0));

        faces.push_back({idx, idx+1, idx+2});
        faces.push_back({idx+1, idx+3, idx+2});
    }

    if (doBot) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        int last0 = (GET_S(0, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 0, 1).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z),       vec2(1.0, 0.5), vec3(0, -1, 0), id, (3 - last0) / 3.0));

        int last1 = (GET_S(0, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(0, 0, 1).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z+1),     vec2(1.0, 0.0), vec3(0, -1, 0), id, (3 - last1) / 3.0));

        int last2 = (GET_S(2, 0, 1).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z),     vec2(0.5, 0.5), vec3(0, -1, 0), id, (3 - last2) / 3.0));

        int last3 = (GET_S(1, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 1).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z+1),   vec2(0.5, 0.0), vec3(0, -1, 0), id, (3 - last3) / 3.0));

        faces.push_back({idx+1, idx, idx+2});
        faces.push_back({idx+1, idx+2, idx+3});
    }

    if (doRig) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        int last0 = (GET_S(2, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 1).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z),     vec2(0.0, 1.0), vec3(1, 0, 0), id, (3 - last0) / 3.0));

        int last1 = (GET_S(2, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 1).id != ID::AIR ? 1 : 0) + (GET_S(2, 1, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z+1),   vec2(0.5, 1.0), vec3(1, 0, 0), id, (3 - last1) / 3.0));

        int last2 = (GET_S(2, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(2, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z),   vec2(0.0, 0.5), vec3(1, 0, 0), id, (3 - last2) / 3.0));

        int last3 = (GET_S(2, 1, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z+1), vec2(0.5, 0.5), vec3(1, 0, 0), id, (3 - last3) / 3.0));

        faces.push_back({idx, idx+2, idx+1});
        faces.push_back({idx+1, idx+2, idx+3});
    }

    if (doLef) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        int last0 = (GET_S(0, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 0, 1).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z),     vec2(0.5, 1.0), vec3(-1, 0, 0), id, (3 - last0) / 3.0));

        int last1 = (GET_S(0, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(0, 0, 1).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z+1),   vec2(0.0, 1.0), vec3(-1, 0, 0), id, (3 - last1) / 3.0));

        int last2 = (GET_S(0, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 2, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z),   vec2(0.5, 0.5), vec3(-1, 0, 0), id, (3 - last2) / 3.0));

        int last3 = (GET_S(0, 1, 2).id != ID::AIR ? 1 : 0) + (GET_S(0, 2, 1).id != ID::AIR ? 1 : 0) + (GET_S(0, 2, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z+1), vec2(0.0, 0.5), vec3(-1, 0, 0), id, (3 - last3) / 3.0));

        faces.push_back({idx, idx+1, idx+2});
        faces.push_back({idx+1, idx+3, idx+2});
    }


    if (doFor) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        int last0 = (GET_S(0, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z+1),     vec2(0.5, 1.0), vec3(0, 0, 1), id, (3 - last0) / 3.0));

        int last1 = (GET_S(0, 2, 2).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 2).id != ID::AIR ? 1 : 0) + (GET_S(1, 2, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z+1),   vec2(0.5, 0.5), vec3(0, 0, 1), id, (3 - last1) / 3.0));

        int last2 = (GET_S(2, 1, 2).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z+1),   vec2(0.0, 1.0), vec3(0, 0, 1), id, (3 - last2) / 3.0));

        int last3 = (GET_S(1, 2, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 1, 2).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 2).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z+1), vec2(0.0, 0.5), vec3(0, 0, 1), id, (3 - last3) / 3.0));

        faces.push_back({idx, idx+2, idx+1});
        faces.push_back({idx+1, idx+2, idx+3});
    }

    if (doBac) {
        // we are on top, so always render the top face
        int idx = vertices.size();

        int last0 = (GET_S(0, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y, z),     vec2(0.0, 1.0), vec3(0, 0, -1), id, (3 - last0) / 3.0));

        int last1 = (GET_S(0, 2, 0).id != ID::AIR ? 1 : 0) + (GET_S(0, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(1, 2, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x, y+1, z),   vec2(0.0, 0.5), vec3(0, 0, -1), id, (3 - last1) / 3.0));

        int last2 = (GET_S(2, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(1, 0, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 0, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y, z),   vec2(0.5, 1.0), vec3(0, 0, -1), id, (3 - last2) / 3.0));

        int last3 = (GET_S(1, 2, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 1, 0).id != ID::AIR ? 1 : 0) + (GET_S(2, 2, 0).id != ID::AIR ? 1 : 0);
        vertices.push_back(ChunkMeshVertex(vec3(x+1, y+1, z), vec2(0.5, 0.5), vec3(0, 0, -1), id, (3 - last3) / 3.0));

        faces.push_back({idx, idx+1, idx+2});
        faces.push_back({idx+1, idx+3, idx+2});
    }
    // done with this face

}

// update the mesh from a given chunk data
void ChunkMesh::update(Chunk* chunk) {

    // reset the variables here
    vertices = {};
    faces = {};

    // iterate through all non-empty blocks, adding them
    // TODO: maybe use the dirtyMin/Max to only update parts of the mesh?
    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                if (chunk->get(x, y, z).id != ID::AIR) {
                    addBlock(vertices, faces, chunk, x, y, z);
                }
            }
        }
    }

    // translate them to real world positions
    for (int i = 0; i < vertices.size(); ++i) {
        vertices[i].pos += vec3(chunk->getWorldPos());

        // and add an AO effect based on height
        vertices[i].ao *= (0.75 + 0.35 * vertices[i].pos.y / CHUNK_SIZE_Y);
    }

    // now, store in the OpenGL objects
    glBindVertexArray(glVAO);

    // now, upload the data
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);

    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ChunkMeshVertex), &vertices[0], GL_DYNAMIC_DRAW);  

    // add the faces to the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * 3 * sizeof(int), &faces[0], GL_DYNAMIC_DRAW);

    // unbind this state
    glBindVertexArray(0);

}

// construct a mesh from a list of vertices and faces.
//   each face is a list of indexes into the vertices array, making up
//   triangles
ChunkMesh::ChunkMesh() {

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

    // ambient occlusion
    glEnableVertexAttribArray(6);	
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), (void*)offsetof(ChunkMeshVertex, ao));


    // unbind state
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
