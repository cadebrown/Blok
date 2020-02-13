/* Render.cc - main rendering engine code */

#include <Blok/Render.hh>

namespace Blok::Render {

// render a chunk of data
void Renderer::renderChunk(ChunkID id, Chunk* chunk) {
    // add this to the render queue
    //torender[id] = chunk;
    queue.chunks[id] = chunk;
}


void Renderer::renderMesh(Mesh* mesh, mat4 T) {

    // use the geometry shader
    shaders["geom_mesh"]->use();

    mat4 gPV = gP * gV;

    // set up global matrices
    shaders["geom_mesh"]->setMat4("gPV", gPV);
    shaders["geom_mesh"]->setMat4("gM", T);

    // bind the mesh data
    glBindVertexArray(mesh->glVAO); 

    // draw the basic cube mesh, with all the attributes given 
    glDrawElements(GL_TRIANGLES, mesh->faces.size() * 3, GL_UNSIGNED_INT, 0);



}

/*
void Renderer::renderObj(mat4 gT) {
    // draw mesh

    mat4 gM = glm::translate(vec3(0, 0, 0)) * glm::rotate((float)glfwGetTime(), vec3(0, 1, 0));
    glm::mat4 gPVM = gT * gM;
    glActiveTexture(GL_TEXTURE7); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/FieryPattern.png")->glTex);
    //cout << shaders["geometry"]->getUL("texDiffuse") << endl;
    shaders["geometry"]->setInt("texDiffuse", 7);
    //shaders["geometry"]->setMat4("gM", Tmat);
    //shaders["geometry"]->setMat4("gPVM", gPVM);
    //printf("RENDER\n");

    shaders["geometry"]->setMat4("gM", gM);
    shaders["geometry"]->setMat4("gPVM", gPVM);

    //printf("%d,%d\n", mymesh->faces.size() * 3, mymesh->vertices.size());

    //printf("%i\n", (int)(mymesh->faces.size() * 3));

    //for (CDGE::Mesh mesh : mrc->model->meshes) {
    glBindVertexArray(mymesh->glVAO); 
    glDrawElements(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
    //}
}
*/

// begin the rendering sequence
void Renderer::render_start() {

    // enable depth testing
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    // FBO of our rendertarget
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // use our geometry pass
    shaders["geometry"]->use();

    // draw all these attachments
    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    // calculate the perspective matrix
    gP = glm::perspective(glm::radians(FOV / 2.0f), (float)width / height, 0.25f, 400.0f);

    gP[0] *= -1;

    // calculate the view matrix
    gV = glm::lookAt(pos, pos + forward, up);


    //check for any errors
    check_GL();

}

// finalize the rendering sequence
void Renderer::render_end() {
    double stim = getTime();
    /* first: loop through all the chunks that were requested to render */

    // temporary variable for the current block
    BlockData cur;

    // linear list of chunks to render
    List<Chunk*> torender = {};
    for (auto item : queue.chunks) {
        torender.push_back(item.second);
    }

    // capture the number of chunks we need to compute
    int N_chunks = torender.size();

    // temporary variable for the current chunk
    Chunk* chunk;
    // temporary variable to hold the current chunk's ID
    ChunkID cid;

    // first, make sure all hashes are up to date
    for (int idx = 0; idx < N_chunks; ++idx) {
        chunk = torender[idx];

        // clear any previous current render hashes, if it has not been computed, or it has
        //   been modified (i.e. is dirty)
        if (chunk->rcache.curHash == 0 || chunk->rcache.isDirty) chunk->rcache.curHash = chunk->calcHash();
    }


    for (int idx = 0; idx < N_chunks; ++idx) {
        // get the current item on the queue
        chunk = torender[idx];
        cid = chunk->XZ;

        // the hash should already be computed

        // view the chunk as top down, with +X being right, +Z being up, etc
        // like so:

        // +-----+-----+-----+
        // |     |  T  |     |
        // |     |     |     |
        // +-----+-----+-----+
        // |  L  | cur |  R  |
        // |     |     |     |
        // +-----+-----+-----+
        // |     |  B  |     |
        // |     |     |     |
        // +-----+-----+-----+
        //
        // (Z)
        //  ^
        //  + > (X)
        //

        // get the neighbors for the following directions:
        // left, top, right, bottom chunks (see above diagram)
        // if NULL then the chunk is not currently being rendered
        Chunk *cL, *cT, *cR, *cB;

        // temporary to search for given chunks
        ChunkID oid;
        
        //oid = {XZ.X + 1, cid.Z};
        oid = cid + ChunkID(1, 0);
        cR = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        oid = cid + ChunkID(0, 1);
        cT = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        oid = cid + ChunkID(-1, 0);
        cL = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];
      
        oid = cid + ChunkID(0, -1);
        cB = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];


