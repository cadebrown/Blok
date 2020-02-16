#version 330 core

// local vertex position
layout (location = 0) in vec3 aPos;

// UV tex coords
layout (location = 1) in vec2 aUV;

// fragment shader vars
out vec2 fUV;

// global transform matrix
uniform mat4 gT;

void main(){

    // just send the UV over
    fUV = aUV;

    // update opengl vars
    gl_Position = gT * vec4(aPos, 1.0);
}

