/* Target.cc - implementation of a rendering target, i.e. a result texture */

#include <Blok/Render.hh>

namespace Blok::Render {

// construct a new rendering target with a given size, and number of individual images
Target::Target(int width, int height, int numTex) {

    // resize our arrays
    glTex.resize(numTex);
    glColorAttachments.resize(numTex);

    // create all buffers
    glGenFramebuffers(1, &glFBO); 
    glGenTextures(1, &glDepth);
    glGenTextures(numTex, &glTex[0]);

    // ensure size is set
    resize(width, height);

    // initialize all the other textures as related to the framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glFBO);

    for (int i = 0; i < numTex; i++) {
        // set up the texture for the first time
        glBindTexture(GL_TEXTURE_2D, glTex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // add it to the frame buffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, glTex[i], 0);
        glColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;

    }

    // generate depth buffer
    glBindTexture(GL_TEXTURE_2D, glDepth);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glDepth, 0);

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

void Target::resize(int w, int h) {
    // if nothing has changed, do nothing
    if (w == width && h == height) return;

    // set the new values
    width = w;
    height = h;

    // resize all attachments
    for (uint i = 0; i < glTex.size(); i++) {
        // resize the texture, erasing existng data
        glBindTexture(GL_TEXTURE_2D, glTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    }

    // recreate depth buffer
    glBindTexture(GL_TEXTURE_2D, glDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

}


}
