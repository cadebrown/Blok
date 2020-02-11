#version 330 core

/* position relative to the local model */
layout (location = 0) in vec3 aPosition;

/* texture coordinates */
layout (location = 1) in vec2 aUV;

/* Tangent/Bitangent/Normal direction vectors */
layout (location = 2) in vec3 aT;
layout (location = 3) in vec3 aB;
layout (location = 4) in vec3 aN;

out vec4 fPosition;
out vec2 fUV;
out mat3 fTBN;

uniform mat4 gM;
uniform mat4 gPVM;

void main() {

    // normal transform matrix (i.e. not counting offsets, this can be used for transforming normals)
    mat3 nT = mat3(gPVM);

    // update fragment vars
    fPosition = gPVM * vec4(aPosition.xyz, 1.0);
    fUV = aUV;
    fTBN = mat3(nT * aT, nT * aB, nT * aN);

    // update opengl vars
    gl_Position = fPosition;


}
