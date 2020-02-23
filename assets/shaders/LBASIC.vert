#version 330 core
// LBASIC.vert - vertex shader for the 'LBASIC' lighting pass, which operates on a frame buffer
// so, this only is used for a screen space quad

// local vertex position
layout (location = 0) in vec3 aPos;

// UV tex coords
layout (location = 1) in vec2 aUV;

// fragment shader vars
out vec2 fUV;

void main(){

    // just send the UV over
    fUV = aUV;

    // update opengl vars
    gl_Position = vec4(aPos, 1.0);
}

