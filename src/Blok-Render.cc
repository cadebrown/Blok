/* Blok-Render.cc - rendering code */

#include <Blok-Render.hh>

// assimp libraries, for asset importing
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Blok::Render {


void Renderer::renderObj(mat4 gT) {
        // draw mesh

    mat4 gM = glm::translate(vec3(0, 0, 6.5f)) * glm::rotate((float)glfwGetTime(), vec3(0, 1, 0));
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

void Renderer::render() {

    // enable depth testing
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);
    
    // FBO of our rendertarget
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(shaders["geometry"]->glID);


    // draw all these attachments
    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    opengl_error_check();

    // clear it

    /* RENDERING */

    // perspective and view matrices
    mat4 gP = glm::perspective(glm::radians(120.0f / 2.0f), (float)width / height, 0.1f, 100.0f);
    //gP[2] *= -1.0f;

    mat4 gV = glm::lookAt(vec3(0, 0, 10), vec3(0, 0, 0), vec3(0, 1, 0));

   //gV = glm::inverse(gV);


    mat4 gPV = gP * gV;

    // renders each 
    //scene.traverse<WireframeRenderer, &WireframeRenderer::travRenderGameObject>(this);
    renderObj(gPV);
    
    /*for (int i = 0; i < mymesh->vertices.size(); ++i) {
        printf("%lf,%lf,%lf\n", mymesh->vertices[i].pos.x, mymesh->vertices[i].pos.y, , mymesh->vertices[i].pos.z);
    }*/

    // clean up and post to the main output

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);

    // read from the 'color' render target
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    opengl_error_check();

}



};
