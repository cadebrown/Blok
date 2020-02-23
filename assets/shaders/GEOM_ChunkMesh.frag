#version 330 core

/* Vertex Shader inputs */

// <x, y, z, w> position (transformed into screen coordinates)
in vec4 fPos;

// <u, v> texture coordinates
in vec2 fUV;

// <x, y, z> normal direction
in vec3 fN;

// <id> the (int) block ID that is currently being rendered
in float fBlockID;

// <x, y, z, 0> wolrd position
in vec4 fWPos;

/* FBO Outputs */

// <r, g, b, a> color output
layout (location = 0) out vec4 gColor;

// <x, y, z, w> screen position output
layout (location = 1) out vec4 gPos;

// <u, v, 0, 0> texture coordinate output
layout (location = 2) out vec4 gUV;

// <x, y, z, 0> output normal direction
layout (location = 3) out vec4 gNormal;

// <x, y, z, 0> world position
layout (location = 4) out vec4 gWPos;




/* Constants */

// textures for various blocks
// TODO: use a texture atlas class
uniform sampler2D texID1;
uniform sampler2D texID2;
uniform sampler2D texID3;

void main() {

    // sample 'col' as the given block
    vec4 col = vec4(1, 0, 0, 1);
    //col = texture(texID3, fUV);

    // check various constants
    // TODO: texture atlas
    if (fBlockID < 1.1) {
        col = texture(texID1, fUV);
    } else if (fBlockID < 2.1) {
        col = texture(texID2, fUV);
    } else if (fBlockID < 3.1) {
        col = texture(texID3, fUV);
    } else {
        discard;
    }


    // make sure N is a unit vector
    vec3 N = normalize(fN);


/*
    // amount of ambient light/sun light
    float la_amb = 0.5, la_sun = 1.0;

    // simulate a light direction
    // TODO: add uniform support for lighting
    // TODO: move this shader to a deferred pass, have lighting happen later
    vec3 ldir = normalize(vec3(.5, -1, 1));
*/

    // now, write outputs to the FBO

    // compute color
    //gColor = col * (la_amb + la_sun * max(0, -dot(N, ldir)));
    gColor = col;
    gPos = fPos;
    gUV = vec4(fUV, 0.0f, 0.0f);
    gNormal = vec4(N, 0.0f);
    gWPos = fWPos;
    
    if (fPos.w < 0.1) gWPos.w = 1.0;
    else gWPos.w = fPos.z / 256.0;
    gWPos.w = max(0, min(1, gWPos.w)) + 1;

    //vec4 owpos = fWPos;

    //if (fWPos.w < 0.1) owpos.w = 100.0;
    //else owpos.w = fPos.z;
    //owpos.w = min(1, max(0, owpos.w));

   // gWPos = owpos;

}
