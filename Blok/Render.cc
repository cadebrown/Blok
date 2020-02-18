/* Render.cc - main rendering engine code */

#include <Blok/Render.hh>

namespace Blok::Render {

// resize the rendering engine to a new output size
void Renderer::resize(int w, int h) {
    // if nothing has changed, return
    if (w == width && h == height) return;

    width = w;
    height = h;

    // else, reset some stuff like the render targets
    targets["geometry"]->resize(width, height);
    targets["ssq"]->resize(width, height);
}

// render a chunk of data
void Renderer::renderChunk(ChunkID id, Chunk* chunk) {
    // add this to the render queue
    //torender[id] = chunk;
    queue.chunks[id] = chunk;
}

// render a mesh with a transform
void Renderer::renderMesh(Mesh* mesh, mat4 T) {

    if (queue.meshes.find(mesh) == queue.meshes.end()) {
        // add an empty list
        queue.meshes[mesh] = {};
    }

    // now, push it on that list, so we mark it for rendering
    queue.meshes[mesh].push_back(T);
}

void Renderer::renderText(vec2 pxy, UIText* text, vec2 scalexy) {
    if (queue.texts.find(text->font) == queue.texts.end()) {
        queue.texts[text->font] = {};
    }

    queue.texts[text->font].push_back({pxy, text});
}

// render a debug line
void Renderer::renderDebugLine(vec3 start, vec3 end, vec3 col) {
    queue.lines.push_back({start, col, end, col});
}


// begin the rendering sequence
void Renderer::render_start() {


}

// finalize the rendering sequence
void Renderer::render_end() {

    // gather some statistics

    // time spent processing chunks
    stats.t_chunks = getTime();

    // keep track of how many triangles there are
    stats.n_tris = 0;

    // number of chunk recalculations (i.e. lighting/mesh/etc )
    stats.n_chunk_recalcs = 0;

    // calculate the perspective matrix
    gP = glm::perspective(
        glm::radians(FOV / 2.0f), // field of view
        (float)width / height, // aspect ratio
        0.25f, // clip near distance
        400.0f // clip far distance
    );

    // invert the X axis, because I want a LHCS
    gP[0] *= -1;

    // calculate a view matrix
    gV = glm::lookAt(
        pos, // camera position
        pos + forward, // target to look at
        up // up direction (always (0, 1, 0))
    );


    // the number of chunk re-hashes
    int num_rehashes = 0;

    // #1: Go through and filter all the chunks that were requested to be renderered


    // first, decompose the map into a linear list, for quick iteration
    List<Chunk*> torender = {};
    for (auto item : queue.chunks) {
        // TODO: perhaps filter/cull based on bounding boxes?
        torender.push_back(item.second);
    }

    // capture the number of chunks we need to compute, for a for loop index
    int N_chunks = torender.size();

    // keep track of how many chunks rendered
    stats.n_chunks = N_chunks;


    // first, remove any rendering ChunkMeshes that are not being rendered

    auto cmit = chunkMeshes.cbegin();
    while (cmit != chunkMeshes.cend()) {
        if (std::find(torender.begin(), torender.end(), cmit->first) == torender.end()) {
            // if we didn't find it, remove it from our meshes
            // TODO: use a chunkMesh pool?
            //delete cmit->second;
            // add back to the pool
            chunkMeshPool.push_back(cmit->second);
            //erase from the current chunk meshes
            chunkMeshes.erase(cmit++);
        } else {
            cmit++;
        }
    }

    // first, make sure all hashes are up to date
    // NOTE: we seperate this into a loop before the main recalculation, so that
    //   we can also tell if any neightbors hashes have changed. Without this,
    //   a chunk could have a neighbor that has not had its hash updated, so may not update correctly
    //   itself
    for (int idx = 0; idx < N_chunks; ++idx) {
        Chunk* chunk = torender[idx];

        // clear any previous current render hashes, if it has not been computed, or it has
        //   been modified (i.e. is dirty)
        if (chunk->rcache.curHash == 0 || chunk->rcache.isDirty) {
            chunk->rcache.curHash = chunk->calcHash();
            num_rehashes++;
        }
    }


    for (int idx = 0; idx < N_chunks; ++idx) {
        // get the current item on the queue
        Chunk* chunk = torender[idx];
        ChunkID cid = chunk->XZ;

        // the hash should already be up-to-date at this point

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
        // if NULL then that neighbor chunk is not currently being rendered
        Chunk *cL, *cT, *cR, *cB;

        // temporary variable to probe for neighboring chunks, go through all 
        //   touching chunks and try and get them from our rendering queue
        ChunkID oid;
        
        oid = cid + ChunkID(1, 0);
        cR = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        oid = cid + ChunkID(0, 1);
        cT = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        oid = cid + ChunkID(-1, 0);
        cL = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];
      
        oid = cid + ChunkID(0, -1);
        cB = queue.chunks.find(oid) == queue.chunks.end() ? NULL : queue.chunks[oid];

        // check if the hash has stayed the same, and if so, try and skip the chunk update
        if (chunk->rcache.curHash == chunk->rcache.lastHash) {

            if (chunkMeshes.find(chunk) != chunkMeshes.end()) {
            
                // if the chunk's hash didn't change, make sure none of the neighbors have changed
                if (cL == chunk->rcache.cL && cT == chunk->rcache.cT && cR == chunk->rcache.cR && cB == chunk->rcache.cB) {
                    
                    // make sure the neighors either don't exist (and so couldn't have changed), or the hash is the same
                    // if nothing has changed, we can skip this iteration of the for loop, because we don't need to recalculate the VBO
                    if (!cL || cL->rcache.curHash == cL->rcache.lastHash)
                    if (!cT || cT->rcache.curHash == cT->rcache.lastHash)
                    if (!cR || cR->rcache.curHash == cR->rcache.lastHash)
                    if (!cB || cB->rcache.curHash == cB->rcache.lastHash) {
                        // skip ahead
                        continue;
                    }
                }

            }

        }

        // else, update the 2D linked list structure, and recalculate the chunk geometry

        // update neighbor references
        chunk->rcache.cL = cL;
        chunk->rcache.cT = cT;
        chunk->rcache.cR = cR;
        chunk->rcache.cB = cB;


        // calculate the mesh

        //if (chunkMeshes[chunk] != NULL) delete chunkMeshes[chunk];
        if (chunkMeshes.find(chunk) == chunkMeshes.end()) {
            // hasn't been created yet
            if (chunkMeshPool.size() > 0) {
                chunkMeshes[chunk] = chunkMeshPool[chunkMeshPool.size()-1];
                chunkMeshPool.pop_back();
                chunkMeshes[chunk]->update(chunk);
            } else {
                // create it
                chunkMeshes[chunk] = ChunkMesh::fromChunk(chunk);
            }

        } else {
            // it has changed, so update it
            chunkMeshes[chunk]->update(chunk);
        }

        // recalculate it

        // recalculate the VBO (i.e. calculate visibility for the chunk)
        //chunk->calcVBO();

        // keep track of recalculations
        stats.n_chunk_recalcs++;
    }

