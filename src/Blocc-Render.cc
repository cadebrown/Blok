/* Blocc-Render.cc - rendering code */

#include <Blocc-Render.hh>

namespace Blocc::Render {

void Mesh::setup() {
    // create buffers/arrays
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);

    glBindVertexArray(glVAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint), &faces[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers

    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    // vertex texture coords
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    // vertex tangent
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, T));
    // vertex bitangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, B));
    // vertex normals
    glEnableVertexAttribArray(4);	
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, N));

    glBindVertexArray(0);
}

Target::Target(int width, int height, int numTex) {
    this->width = width;
    this->height = height;

    // create a frame buffer object to render to
    glGenFramebuffers(1, &glFBO); 
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glFBO);

    glTex.resize(numTex);
    glColorAttachments.resize(numTex);

    // Create the gbuffer textures
    glGenTextures(numTex, &glTex[0]);
    glGenTextures(1, &glDepth);

    for (unsigned int i = 0 ; i < numTex; i++) {
        glBindTexture(GL_TEXTURE_2D, glTex[i]);

        // default to RGBA in float format, to avoid any potential issues
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

        // and just allocate color attachments starting at 0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, glTex[i], 0);
        glColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    // depth buffer render object
    glBindTexture(GL_TEXTURE_2D, glDepth);
    // initialize it
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // set it as the depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glDepth, 0);

    // draw on all these bufers
    glDrawBuffers(numTex, &glColorAttachments[0]);

    // make sure we were sucecssful
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        b_error("While calling glCheckFramebufferStatus(), got 0x%x", Status);
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


};
