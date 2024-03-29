/* Render.cc - main rendering engine code 
 *
 * Blok's rendering engine is split up into a few phases:
 * 
 *   1 (Collect): Chunks, meshes, text, etc are collected and entered into a queue/set datastructure
 *     , or even sometimes, maps are used to instance render. These meshes enter into the 'scene geometry', representing real-world coordinates
 *   2 (Group): Once all calls have been submitted, the idea is that the engine now has a collection of similar meshes with different
 *     transforms/textures/etc to render, so now we can call instanced rendering shaders to speed up the process
 *   3 (Render Pipeline): Now, the scene geometry is rendered in a few passes:
 *     a. 'GEOM', a pass that just renders to a frame buffer with color, position, UVs, etc. Some ambient occlusion is baked in, but no dynamic lights
 *     b. 'SHADOW_MAPS', a pass that renders out shadow maps for the sun, torches (maybe?), 
 * 
 */

#include <Blok/Render.hh>

#include <Blok/Entity.hh>

namespace Blok::Render {

// resize the rendering engine to a new output size
void Renderer::resize(int w, int h) {
    // if nothing has changed, return
    if (w == width && h == height) return;

    width = w;
    height = h;

    // else, reset some stuff like the render targets
    targets["GEOM"]->resize(width, height);
    targets["LBASIC"]->resize(width, height);
}

// render a chunk of data
void Renderer::renderChunk(ChunkID id, Chunk* chunk) {
    // add this to the render queue
    //torender[id] = chunk;
    queue.chunks[id] = chunk;
}

// render a render data
void Renderer::renderData(RenderData& data) {
    // for now, just render the given mesh
    if (!data.mesh) {
        blok_warn("Attempted to renderData with mesh==NULL!");
        return;
    }

    // ensure there is a list for the current mesh
    if (queue.rds.find(data.mesh) == queue.rds.end()) {
        queue.rds[data.mesh] = {};
    }

    // now, push it on that list, so we mark it for rendering
    queue.rds[data.mesh].push_back(data);

}

// render some text
void Renderer::renderText(vec2 pxy, UIText* text, vec2 scalexy) {

    // ensure there is a list of texts for the given font
    if (queue.texts.find(text->font) == queue.texts.end()) {
        queue.texts[text->font] = {};
    }

    queue.texts[text->font].push_back({pxy, text});
}

// render a debug line
void Renderer::renderDebugLine(vec3 start, vec3 end, vec3 col) {
    queue.lines.push_back({start, col, end, col});
}

// finalize the rendering sequence
void Renderer::renderFrame() {

    // gather some statistics

    // time spent processing chunks
    stats.t_chunks = getTime();

    // keep track of how many triangles there are
    stats.n_tris = 0;

    // number of chunk recalculations (i.e. lighting/mesh/etc )
    stats.n_chunk_recalcs = 0;

    float aspect = (float)width / height;

    // calculate the perspective matrix
    gP = glm::perspective(
        glm::radians(FOV), // field of view
        (float)width / height, // aspect ratio
        0.1f, // clip near distance
        1000.0f // clip far distance
    );

    /*gP = glm::ortho<float>(
        -25 * aspect, 25 * aspect,
        -25, 25,
        0.1, 300.0
    );*/

    // invert the X axis, because I want a LHCS
    gP[0] *= -1;

    // calculate a view matrix
    gV = glm::lookAt(
        pos, // camera position
        pos + forward, // target to look at
        up // up direction (always (0, 1, 0))
    );

    // just compute this once
    mat4 gPV = gP * gV;



    /* COLLECT CHUNKS */

    // the number of chunk re-hashes
    int num_rehashes = 0;

    // #1: Go through and filter all the chunks that were requested to be renderered

    // first, decompose the map into a linear list, for quick iteration
    List<Chunk*> torender = {};
    for (auto item : queue.chunks) {
        // TODO: perhaps filter/cull based on bounding boxes?
        if (item.second != NULL) torender.push_back(item.second);
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
        double stime = getTime();
        // get the current item on the queue
        Chunk* chunk = torender[idx];
        ChunkID cid = chunk->XZ;

    
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
        // TODO: maybe add a specific range of values that have been modified. For example,
        //   if an interior block is modified that won't affect neighboring chunks, don't update
        //   or recalculate the neighbors
        // This makes the queuing code more complex, but it shouldn't be too bad
        if (chunk->rcache.curHash != 0 && chunk->rcache.curHash == chunk->rcache.lastHash) {

            if (chunkMeshes.find(chunk) != chunkMeshes.end()) {
            
                // if the chunk's hash didn't change, make sure none of the neighbors have changed
                if (cL == chunk->rcache.cL && cT == chunk->rcache.cT && cR == chunk->rcache.cR && cB == chunk->rcache.cB) {
                    
                    // make sure the neighors either don't exist (and so couldn't have changed), or the hash is the same
                    // if nothing has changed, we can skip this iteration of the for loop, because we don't need to recalculate the VBO
                    // TODO: just check if the dirtyMin/Max includes near the edge of the chunk
                    //   if just interior blocks of the neighbors have changed, we don't need to recalculate our geometry
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

        // the hash should already be up-to-date at this point

        // otherwise, we need to recalculate it
        chunkMeshRequests.insert(chunk);
        
    }


    double stime_cu = getTime();
    // take one off 
    int ct = 0;
    // spend up to 5ms per frame updating them
    while (chunkMeshRequests.size() > 0 && getTime() - stime_cu < 0.005) {
        auto cmit = chunkMeshRequests.begin();

        ChunkMesh* newcm = NULL;
        if (chunkMeshes.find(*cmit) != chunkMeshes.end()) {
            // first try and reuse
            newcm = chunkMeshes[*cmit];

        } else if (chunkMeshPool.size() == 0) {
            //blok_trace("new ChunkMesh");
            newcm = new ChunkMesh();
            chunkMeshes[*cmit] = newcm;
        } else {
            newcm = chunkMeshPool.back();
            chunkMeshPool.pop_back();
            chunkMeshes[*cmit] = newcm;
        }

        // calculate the update
        newcm->update(*cmit);
        stats.n_chunk_recalcs++;

        chunkMeshRequests.erase(cmit);
        ct++;
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

    // output statistics
    if (stats.n_chunk_recalcs != 0) blok_trace("updated %i chunks in %.1lfms", (int)stats.n_chunk_recalcs, 1000.0 * stats.t_chunks);


    /* Now, actually render with OpenGL */

    // set up the screen
    glViewport(0, 0, width, height);

    // enable face culling, and cull the faces that are facing away from us
    // I'm using clockwise as the winding
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    /* 'GEOM', RENDER GEOMETRY PASS */

    // enable depth testing, and make objects that are closer (i.e. have less distance) show up in front
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);
    
    // disable blending for the geometry pass
    glDisable(GL_BLEND);

    // draw to the 'geometry' target in the renderer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["GEOM"]->glFBO);

    // and draw to all the color attachments
    glDrawBuffers(targets["GEOM"]->glColorAttachments.size(), &targets["GEOM"]->glColorAttachments[0]);

    // clear the render target from last frame
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // now, set background color
    glClearColor(160/255.0f, 224/255.0f, 254/255.0f, 1.0f);


    // use our geometry shader
    shaders["GEOM_ChunkMesh"]->use();

    // set up global matrices (i.e. the camera transform)
    shaders["GEOM_ChunkMesh"]->setMat4("gPV", gPV);

    // now, set the diffuse texture for all blocks
    // TODO: write a texture atlas class to handle these
    shaders["GEOM_ChunkMesh"]->setInt("texID1", 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/DIRT.png")->glTex);

    shaders["GEOM_ChunkMesh"]->setInt("texID2", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/DIRT_GRASS.png")->glTex);

    shaders["GEOM_ChunkMesh"]->setInt("texID3", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, Texture::loadConst("assets/tex/block/STONE.png")->glTex);

    // render through all the available chunks to render
    for (int idx = 0; idx < N_chunks; ++idx) {
        Chunk* chunk = torender[idx];
        if (chunkMeshes.find(chunk) != chunkMeshes.end()) {
            // we've got a mesh ready to render
            ChunkMesh* cm = chunkMeshes[chunk];

            // bind the chunk mesh (which also binds all the other properties with it)
            glBindVertexArray(cm->glVAO);
            glDrawElements(GL_TRIANGLES, cm->faces.size() * 3, GL_UNSIGNED_INT, 0);

     
            // add the number of triangles we requested to render
            stats.n_tris += cm->faces.size();
        }

    } 

    // // Render misc. meshes out

    // use the geometry shader for arbitrary meshes
    shaders["GEOM_Mesh"]->use();

    // set up global matrices
    shaders["GEOM_Mesh"]->setMat4("gPV", gPV);

    // loop through mesh/transform requests
    for (const Pair< Mesh*, List<RenderData> >& MTs : queue.rds) {

        // bind the current mesh
        glBindVertexArray(MTs.first->glVAO); 

        // render all the transforms
        // TODO: we could put them into a VBO
        for (RenderData T : MTs.second) {
            shaders["GEOM_Mesh"]->setMat4("gM", T.T);             

            glDrawElements(GL_TRIANGLES, MTs.first->faces.size() * 3, GL_UNSIGNED_INT, 0);

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

    /* LBASIC: lighting pass */

    // get the screen-space-quad model
    Mesh* ssq = Mesh::getConstSSQ();

    glBindFramebuffer(GL_FRAMEBUFFER, targets["LBASIC"]->glFBO);
    glDrawBuffers(targets["LBASIC"]->glColorAttachments.size(), &targets["LBASIC"]->glColorAttachments[0]);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); 
    
    // render a screen space coord
    shaders["LBASIC"]->use();

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, targets["GEOM"]->glTex[0]);
    shaders["LBASIC"]->setInt("texDiffuse", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, targets["GEOM"]->glTex[3]);
    shaders["LBASIC"]->setInt("texNormal", 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, targets["GEOM"]->glTex[4]);
    shaders["LBASIC"]->setInt("texWPos", 6);

    // draw the quad
    glBindVertexArray(ssq->glVAO);
    glDrawElements(GL_TRIANGLES, ssq->faces.size() * 3, GL_UNSIGNED_INT, 0);
    stats.n_tris += ssq->faces.size();


    /* RENDER UI/TEXT PASS */

    // first, set up state
    // enable blending so just the colored parts of the text show up
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST); 

    // use the shader for rending characters
    shaders["TextQuad"]->use();

    // use a simple orthographic projection in screen coordinates
    // TODO: use a normalized coordinate system? So that text scales linearly?
    mat4 gP_text = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

    for (auto& entry : queue.texts) {
        FontTexture* ftext = entry.first;

        // set up the font texture
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, ftext->glTex);
        shaders["TextQuad"]->setInt("texFont", 8);

        for (auto& uit : entry.second) {
            if (uit.second->cache.lastText != uit.second->text || uit.second->cache.lastMaxWidth != uit.second->maxWidth) {
                // recalculate it
                uit.second->calcVBO();

                uit.second->cache.lastText = uit.second->text;
                uit.second->cache.lastMaxWidth = uit.second->maxWidth;
            }

            // translate the position off
            mat4 gM = glm::translate(vec3(uit.first.x, uit.first.y, 0.0));
            shaders["TextQuad"]->setMat4("gPM", gP_text * gM);

            // now, draw it
            glBindVertexArray(uit.second->glVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3 * uit.second->tris);
        }
    }


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

    /*
    glBindFramebuffer(GL_FRAMEBUFFER, targets["ssq"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);

    // render a screen space coord
    shaders["ssq"]->use();

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, targets["GEOM"]->glTex[0]);
    shaders["ssq"]->setInt("texSrc", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, targets["GEOM"]->glTex[1]);
    shaders["ssq"]->setInt("texPos", 5);

    // draw the quad
    glBindVertexArray(ssq->glVAO); 
    glDrawElements(GL_TRIANGLES, ssq->faces.size() * 3, GL_UNSIGNED_INT, 0);
    stats.n_tris += ssq->faces.size();
    */


    /* draw to the screen */
    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["LBASIC"]->glFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    //glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["GEOM"]->glFBO);
    //glReadBuffer(GL_COLOR_ATTACHMENT1);
    glDrawBuffer(GL_BACK);

    // TODO: WHY?
    glBlitFramebuffer(0, 0, width, height, 0, 0, 2*width, 2*height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // now, clear the cache for the next run

    // remove all chunks requested for render
    queue.chunks.clear();
    // clear all requested meshes
    queue.rds.clear();
    // clera all tests
    queue.texts.clear();

    queue.lines.clear();

    // do an error check
    check_GL();

}
};