        if (chunk->rcache.curHash == chunk->rcache.lastHash) {
            // the chunk hasn't changed, make sure the neighbors havent either
            if (cL == chunk->rcache.cL && cT == chunk->rcache.cT && cR == chunk->rcache.cR && cB == chunk->rcache.cB) {
                // all the members are the same, then just continue in the for loop, and don't update any thing
                if (!cL || cL->rcache.curHash == cL->rcache.lastHash)
                if (!cT || cT->rcache.curHash == cT->rcache.lastHash)
                if (!cR || cR->rcache.curHash == cR->rcache.lastHash)
                if (!cB || cB->rcache.curHash == cB->rcache.lastHash) {
                    continue;
                }
            }
        }

        // else, set the hash & 2D linked list and continue on

        // reset all the blocks that must be rendered, and recalculate them
        chunk->rcache.renderBlocks.clear();

        // also just set the render cache's 2D doubly-linked list
        chunk->rcache.cL = cL;
        chunk->rcache.cT = cT;
        chunk->rcache.cR = cR;
        chunk->rcache.cB = cB;

        // get the base position of the bottom left back corner
        vec3 chunk_pos = chunk->getWorldPos();

        /* render the highest and lowest layer in the chunk */

        // always render the very top layer and very bottom of each layer
        for (int x = 0; x < CHUNK_SIZE_X; ++x) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                cur = chunk->get(x, 0, z);
                if (cur.id) chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, 0, z), cur });

                cur = chunk->get(x, CHUNK_SIZE_Y - 1, z);
                if (cur.id) chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, CHUNK_SIZE_Y- 1, z), cur });
            }
        }

        
        /* render all the sides, using close chunks if they are also being rendered */

        if (cR != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int x = CHUNK_SIZE_X - 1;

            // check the right side
            for (int z = 1; z < CHUNK_SIZE_X - 1; ++z) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                    // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::AIR) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = chunk->get(x-1, y, z).id != ID::AIR && cR->get(0, y, z).id != ID::AIR;
                    bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::AIR && chunk->get(x, y, z+1).id != ID::AIR;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int x = CHUNK_SIZE_X - 1;
            for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::AIR) continue;

                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        if (cT != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int z = CHUNK_SIZE_Z - 1;

            // check the entire right side (minus very top and bottom)
            for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::AIR) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = chunk->get(x-1, y, z).id != ID::AIR && chunk->get(x+1, y, z).id != ID::AIR;
                    bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::AIR && cT->get(x, y, 0).id != ID::AIR;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int z = CHUNK_SIZE_Z - 1;
            for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::AIR) continue;

                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        if (cL != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int x = 0;

            // check the entire right side (minus very top and bottom)
            for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                    // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::AIR) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = cL->get(CHUNK_SIZE_X-1, y, z).id != ID::AIR && chunk->get(x+1, y, z).id != ID::AIR;
                    bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::AIR && chunk->get(x, y, z+1).id != ID::AIR;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                       chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int x = 0;
            for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::AIR) continue;

                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        if (cB != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int z = 0;

            // check the entire right side (minus very top and bottom)
            for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::AIR) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = chunk->get(x-1, y, z).id != ID::AIR && chunk->get(x+1, y, z).id != ID::AIR;
                    bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                    bool hasZ = cB->get(x, y, CHUNK_SIZE_Z-1).id != ID::AIR && chunk->get(x, y, z+1).id != ID::AIR;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int z = 0;
            for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::AIR) continue;

                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        /* Now, handle the 4 corners */

        // +X,+Z corner
        if (cR != NULL && cT != NULL) {
            int x = CHUNK_SIZE_X - 1, z = CHUNK_SIZE_Z - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = chunk->get(x-1, y, z).id != ID::AIR && cR->get(0, y, z).id != ID::AIR;
                bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                bool hasZ = chunk->get(x, y, z-1).id != ID::AIR && cT->get(x, y, 0).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
                /*
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::AIR) continue;

                blocks.push_back({ chunk_pos + vec3(x, y, z), cur });*/
            }

        } else {
            int x = CHUNK_SIZE_X - 1, z = CHUNK_SIZE_Z - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::AIR) continue;

                chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

        // -X,+Z corner
        if (cL != NULL && cT != NULL) {
            int x = 0, z = CHUNK_SIZE_Z - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = cL->get(CHUNK_SIZE_X-1, y, z).id != ID::AIR && chunk->get(x, y, z).id != ID::AIR;
                bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                bool hasZ = chunk->get(x, y, z-1).id != ID::AIR && cT->get(x, y, 0).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }

        } else {
            int x = 0, z = CHUNK_SIZE_Z - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::AIR) continue;

                chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

        // -X,-Z corner
        if (cL != NULL && cB != NULL) {
            int x = 0, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = cL->get(CHUNK_SIZE_X-1, y, z).id != ID::AIR && chunk->get(x, y, z).id != ID::AIR;
                bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                bool hasZ = cB->get(x, y, CHUNK_SIZE_Z-1).id != ID::AIR && chunk->get(x, y, z+1).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }

        } else {
            int x = 0, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::AIR) continue;

                chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }


        // +X,-Z corner
        if (cR != NULL && cB != NULL) {
            int x = CHUNK_SIZE_X - 1, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = chunk->get(x-1, y, z).id != ID::AIR && cR->get(0, y, z).id != ID::AIR;
                bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                bool hasZ = cB->get(x, y, CHUNK_SIZE_Z-1).id != ID::AIR && chunk->get(x, y, z+1).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }

        } else {
            int x = CHUNK_SIZE_X - 1, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::AIR) continue;

                chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }


        /* now, handle all the interior blocks that haven't been handled so far 
         * 
         * The benefit is that all basic culling operations operate solely within the main chunk
         * 
         */
        for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
            for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
                for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                    cur = chunk->get(x, y, z);

                    // skip block
                    if (cur.id == ID::AIR) continue;


                    // compute whether there are any blocks blocking us
                    bool hasX = chunk->get(x-1, y, z).id != ID::AIR && chunk->get(x+1, y, z).id != ID::AIR;
                    bool hasY = chunk->get(x, y-1, z).id != ID::AIR && chunk->get(x, y+1, z).id != ID::AIR;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::AIR && chunk->get(x, y, z+1).id != ID::AIR;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }
        }
    }

    // just compute this once
    mat4 gPV = gP * gV;

    // keep a list of all visible blocks
    List< Pair<vec3, BlockData> > blocks;

    for (int idx = 0; idx < N_chunks; ++idx) {
        // just expand out the queue entry
        Chunk* chunk = torender[idx];

        // get the number of visible renderable blocks in the chunk
        int N_blocks = chunk->rcache.renderBlocks.size();

        for (int jdx = 0; jdx < N_blocks; ++jdx) {
            blocks.push_back(chunk->rcache.renderBlocks[jdx]);
        }

    }

    //List<mat4> mats_gM, mats_gPVM;

    // TODO: add a per-chunk VBO system
    // For now, just build up the VBO here
    List<vec3> positions;
    List<float> ids;

    for (auto ritem : blocks) {
        positions.push_back(ritem.first);
        ids.push_back((float)ID(ritem.second.id));
    }


    // use the geometry shader
    shaders["geometry"]->use();

    // set up global matrices
    shaders["geometry"]->setMat4("gPV", gPV);

    // now, set the diffuse texture for all blocks
    shaders["geometry"]->setInt("texID1", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/DIRT.png")->glTex);

    shaders["geometry"]->setInt("texID2", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/DIRT_GRASS.png")->glTex);

    shaders["geometry"]->setInt("texID3", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/STONE.png")->glTex);


    // bind the cube data
    glBindVertexArray(mymesh->glVAO); 

    // set the block position VBO data for all the blocks we need to render
    glBindBuffer(GL_ARRAY_BUFFER, glBlockVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnableVertexAttribArray(5);
    glBindBuffer(GL_ARRAY_BUFFER, glBlockVBO);

    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glVertexAttribDivisor(5, 1);  

    // set the VBO that contains their IDs
    glBindBuffer(GL_ARRAY_BUFFER, glIDVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ids.size(), &ids[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnableVertexAttribArray(6);
    glBindBuffer(GL_ARRAY_BUFFER, glIDVBO);

    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(6, 1);  


    // draw the basic cube mesh, with all the attributes given 
    glDrawElementsInstanced(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0, positions.size());



    // do an error check
    check_GL();


    /* now, clear the render queue */

    // set up flags on them
    for (int idx = 0; idx < N_chunks; ++idx) {
        // just expand out the queue entry
        Chunk* chunk = torender[idx];

        // it is not dirty any more
        chunk->rcache.isDirty = false;

        // update the render hash as well
        chunk->rcache.lastHash = chunk->rcache.curHash;

    }

    // remove them all from the queue
    queue.chunks.clear();

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_BACK);
    
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // this code outputs all 4 components of the render texture
    /*int aw = width / 2, ah = height / 2;
    for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j) {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + 2 * i + j);
        glBlitFramebuffer(0, 0, width, height, i * aw, j * ah, (i+1) * aw, (j+1) * ah, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            
    }*/

    // do an error check
    check_GL();
    stim = getTime() - stim;
    static int ct = 0;
    if (++ct % 100 == 0) printf("gfx proc: %lfms\n", 1000.0 * stim);
    if (ct % 100 == 0) blok_debug("tris: %i", (int)(mymesh->faces.size() * ids.size()));


}
};
