#version 330 core

// input from vertex shader
in vec2 fUV;

// outputs to frame buffers
layout (location = 0) out vec4 gColor;

// the source texture
uniform sampler2D texSrc;
// the position texture
uniform sampler2D texPos;
uniform float time;

void main() {
    //gColor = vec4(fUV, texture(texSrc, vec2(.5, .5)).x, 1.0);
    vec2 px = vec2(1/1280.0, 0);
    vec2 py = vec2(0, 1/800.0);
    //vec4 sm = texture(texSrc, fUV) + .3 * (texture(texSrc, fUV + px)+texture(texSrc, fUV + px+py)+texture(texSrc, fUV + py)+texture(texSrc, fUV - px + py)+texture(texSrc, fUV - px)+texture(texSrc, fUV - px - py)+texture(texSrc, fUV - py)+texture(texSrc, fUV + px - py)) / 8;

    /*vec4 sm = texture(texSrc, fUV) - 1.0 * 
        (texture(texSrc, fUV + px)+texture(texSrc, fUV + px+py)+texture(texSrc, fUV + py)+texture(texSrc, fUV - px + py)+texture(texSrc, fUV - px)+texture(texSrc, fUV - px - py)+texture(texSrc, fUV - py)+texture(texSrc, fUV + px - py))/8;
    gColor = 10 * sm;*/
    //gColor = sm;
    //gColor = texture(texSrc, fUV + vec2(.01 * sin(30.0 * fUV.y + 1.2 * time), .01 * sin(30.0 * fUV.x + .8 * time)));

    // fog
    vec4 posSample = texture(texPos, fUV);
    float dep = posSample.z;
    dep /= 256;

    if (posSample.w < 0.1) dep = 1.0;

    //if (dep < 0.00001) dep = 1;
    dep = min(1, max(0, dep));
    //dep = (exp(4.0 * dep) - 1) / (exp(4.0) - 1);

    dep = dep * 0.45;
    gColor = vec4(texture(texSrc, fUV)) * (1 - dep) + vec4(dep, dep, dep, 1.0);
    //gColor = texture(texSrc, fUV/2) + texture(texSrc, fUV/2 + px);
    //gColor = texture(texSrc, fUV) + texture(texSrc, fUV + vec2(.01, .01));
    //gColor = texture(texSrc, fUV + 0.005 * vec2(sin(time+1024.0*fUV.x),cos(time+768.0*fUV.y)) );
}
