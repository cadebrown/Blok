#version 330 core
/* TextQuad.vert - vertex shader for text quads (i.e. rendering UI text) */

// packed representation of <pos.xy, UV.xy>, not scaled
//   so basically a screen quad
layout (location = 0) in vec4 aPosUV;

// the projection matrix * position matrix for the camera orientation
uniform mat4 gPM;

// fragment shader vars
out vec2 fUV;


void main(){

    // send over the UV
    fUV = aPosUV.zw;

    // update opengl vars
    gl_Position = gPM * vec4(aPosUV.xy, 0.0, 1.0);
}

