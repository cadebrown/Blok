#version 330 core

/* VAO/VBO Inputs */

// local vertex position of the model
layout (location = 0) in vec3 aPos;

// UV coordinates of the vertex
layout (location = 1) in vec2 aUV;

// Tangent, Bitangent, Normal space of the vertex
layout (location = 2) in vec3 aT;
layout (location = 3) in vec3 aB;
layout (location = 4) in vec3 aN;

/* Fragment Shader Outputs */

// the screen position
out vec4 fPos;
// the texture coordinates
out vec2 fUV;
// the normal direction
out vec3 fN;


/* Globals */

// the Projection * View matrix
uniform mat4 gPV;

void main() {
    // calculate transformed position
    fPos = gPV * vec4(aPos, 1.0);

    // just send the UV over
    fUV = aUV;

    // send the normal over (TODO: include just the model scaling here)
    fN = aN;

    // update opengl vars
    gl_Position = fPos;

}
