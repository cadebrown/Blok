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


void Renderer::renderMesh(Mesh* mesh, mat4 T) {

    if (queue.meshes.find(mesh) == queue.meshes.end()) {
        // add an empty list
        queue.meshes[mesh] = {};
    }

    // now, push it on that list, so we mark it for rendering
    queue.meshes[mesh].push_back(T);

}


// begin the rendering sequence
void Renderer::render_start() {


}

// finalize the rendering sequence
void Renderer::render_end() {

    // gather some statistics

    // time spent processing chunks
    double t_chunks = getTime();


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

    // number of chunk recalculations (i.e. lighting/mesh/etc )
    int num_recalcs = 0;

    // #1: Go through and filter all the chunks that were requested to be renderered


    // first, decompose the map into a linear list, for quick iteration
    List<Chunk*> torender = {};
    for (auto item : queue.chunks) {
        // TODO: perhaps filter/cull based on bounding boxes?
        torender.push_back(item.second);
    }

    // capture the number of chunks we need to compute, for a for loop index
    int N_chunks = torender.size();

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

        // else, update the 2D linked list structure, and recalculate the chunk geometry

        // update neighbor references
        chunk->rcache.cL = cL;
        chunk->rcache.cT = cT;
        chunk->rcache.cR = cR;
        chunk->rcache.cB = cB;

        // recalculate the VBO (i.e. calculate visibility for the chunk)
        chunk->calcVBO();

        // keep track of recalculations
        num_recalcs++;
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
    t_chunks = getTime() - t_chunks;

    // record stats
    stats.t_chunks = t_chunks;
    stats.n_chunks = N_chunks;

    // number of recalculations
    stats.n_chunk_recalcs = num_recalcs;

    /* Now, actually render with OpenGL */

    // keep track of how many triangles there are
    int n_tris = 0;

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

        n_tris += mymesh->faces.size() * chunk->rcache.renderBlocks.size();
    }

    // // Render misc. meshes out

    // use the geometry shader
    shaders["geom_mesh"]->use();

    // set up global matrices
    shaders["geom_mesh"]->setMat4("gPV", gPV);

    // loop through mesh/transform requests
    for (const Pair< Mesh*, List<mat4> >& MTs : queue.meshes) {

        // bind the current mesh
        glBindVertexArray(MTs.first->glVAO); 

        // render all the transforms
        // TODO: we could put them into a VBO
        for (mat4 T : MTs.second) {
            shaders["geom_mesh"]->setMat4("gM", T);             
            glDrawElements(GL_TRIANGLES, MTs.first->faces.size() * 3, GL_UNSIGNED_INT, 0);

            n_tris += MTs.first->faces.size();
        }
    }

    // do an error check and make sure everything was valid
    check_GL();

    /* RENDER UI/TEXT PASS */

    // first, set up state
/*
    // enable blending so just the colored parts of the text show up
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // use the shader for rending characters
    shaders["textquad"]->use();

    // use a simple orthographic projection in screen coordinates
    // TODO: use a normalized coordinate system? So that text scales linearly?
    shaders["textquad"]->setMat4("gP", glm::ortho(0.0f, (float)width, 0.0f, (float)height));

    // set up the font texture
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, mainFont->glTex);
    shaders["textquad"]->setInt("texFont", 8);


    UIText* uit = new UIText(mainFont);

    // now, render the text
    uit->text = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !@#$%^&*()_+`-=[]\\{}|;':\",./<>?";

    glBindVertexArray(uit->glVAO);

    float x = 25.0f, y = height - 48;

    float scale = 0.3f;

    // Render glyph texture over quad
    glBindTexture(GL_TEXTURE_2D, uit->font->glTex);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = uit->text.begin(); c != uit->text.end(); c++)
    {

        FontTexture::CharInfo ch = uit->font->charInfos[*c];

        vec2i size = ch.texStop - ch.texStart;

        if (x + size.x * scale >= width - 25.0f) {
            x = 25.0f;
            y -= 64.0f;
        }

        GLfloat xpos = x + ch.bearing.x * scale;

        GLfloat ypos = y - (size.y - ch.bearing.y) * scale;

        GLfloat w = size.x * scale;
        GLfloat h = size.y * scale;

        vec2 uvStart = vec2(ch.texStart) / vec2(uit->font->width, uit->font->height);
        vec2 uvStop = vec2(ch.texStop) / vec2(uit->font->width, uit->font->height);

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos,       uvStart.x, uvStart.y },
            { xpos,     ypos + h,   uvStart.x, uvStop.y },            
            { xpos + w, ypos,       uvStop.x, uvStart.y },

            { xpos,     ypos + h,   uvStart.x, uvStop.y },
            { xpos + w, ypos + h,   uvStop.x, uvStop.y },
            { xpos + w, ypos,       uvStop.x, uvStart.y },
        };
        
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, uit->glVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // draw all these attachments

    // this just puts an image over the entire screen
*/

    Mesh* ssq = Mesh::getConstSSQ();
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
    n_tris += ssq->faces.size();

    /*
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["ssq"]->glFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glDrawBuffer(GL_BACK);
    
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    */

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["ssq"]->glFBO);
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

    // update stats
    stats.n_tris = n_tris;


    // now, clear the cache for the next run

    // remove all chunks requested for render
    queue.chunks.clear();
    // clear all requested meshes
    queue.meshes.clear();

    // do an error check
    check_GL();

}
};
