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

    if (queue.meshes.find(mesh) == queue.meshes.end()) {
        // add an empty list
        queue.meshes[mesh] = {};
    }

    // now, push it on that list, so we mark it for rendering
    queue.meshes[mesh].push_back(T);
/*
    // use the geometry shader
    shaders["geom_mesh"]->use();

    mat4 gPV = gP * gV;

    // set up global matrices
    shaders["geom_mesh"]->setMat4("gPV", gPV);
    shaders["geom_mesh"]->setMat4("gM", T);

    // bind the mesh data
    glBindVertexArray(mesh->glVAO); 

    printf("RENDERMESH\n");

    // draw the basic cube mesh, with all the attributes given 
    glDrawElements(GL_TRIANGLES, mesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
*/
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

    /* STATISTICS */

    // the time of the rendering
    double stim = getTime();
    
    // the number of chunk re-hashes
    int num_rehashes = 0;

    // number of chunk recalculations (i.e. lighting/mesh/etc )
    int num_recalcs = 0;

    /* first: loop through all the chunks that were requested to render */

    // linear list of chunks to render, for quick iteration
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
        if (chunk->rcache.curHash == 0 || chunk->rcache.isDirty) {
            chunk->rcache.curHash = chunk->calcHash();
            num_rehashes++;
        }
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

        // also just set the render cache's 2D doubly-linked list
        chunk->rcache.cL = cL;
        chunk->rcache.cT = cT;
        chunk->rcache.cR = cR;
        chunk->rcache.cB = cB;

        // recalculate the VBO
        chunk->calcVBO();

        // keep track of recalculations
        num_recalcs++;
    }

    stim = getTime() - stim;
    static int ct = 0;
    if (++ct % 100 == 0) blok_debug("gfx proc: %lfms, n_recalcs: %i", 1000.0 * stim, num_recalcs);
    //if (ct % 100 == 0) blok_debug("tris: %i", (int)(mymesh->faces.size() * ids.size()));

    /* Now, actually render with OpenGL */

    // enable depth testing
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    /* RENDER GEOMETRY PASS */
    
    // first, set up the render target for the geometry pass (i.e. the scene geometry to image)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    // DO CHUNKS

    // use our geometry pass
    shaders["geometry"]->use();

    // draw all these attachments
    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    // just compute this once
    mat4 gPV = gP * gV;

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

    glBindVertexArray(mymesh->glVAO); 

    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);  

    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1); 
    
    // render the chunks
    for (int idx = 0; idx < N_chunks; ++idx) {
        // just expand out the queue entry
        Chunk* chunk = torender[idx];

        glBindBuffer(GL_ARRAY_BUFFER, chunk->rcache.glVBO_blocks);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, chunk->rcache.glVBO_ids);
        glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

        glDrawElementsInstanced(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0, chunk->rcache.renderBlocks.size());
    }



    // DO MESHES

    // use the geometry shader
    shaders["geom_mesh"]->use();

    // set up global matrices
    shaders["geom_mesh"]->setMat4("gPV", gPV);

    // loop through mesh & transform sets
    for (const Pair< Mesh*, List<mat4> >& MTs : queue.meshes) {

        // bind the current mesh
        glBindVertexArray(MTs.first->glVAO); 

        // render all the transforms
        // TODO: we could put them into a VBO
        for (mat4 T : MTs.second) {
            shaders["geom_mesh"]->setMat4("gM", T);
             
            glDrawElements(GL_TRIANGLES, MTs.first->faces.size() * 3, GL_UNSIGNED_INT, 0);

        }
    }

    // draw the basic cube mesh, with all the attributes given 
    //glDrawElementsInstanced(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0, positions.size());

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

    Mesh* ssq = Mesh::getConstSSQ();

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // FBO of our rendertarget
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["ssq"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // draw all these attachments
    glDrawBuffers(targets["ssq"]->glColorAttachments.size(), &targets["ssq"]->glColorAttachments[0]);

    shaders["ssq"]->use();

    shaders["ssq"]->setFloat("time", (float)getTime());
    shaders["ssq"]->setInt("texSrc", 6);
    glActiveTexture(GL_TEXTURE6);
    //glBindTexture(GL_TEXTURE_2D, Texture::loadConst("../resources/DIRT.png")->glTex);
    glBindTexture(GL_TEXTURE_2D, targets["geometry"]->glTex[0]);
    //glBindTexture(GL_TEXTURE_2D, mainFont->glTex);


    glBindVertexArray(ssq->glVAO); 

    // draw the basic cube mesh, with all the attributes given 
    glDrawElements(GL_TRIANGLES, ssq->faces.size() * 3, GL_UNSIGNED_INT, 0);


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["ssq"]->glFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_BACK);
    
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


    // draw to actual screen
    /*glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_BACK);
    
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

/*

*/
    // this code outputs all 4 components of the render texture
    /*int aw = width / 2, ah = height / 2;
    for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j) {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + 2 * i + j);
        glBlitFramebuffer(0, 0, width, height, i * aw, j * ah, (i+1) * aw, (j+1) * ah, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            
    }*/


    // now, clear the cache for the next run

    // remove all chunks requested for render
    queue.chunks.clear();
    // clear all requested meshes
    queue.meshes.clear();

    // do an error check
    check_GL();

}
};
