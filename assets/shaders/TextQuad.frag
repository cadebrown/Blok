#version 330 core
/* TextQuad.frag - fragment shader for UI text */

// input from vertex shader
in vec2 fUV;

// outputs to frame buffers
layout (location = 0) out vec4 gColor;

// the source texture atlas for the font
uniform sampler2D texFont;

void main() {
    vec4 cur = texture(texFont, fUV);

    // disregard transparent parts
    if (cur.a < 0.1) cur = vec4(0);

    gColor = cur;
}
