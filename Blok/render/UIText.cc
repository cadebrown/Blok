/* UIText.cc - UI text renderer */

#include <Blok/Render.hh>

namespace Blok::Render {

// construct a UIText object from a given font
UIText::UIText(FontTexture* font) {
    this->font = font;

    // generate the OpenGL handles
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);

    glBindVertexArray(glVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    // there is one attachment: a 4 component vector describing <pos.xy, uv>
    //   of a given 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // disable using this vertex array
    glBindVertexArray(0);

}


}