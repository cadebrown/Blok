#version 330 core


// local vertex position
layout (location = 0) in vec3 aPos;

// UV tex coords
layout (location = 1) in vec2 aUV;

// Tangent, Bitangent, Normal space
layout (location = 2) in vec3 aT;
layout (location = 3) in vec3 aB;
layout (location = 4) in vec3 aN;

// fragment shader vars
out vec4 fPosition;
out vec2 fUV;
out mat3 fTBN;

uniform mat4 gM;
uniform mat4 gPV;

void main() {

    // normal transform matrix (i.e. not counting offsets, this can be used for transforming normals)
    mat3 nT = mat3(gM);

    // calculate transformed position
    fPosition = gPV * gM * vec4(aPos, 1.0);

    // just send the UV over
    fUV = aUV;

    // send over the tangent space
    fTBN = mat3(nT * aT, nT * aB, nT * aN);

    // update opengl vars
    gl_Position = fPosition;

}
