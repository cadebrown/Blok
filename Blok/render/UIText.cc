/* UIText.cc - UI text renderer */

#include <Blok/Render.hh>

namespace Blok::Render {

// construct a UIText object from a given font
UIText::UIText(FontTexture* font) {
    this->font = font;

    maxWidth = 800.0;

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


// recalculate the VBO object
void UIText::calcVBO() {

    // scale of the float
    float scale = 0.2f;

    float gap = 0.0f;

    // current positions
    float x = gap, y = - (gap + 64 * scale);

    // create a list of <x, y, u, v> coordinates for each vertex
    List<vec4> xyuv_s = {};

    List<Face> faces = {};

    int ct = 0;

    for (char c : text) {

        // special case for a line break
        if (c == '\n') {
            x = gap;
            y -= 128 * scale;
            continue;
        }

        FontTexture::CharInfo ch = font->charInfos[c];

        // get the size requested
        vec2i size = ch.texStop - ch.texStart;

        if (x + size.x * scale >= maxWidth - gap) {
            x = gap;
            y -= 128 * scale;
        }

        float xpos = x + ch.bearing.x * scale;

        float ypos = y - (size.y - ch.bearing.y) * scale;

        // get width/height units
        float w = size.x * scale;
        float h = size.y * scale;

        // convert to UV coordinates
        vec2 uvStart = vec2(ch.texStart) / vec2(font->width, font->height);
        vec2 uvStop = vec2(ch.texStop) / vec2(font->width, font->height);

        // add 2 triangles to make up a quad

        // first tri
        xyuv_s.push_back({ xpos, ypos, uvStart.x, uvStart.y });
        xyuv_s.push_back({ xpos, ypos+h, uvStart.x, uvStop.y });
        xyuv_s.push_back({ xpos+w, ypos, uvStop.x, uvStart.y });

        // second tri
        xyuv_s.push_back({ xpos, ypos+h, uvStart.x, uvStop.y });
        xyuv_s.push_back({ xpos+w, ypos+h, uvStop.x, uvStop.y });
        xyuv_s.push_back({ xpos+w, ypos, uvStop.x, uvStart.y });

        // add the advance amount (i.e. space between starts)
        //   to the X coordinate
        x += ch.advance * scale / 64;

    }


    tris = xyuv_s.size();
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * xyuv_s.size(), &xyuv_s[0], GL_STATIC_DRAW); 
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}


}