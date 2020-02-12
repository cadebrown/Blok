#version 330 core

/* position relative to the local model */
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aN;

/* texture coordinates */
layout (location = 2) in vec2 aUV;

out vec4 fPosition;
out vec2 fUV;
out vec3 fN;

uniform mat4 gM;
uniform mat4 gPVM;

void main() {

    // normal transform matrix (i.e. not counting offsets, this can be used for transforming normals)
    mat3 nT = mat3(gPVM);


    // update fragment vars
    fPosition = gPVM * vec4(aPos, 1.0);
    fUV = aUV;
    //fTBN = mat3(nT * aT, nT * aB, nT * aN);
    fN = aN;

    // update opengl vars
    gl_Position = fPosition;

}
