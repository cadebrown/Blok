#version 330 core

// input from vertex shader
in vec2 fUV;

// outputs to frame buffers
layout (location = 0) out vec4 gColor;

// the source texture atlas for the font
uniform sampler2D texFont;

void main() {
    gColor = texture(texFont, fUV);
    //gColor = vec4(1.0, 0.0, 0.0, 1.0);
}
