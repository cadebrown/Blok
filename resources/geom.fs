#version 330 core

/* inputs from vertex shader */
in vec4 fPosition;
in vec2 fUV;
in mat3 fTBN;

/* outputs to frame buffer */
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gUV;
layout (location = 3) out vec4 gNormal;

//uniform sampler2D texDiffuse;

void main() {
    gColor = vec4(1.0f, 0.0f, 0.4f, 1.0f);
    gPosition = vec4(1.0f, 0.0f, 0.4f, 1.0f);
    gUV = vec4(1.0f, 0.0f, 0.4f, 1.0f);
    gNormal = vec4(1.0f, 0.0f, 0.4f, 1.0f);
    //gColor = texture(texDiffuse, fUV);
    //gPosition = fPosition;
    //gUV = vec4(fUV, 0.0f, 0.0f);
    //gNormal = vec4(fTBN[2], 0.0f);
}
