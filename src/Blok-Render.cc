/* Blok-Render.cc - rendering code */

#include <Blok-Render.hh>
#include <Blok-Entity.hh>

// assimp libraries, for asset importing
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Blok::Render {

void Renderer::renderEntity(Entity* entity) {
    // construct the model matrix
    mat4 gM = glm::translate(entity->loc);

    // combine all 3 to get PVM
    mat4 gPVM = gP * gV * gM;

    // now, set the diffuse texture
    shaders["geometry"]->setInt("texDiffuse", 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, Texture::get("../resources/grass.jpg")->glTex);

    // and set matrices
    shaders["geometry"]->setMat4("gM", gM);
    shaders["geometry"]->setMat4("gPVM", gPVM);


    // draw the actual mesh
    glBindVertexArray(mymesh->glVAO); 
    glDrawElements(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0);

}


// render a chunk of data
void Renderer::renderChunk(ChunkID id, Chunk* chunk) {

    // start of the chunk
    vec3 chunk_pos(id.X * CHUNK_SIZE, 0, id.Z * CHUNK_SIZE);

    mat4 gPV = gP * gV;

    // now, set the diffuse texture
    shaders["geometry"]->setInt("texDiffuse", 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, Texture::get("../resources/grass.jpg")->glTex);
    glBindVertexArray(mymesh->glVAO); 

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                if (chunk->get(x, y, z).id != ID_NONE) {
                    // do a cube
                    // construct the model matrix
                    mat4 gM = glm::translate(chunk_pos + vec3(x, y, z));

                    // combine all 3 to get PVM
                    mat4 gPVM = gPV * gM;

                    // and set matrices
                    shaders["geometry"]->setMat4("gM", gM);
                    shaders["geometry"]->setMat4("gPVM", gPVM);

                    // draw the actual mesh
                    glDrawElements(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0);

                }
            }
        }
    }

}

void Renderer::renderObj(mat4 gT) {
    // draw mesh

    mat4 gM = glm::translate(vec3(0, 0, 0)) * glm::rotate((float)glfwGetTime(), vec3(0, 1, 0));
    glm::mat4 gPVM = gT * gM;
    glActiveTexture(GL_TEXTURE7); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, Texture::get("../resources/grass.jpg")->glTex);
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


// begin the rendering sequence
void Renderer::render_start() {

    // enable depth testing
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);
    
    // FBO of our rendertarget
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(shaders["geometry"]->glID);

    // draw all these attachments
    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    // calculate the perspective matrix
    gP = glm::perspective(glm::radians(FOV / 2.0f), (float)width / height, 0.25f, 250.0f);

    // calculate the view matrix
    gV = glm::lookAt(pos, pos + forward, up);

    opengl_error_check();

}

// finalize the rendering sequence
void Renderer::render_end() {

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);

    // read from the 'color' render target
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    opengl_error_check();
}


};
