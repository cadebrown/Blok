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
    gl_Position = vec4(aPosition.xy, 0.0, 1.0);
}
