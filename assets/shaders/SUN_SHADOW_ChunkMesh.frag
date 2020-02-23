#version 330 core

/* Vertex Shader inputs */

// <x, y, z, w> position (transformed into screen coordinates)
in vec4 fPos;

// <u, v> texture coordinates
in vec2 fUV;

// <x, y, z> normal direction
in vec3 fN;


/* FBO Outputs */

// <r, g, b, a> color output
layout (location = 0) out vec4 gShadowPos;


void main() {

    gShadowPos = vec4(vec3(fPos.z), 1);
    //gShadowPos = vec4(1, 0, 0, 1);

}