    // now, reset the Chunk variables, mark them as rendered
    //   and update their last render hash to their current, for next time
    for (int idx = 0; idx < N_chunks; ++idx) {
        // just expand out the queue entry
        Chunk* chunk = torender[idx];

        // it is not dirty any more
        chunk->rcache.isDirty = false;

        // update the render hash as well
        chunk->rcache.lastHash = chunk->rcache.curHash;
    }

    // record the time it took
    stats.t_chunks = getTime() - stats.t_chunks;

    /* Now, actually render with OpenGL */


    /* RENDER GEOMETRY PASS */

    // enable depth testing, and make objects that are closer (i.e. have less distance) show up in front
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);

    // disable blending for the geometry pass
    glDisable(GL_BLEND);

    // enable face culling, and cull the faces that are facing away from us
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // use clockwise by our convention
    glFrontFace(GL_CW);

    // draw to the 'geometry' target in the renderer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);

    // and draw to all the color attachments
    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    // clear the render target
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // just compute this once
    mat4 gPV = gP * gV;

    // // Render chunks out

    // use our geometry shader
    shaders["geometry"]->use();

    // set up global matrices (i.e. the camera transform)
    shaders["geometry"]->setMat4("gPV", gPV);

    // now, set the diffuse texture for all blocks
    // TODO: write a texture atlas
    shaders["geometry"]->setInt("texID1", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/DIRT.png")->glTex);

    shaders["geometry"]->setInt("texID2", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/DIRT_GRASS.png")->glTex);

    shaders["geometry"]->setInt("texID3", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/STONE.png")->glTex);

    //glBindVertexArray(mymesh->glVAO);

    for (int idx = 0; idx < N_chunks; ++idx) {
        Chunk* chunk = torender[idx];
        ChunkMesh* cm = chunkMeshes[chunk];

        // bind the chunk mesh
        glBindVertexArray(cm->glVAO);
        glDrawElements(GL_TRIANGLES, cm->faces.size() * 3, GL_UNSIGNED_INT, 0);


        stats.n_tris += cm->faces.size();

    } 
/*

    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);  

    
    // render the chunks, by using their list of blocks & IDs
    for (int idx = 0; idx < N_chunks; ++idx) {
        // just expand out the queue entry
        Chunk* chunk = torender[idx];

        glBindBuffer(GL_ARRAY_BUFFER, chunk->rcache.glVBO_blocks);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, chunk->rcache.glVBO_ids);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

        glDrawElementsInstanced(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0, chunk->rcache.renderBlocks.size());

        stats.n_tris += mymesh->faces.size() * chunk->rcache.renderBlocks.size();
    }
*/

    /*
    // use our geometry shader
    shaders["geometry"]->use();

    // set up global matrices (i.e. the camera transform)
    shaders["geometry"]->setMat4("gPV", gPV);

    // now, set the diffuse texture for all blocks
    // TODO: write a texture atlas
    shaders["geometry"]->setInt("texID1", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/DIRT.png")->glTex);

    shaders["geometry"]->setInt("texID2", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/DIRT_GRASS.png")->glTex);

    shaders["geometry"]->setInt("texID3", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/STONE.png")->glTex);

    glBindVertexArray(mymesh->glVAO); 

    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);  

    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);

    
    // render the chunks, by using their list of blocks & IDs
    for (int idx = 0; idx < N_chunks; ++idx) {
        // just expand out the queue entry
        Chunk* chunk = torender[idx];

        glBindBuffer(GL_ARRAY_BUFFER, chunk->rcache.glVBO_blocks);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, chunk->rcache.glVBO_ids);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

        glDrawElementsInstanced(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0, chunk->rcache.renderBlocks.size());

        stats.n_tris += mymesh->faces.size() * chunk->rcache.renderBlocks.size();
    }

    */

    // // Render misc. meshes out

    // use the geometry shader
    shaders["geom_mesh"]->use();

    // set up global matrices
    shaders["geom_mesh"]->setMat4("gPV", gPV);

    // loop through mesh/transform requests
    for (const Pair< Mesh*, List<mat4> >& MTs : queue.meshes) {

        // bind the current mesh
        glBindVertexArray(MTs.first->glVAO); 

        vec4 col = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        // render all the transforms
        // TODO: we could put them into a VBO
        for (mat4 T : MTs.second) {
            shaders["geom_mesh"]->setVec4("col", col);
            shaders["geom_mesh"]->setMat4("gM", T);             
            glDrawElements(GL_TRIANGLES, MTs.first->faces.size() * 3, GL_UNSIGNED_INT, 0);
            col.r = fmodf(col.r + 0.1f, 1.0f);

            stats.n_tris += MTs.first->faces.size();
        }
    }


    // render debugging lines
    if (queue.lines.size() > 0) {
        // render debugging lines

        shaders["DebugLine"]->use();
        shaders["DebugLine"]->setMat4("gPV", gPV);

        // now, draw debug lines
        glBindBuffer(GL_ARRAY_BUFFER, debug.glLinesVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(queue.lines[0]) * queue.lines.size(), &queue.lines[0], GL_DYNAMIC_DRAW);

        // now actually render the lines
        glBindVertexArray(debug.glLinesVAO);
        glDrawArrays(GL_LINES, 0, 2 * queue.lines.size());
    }

    /* RENDER UI/TEXT PASS */

    // first, set up state
    // enable blending so just the colored parts of the text show up
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST); 

    // use the shader for rending characters
    shaders["textquad"]->use();

    // use a simple orthographic projection in screen coordinates
    // TODO: use a normalized coordinate system? So that text scales linearly?
    mat4 gP_text = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

    for (auto& entry : queue.texts) {
        FontTexture* ftext = entry.first;

        // set up the font texture
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, ftext->glTex);
        shaders["textquad"]->setInt("texFont", 8);

        for (auto& uit : entry.second) {
            if (uit.second->cache.lastText != uit.second->text || uit.second->cache.lastMaxWidth != uit.second->maxWidth) {
                // recalculate it
                uit.second->calcVBO();

                uit.second->cache.lastText = uit.second->text;
                uit.second->cache.lastMaxWidth = uit.second->maxWidth;
            }

            // translate the position off
            mat4 gM = glm::translate(vec3(uit.first.x, uit.first.y, 0.0));
            shaders["textquad"]->setMat4("gPM", gP_text * gM);

            // now, draw it
            glBindVertexArray(uit.second->glVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3 * uit.second->tris);
        }
    }

    
    Mesh* ssq = Mesh::getConstSSQ();

    // render reticle

    glDisable(GL_BLEND);

    shaders["Reticle"]->use();

    Texture* reticle = Texture::loadConst("assets/tex/reticle.png");

    // set up the font texture
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, reticle->glTex);
    shaders["Reticle"]->setInt("texFont", 9);

    float size = 16.0f;

    shaders["Reticle"]->setMat4("gT", gP_text * glm::translate(vec3(width / 2.0f, height / 2.0f, 0.0f)) * glm::scale(vec3(size, size, 1)));

    // now, draw it
    glBindVertexArray(ssq->glVAO);
    glDrawElements(GL_TRIANGLES, ssq->faces.size() * 3, GL_UNSIGNED_INT, 0);
    stats.n_tris += ssq->faces.size();


    // do an error check and make sure everything was valid
    check_GL();

    glBindFramebuffer(GL_FRAMEBUFFER, targets["ssq"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);

    // render a screen space coord
    shaders["ssq"]->use();

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, targets["geometry"]->glTex[0]);
    shaders["ssq"]->setInt("texSrc", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, targets["geometry"]->glTex[1]);
    shaders["ssq"]->setInt("texPos", 5);

    // draw the quad
    glBindVertexArray(ssq->glVAO); 
    glDrawElements(GL_TRIANGLES, ssq->faces.size() * 3, GL_UNSIGNED_INT, 0);
    stats.n_tris += ssq->faces.size();

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["ssq"]->glFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_BACK);

    
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // now, clear the cache for the next run

    // remove all chunks requested for render
    queue.chunks.clear();
    // clear all requested meshes
    queue.meshes.clear();
    // clera all tests
    queue.texts.clear();

    queue.lines.clear();

    // do an error check
    check_GL();

}
};
