#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in vec3 aNormal;

out vec4 fPosition;
out vec2 fUV;
out mat3 fTBN;

uniform mat4 gM;
uniform mat4 gPVM;


void main() {
    vec4 aPosV4 = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
    fPosition = gM * aPosV4;
    fUV = aUV;
    fTBN = mat3(aTangent, aBitangent, aNormal);
    gl_Position = gPVM * aPosV4;
}
