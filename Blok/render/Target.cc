/* Target.cc - implementation of a rendering target, i.e. a result texture */

#include <Blok/Render.hh>

namespace Blok::Render {

// construct a new rendering target with a given size, and number of individual images
Target::Target(int width, int height, int numTex) {
    this->width = width;
    this->height = height;

    // create a frame buffer object to render to
    glGenFramebuffers(1, &glFBO); 
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glFBO);

    // set their size to the number of targets we want
    glTex.resize(numTex);
    glColorAttachments.resize(numTex);

    // Create the gbuffer textures
    glGenTextures(numTex, &glTex[0]);
    glGenTextures(1, &glDepth);

    // initialize all the other textures
    for (int i = 0 ; i < numTex; i++) {
        // initialize the texture
        glBindTexture(GL_TEXTURE_2D, glTex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, glTex[i], 0);
        glColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    // generate depth buffer
    glBindTexture(GL_TEXTURE_2D, glDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glDepth, 0);

    //glDrawBuffers(numTex, &glColorAttachments[0]);

    // make sure we were sucecssful
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        blok_error("While calling glCheckFramebufferStatus(), got 0x%x", Status);
        return;
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // check for any errors
    check_GL();

}

}
