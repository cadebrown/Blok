#version 330 core
/* DebugLine.frag - basic debugging line fragment shader */


/* Vertex Shader inputs */

// <x, y, z, w> position (transformed into screen coordinates)
in vec4 fPos;
in vec4 fCol;

/* FBO Outputs */

// <r, g, b, a> color output
layout (location = 0) out vec4 gColor;

void main() {

    // color it red by default
    gColor = fCol;

}
