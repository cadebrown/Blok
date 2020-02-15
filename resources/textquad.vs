#version 330 core

// packed representation of <pos.xy, UV.xy>, not scaled
//   so basically a screen quad
layout (location = 0) in vec4 aPosUV;

// the projection matrix for the camera orientation
uniform mat4 gP;

// fragment shader vars
out vec2 fUV;


void main(){

    // send over the UV
    fUV = aPosUV.zw;

    // update opengl vars
    gl_Position = gP * vec4(aPosUV.xy, 0.0, 1.0);
}

