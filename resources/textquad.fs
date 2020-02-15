#version 330 core

// input from vertex shader
in vec2 fUV;

// outputs to frame buffers
layout (location = 0) out vec4 gColor;

// the source texture atlas for the font
uniform sampler2D texFont;

void main() {
    vec4 cur = texture(texFont, fUV);
    if (cur.a < 0.1) cur = vec4(0);

    gColor = cur;
    //gColor = vec4(1.0, 0.0, 0.0, 1.0);
}
