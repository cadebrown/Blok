#version 330 core

/* inputs from vertex shader */
in vec4 fPosition;
in vec2 fUV;
in vec3 fN;
in float fBlockID;

/* outputs to frame buffer */
layout (location = 0) out vec4 gColor;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gUV;
layout (location = 3) out vec4 gNormal;

uniform sampler2D texDiffuse;
uniform sampler2D texDiffuse2;
uniform sampler2D texDiffuse3;

void main() {

    // get the color

    vec4 col;
    
    if (fBlockID == 1.0) {
        col = texture(texDiffuse2, fUV);
    } else if (fBlockID == 2.0) {
        col = texture(texDiffuse3, fUV);
    } else {
        col = texture(texDiffuse, fUV);
    }

    vec3 N = normalize(fN);
    vec3 ldir = normalize(vec3(.5, -1, 1));

    gColor = col * (.3 + .7 * max(0, -dot(N, ldir)));
    gPosition = fPosition;
    gUV = vec4(fUV, 1.0f, 0.0f);
    gNormal = vec4(N, 0.0f);
    //gColor = texture(texDiffuse, fUV);
    //gPosition = fPosition;
    //gUV = vec4(fUV, 0.0f, 0.0f);
    //gNormal = vec4(fTBN[2], 0.0f);
}