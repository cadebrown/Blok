#version 330 core

// local vertex position
layout (location = 0) in vec3 aPos;

// UV tex coords
layout (location = 1) in vec2 aUV;

// Tangent, Bitangent, Normal space
layout (location = 2) in vec3 aT;
layout (location = 3) in vec3 aB;
layout (location = 4) in vec3 aN;

// Instance values (block position in the world, and ID)
layout (location = 5) in vec3 gBlockPos;
layout (location = 6) in float gBlockID;

out vec4 fPosition;
out vec2 fUV;
out vec3 fN;
out float fBlockID;

//uniform mat4 gM;
uniform mat4 gPV;

void main() {
    // calculate transformed position
    fPosition = gPV * vec4(aPos + gBlockPos, 1.0);

    // just send the UV over
    fUV = aUV;

    // send the normal over (TODO: include just the model scaling here)
    fN = aN;

    // send the block ID over
    fBlockID = gBlockID;

    // update opengl vars
    gl_Position = fPosition;

}
