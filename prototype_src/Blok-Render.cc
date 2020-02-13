/* Blok-Render.cc - rendering code */

#include <Blok-Render.hh>
#include <Blok-Entity.hh>

// assimp libraries, for asset importing
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Blok::Render {

// render a chunk of data
void Renderer::renderChunk(ChunkID id, Chunk* chunk) {
    // add this to the render queue
    queue.chunks[id] = chunk;
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

    glUseProgram(shaders["geometry"]->glID);

    // draw all these attachments
    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    // calculate the perspective matrix
    gP = glm::perspective(glm::radians(FOV / 2.0f), (float)width / height, 0.25f, 400.0f);

    gP[0] *= -1;

    // calculate the view matrix
    gV = glm::lookAt(pos, pos + forward, up);

    opengl_error_check();

}

// finalize the rendering sequence
void Renderer::render_end() {

    /* first: loop through all the chunks that were requested to render */

    // temporary variable for the current block
    BlockInfo cur;

    // compute whether the renderer direction is pointed in certain ways (for culling)
    // for example, if we are looking in the positive x direction (right), then we just need to check
    // blocks to the left for occlusion (roughly)
    bool lookingX = forward.x > 0, lookingY = forward.y > 0, lookingZ = forward.z > 0;

    List< Pair<ChunkID, Chunk*> > torender = {};
    for (auto item : queue.chunks) {
        torender.push_back(item);
    }

    int torender_size = torender.size();
    // loop through all the chunks that have been requested in this frame


    for (int idx = 0; idx < queue.chunks.size(); ++idx) {
        auto item = torender[idx];
        // just expand out the queue entry
        ChunkID cid = item.first;
        Chunk* chunk = item.second;

        // clear any previous current render hashes
        if (chunk->cache.curRenderHash == 0 || chunk->cache.isRenderDirty) chunk->cache.curRenderHash = chunk->getHash();
    }


    for (int idx = 0; idx < torender_size; ++idx) {
        auto item = torender[idx];
        // just expand out the queue entry
        ChunkID cid = item.first;
        Chunk* chunk = item.second;

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
        
        oid = {cid.X + 1, cid.Z};
        cR = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        oid = {cid.X, cid.Z + 1};
        cT = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        oid = {cid.X - 1, cid.Z};
        cL = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];
      
        oid = {cid.X, cid.Z - 1};
        cB = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        /*
        if (cR && cR->cache.curRenderHash == 0) cR->cache.curRenderHash = cR->getHash();
        if (cT && cT->cache.curRenderHash == 0) cT->cache.curRenderHash = cT->getHash();
        if (cL && cL->cache.curRenderHash == 0) cL->cache.curRenderHash = cL->getHash();
        if (cB && cB->cache.curRenderHash == 0) cB->cache.curRenderHash = cB->getHash();*/


        if (chunk->cache.curRenderHash == chunk->cache.lastRenderHash) {
            // the chunk hasn't changed, make sure the neighbors havent either
            if (cL == chunk->cache.cL && cT == chunk->cache.cT && cR == chunk->cache.cR && cB == chunk->cache.cB) {
                // all the members are the same
                if (!cL || cL->cache.curRenderHash == cL->cache.lastRenderHash)
                if (!cT || cT->cache.curRenderHash == cT->cache.lastRenderHash)
                if (!cR || cR->cache.curRenderHash == cR->cache.lastRenderHash)
                if (!cB || cB->cache.curRenderHash == cB->cache.lastRenderHash) {
                    continue;
                }
            }
        }

        // skip if everything has stayed the same
        /*if (chunk->cache.curRenderHash == chunk->cache.lastRenderHash && 
                (cL == chunk->cache.cL || !cL || cL->cache.curRenderHash == cL->cache.lastRenderHash) &&
                (cT == chunk->cache.cT || !cT || cT->cache.curRenderHash == cT->cache.lastRenderHash) &&
                (cR == chunk->cache.cR || !cR || cR->cache.curRenderHash == cR->cache.lastRenderHash) &&
                (cB == chunk->cache.cB || !cB || cB->cache.curRenderHash == cB->cache.lastRenderHash)
        ) continue;*/

        // else, set the hash & 2D linked list and continue on
        // reset all the blocks that must be rendered, and recalculate them
        chunk->cache.renderBlocks.clear();

        chunk->cache.cL = cL;
        chunk->cache.cT = cT;
        chunk->cache.cR = cR;
        chunk->cache.cB = cB;

        // get the base position of the bottom left back corner
        vec3 chunk_pos(cid.X * CHUNK_SIZE, 0, cid.Z * CHUNK_SIZE);

        // blocks IDs in the positive or negative XYZ direction
        /*uint8_t pX, nX;
        uint8_t pY, nY;
        uint8_t pZ, nZ;*/

        /* render the highest and lowest layer in the chunk */

        // always render the very top layer and very bottom of each layer
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                cur = chunk->get(x, 0, z);
                if (cur.id) chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, 0, z), cur });

                cur = chunk->get(x, CHUNK_HEIGHT - 1, z);
                if (cur.id) chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, CHUNK_HEIGHT - 1, z), cur });
            }
        }

        
        /* render all the sides, using close chunks if they are also being rendered */

        if (cR != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int x = CHUNK_SIZE - 1;

            // check the right side
            for (int z = 1; z < CHUNK_SIZE - 1; ++z) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                    // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::NONE) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = chunk->get(x-1, y, z).id != ID::NONE && cR->get(0, y, z).id != ID::NONE;
                    bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::NONE && chunk->get(x, y, z+1).id != ID::NONE;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int x = CHUNK_SIZE - 1;
            for (int z = 1; z < CHUNK_SIZE - 1; ++z) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::NONE) continue;

                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        if (cT != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int z = CHUNK_SIZE - 1;

            // check the entire right side (minus very top and bottom)
            for (int x = 1; x < CHUNK_SIZE - 1; ++x) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::NONE) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = chunk->get(x-1, y, z).id != ID::NONE && chunk->get(x+1, y, z).id != ID::NONE;
                    bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::NONE && cT->get(x, y, 0).id != ID::NONE;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int z = CHUNK_SIZE - 1;
            for (int x = 1; x < CHUNK_SIZE - 1; ++x) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::NONE) continue;

                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        if (cL != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int x = 0;

            // check the entire right side (minus very top and bottom)
            for (int z = 1; z < CHUNK_SIZE - 1; ++z) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                    // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::NONE) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = cL->get(CHUNK_SIZE-1, y, z).id != ID::NONE && chunk->get(x+1, y, z).id != ID::NONE;
                    bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::NONE && chunk->get(x, y, z+1).id != ID::NONE;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                       chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int x = 0;
            for (int z = 1; z < CHUNK_SIZE - 1; ++z) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::NONE) continue;

                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        if (cB != NULL) {
            // we are also rendering a chunk to the right, so do special detection check here,

            // just compute at the right plane
            int z = 0;

            // check the entire right side (minus very top and bottom)
            for (int x = 1; x < CHUNK_SIZE - 1; ++x) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                    cur = chunk->get(x, y, z);

                    // skip empty blocks
                    if (cur.id == ID::NONE) continue;

                    // compute whether there are any blocks blocking us in any direction
                    bool hasX = chunk->get(x-1, y, z).id != ID::NONE && chunk->get(x+1, y, z).id != ID::NONE;
                    bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                    bool hasZ = cB->get(x, y, CHUNK_SIZE-1).id != ID::NONE && chunk->get(x, y, z+1).id != ID::NONE;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }

        } else {
            int z = 0;
            for (int x = 1; x < CHUNK_SIZE - 1; ++x) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                    
                    // check and make sure the current block is renderable and visible
                    cur = chunk->get(x, y, z);
                    if (cur.id == ID::NONE) continue;

                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

        /* Now, handle the 4 corners */

        // +X,+Z corner
        if (cR != NULL && cT != NULL) {
            int x = CHUNK_SIZE - 1, z = CHUNK_SIZE - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::NONE) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = chunk->get(x-1, y, z).id != ID::NONE && cR->get(0, y, z).id != ID::NONE;
                bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                bool hasZ = chunk->get(x, y, z-1).id != ID::NONE && cT->get(x, y, 0).id != ID::NONE;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
                /*
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::NONE) continue;

                blocks.push_back({ chunk_pos + vec3(x, y, z), cur });*/
            }

        } else {
            int x = CHUNK_SIZE - 1, z = CHUNK_SIZE - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::NONE) continue;

                chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

        // -X,+Z corner
        if (cL != NULL && cT != NULL) {
            int x = 0, z = CHUNK_SIZE - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::NONE) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = cL->get(CHUNK_SIZE-1, y, z).id != ID::NONE && chunk->get(x, y, z).id != ID::NONE;
                bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                bool hasZ = chunk->get(x, y, z-1).id != ID::NONE && cT->get(x, y, 0).id != ID::NONE;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }

        } else {
            int x = 0, z = CHUNK_SIZE - 1;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::NONE) continue;

                chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

        // -X,-Z corner
        if (cL != NULL && cB != NULL) {
            int x = 0, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::NONE) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = cL->get(CHUNK_SIZE-1, y, z).id != ID::NONE && chunk->get(x, y, z).id != ID::NONE;
                bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                bool hasZ = cB->get(x, y, CHUNK_SIZE-1).id != ID::NONE && chunk->get(x, y, z+1).id != ID::NONE;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }

        } else {
            int x = 0, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::NONE) continue;

                chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }


        // +X,-Z corner
        if (cR != NULL && cB != NULL) {
            int x = CHUNK_SIZE - 1, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {

                cur = chunk->get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::NONE) continue;

                // compute whether there are any blocks blocking us in any direction
                bool hasX = chunk->get(x-1, y, z).id != ID::NONE && cR->get(0, y, z).id != ID::NONE;
                bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                bool hasZ = cB->get(x, y, CHUNK_SIZE-1).id != ID::NONE && chunk->get(x, y, z+1).id != ID::NONE;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }

        } else {
            int x = CHUNK_SIZE - 1, z = 0;

            // check up the corner
            for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = chunk->get(x, y, z);
                if (cur.id == ID::NONE) continue;

                chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }


        /* now, handle all the interior blocks that haven't been handled so far 
         * 
         * The benefit is that all basic culling operations operate solely within the main chunk
         * 
         */
        for (int x = 1; x < CHUNK_SIZE - 1; ++x) {
            for (int z = 1; z < CHUNK_SIZE - 1; ++z) {
                for (int y = 1; y < CHUNK_HEIGHT - 1; ++y) {
                    cur = chunk->get(x, y, z);

                    // skip block
                    if (cur.id == ID::NONE) continue;


                    // compute whether there are any blocks blocking us
                    bool hasX = chunk->get(x-1, y, z).id != ID::NONE && chunk->get(x+1, y, z).id != ID::NONE;
                    bool hasY = chunk->get(x, y-1, z).id != ID::NONE && chunk->get(x, y+1, z).id != ID::NONE;
                    bool hasZ = chunk->get(x, y, z-1).id != ID::NONE && chunk->get(x, y, z+1).id != ID::NONE;

                    // if there is any chance of visibility
                    if (!(hasX && hasY && hasZ)) {
                        chunk->cache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                    }
                }
            }
        }
    }

    // just compute this once
    mat4 gPV = gP * gV;

    // now, set the diffuse texture for all blocks
    shaders["geometry"]->setInt("texDiffuse", 7);
    shaders["geometry"]->setInt("texDiffuse2", 8);
    shaders["geometry"]->setInt("texDiffuse3", 9);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/STONE.png")->glTex);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/DIRT.png")->glTex);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/DIRT_GRASS.png")->glTex);


    glBindVertexArray(mymesh->glVAO); 

    // keep a list of all visible blocks
    List< Pair<vec3, BlockInfo> > blocks;

    //#pragma omp parallel for
    for (int idx = 0; idx < torender_size; ++idx) {
    //for (auto item : queue.chunks) {
        auto item = torender[idx];
        // just expand out the queue entry
        ChunkID cid = item.first;
        Chunk* chunk = item.second;

        for (int jdx = 0; jdx < chunk->cache.renderBlocks.size(); ++jdx) {
            blocks.push_back(chunk->cache.renderBlocks[jdx]);
        }

    }

    //List<mat4> mats_gM, mats_gPVM;

    List<vec3> positions;
    List<float> ids;
    // collect all instances of blocks
    for (auto ritem : blocks) {
        positions.push_back(ritem.first);
        ids.push_back((float)ID(ritem.second.id));
        //mat4 gM = glm::translate(ritem.first);

        // combine all 3 to get PVM
        //mat4 gPVM = gPV * gM;

        //mats_gM.push_back(gM);
        //mats_gPVM.push_back(gPVM);

        // and set matrices
        //shaders["geometry"]->setMat4("gM", gM);
        //shaders["geometry"]->setMat4("gPVM", gPVM);

        // draw the actual mesh
       //glDrawElements(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
    }

    shaders["geometry"]->setMat4("gPV", gPV);

    glBindBuffer(GL_ARRAY_BUFFER, glBlockVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, glBlockVBO);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glVertexAttribDivisor(3, 1);  


    glBindBuffer(GL_ARRAY_BUFFER, glIDVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ids.size(), &ids[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, glIDVBO);

    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(4, 1);  

    opengl_error_check();



    // draw them
    glDrawElementsInstanced(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0, positions.size());

    printf("numtris: %i\n", (int)(positions.size() * mymesh->faces.size()));


    /* now, clear the render queue */

    // set up flags on them
    for (int idx = 0; idx < torender_size; ++idx) {
        auto item = torender[idx];
        // just expand out the queue entry
        ChunkID cid = item.first;
        Chunk* chunk = item.second;

        // it is not dirty any more
        chunk->cache.isRenderDirty = false;

        // update the render hash as well
        chunk->cache.lastRenderHash = chunk->cache.curRenderHash;

    }

    // remove them all from the queue
    queue.chunks.clear();



    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);


    glReadBuffer(GL_COLOR_ATTACHMENT0 );
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // read from the 'color' render target
    /*
    int aw = width / 2, ah = height / 2;
    for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j) {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + 2 * i + j);
        glBlitFramebuffer(0, 0, width, height, i * aw, j * ah, (i+1) * aw, (j+1) * ah, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            
    }*/

    opengl_error_check();



}
};
