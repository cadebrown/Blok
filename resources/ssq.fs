#version 330 core

// input from vertex shader
in vec2 fUV;

// outputs to frame buffers
layout (location = 0) out vec4 gColor;

// the source texture
uniform sampler2D texSrc;
uniform float time;

void main() {
    //gColor = vec4(fUV, texture(texSrc, vec2(.5, .5)).x, 1.0);
    vec2 px = vec2(1/1280.0, 0);
    vec2 py = vec2(0, 1/800.0);
    //vec4 sm = texture(texSrc, fUV) + .3 * (texture(texSrc, fUV + px)+texture(texSrc, fUV + px+py)+texture(texSrc, fUV + py)+texture(texSrc, fUV - px + py)+texture(texSrc, fUV - px)+texture(texSrc, fUV - px - py)+texture(texSrc, fUV - py)+texture(texSrc, fUV + px - py)) / 8;

    /*vec4 sm = texture(texSrc, fUV) - 1.0 * 
        (texture(texSrc, fUV + px)+texture(texSrc, fUV + px+py)+texture(texSrc, fUV + py)+texture(texSrc, fUV - px + py)+texture(texSrc, fUV - px)+texture(texSrc, fUV - px - py)+texture(texSrc, fUV - py)+texture(texSrc, fUV + px - py))/8;
    gColor = 100 * sm;*/
    //gColor = sm;
    //gColor = texture(texSrc, fUV + vec2(.01 * sin(30.0 * fUV.y + .3 * time), .01 * sin(30.0 * fUV.x + .12 * time)));
    gColor = texture(texSrc, fUV);
    //gColor = texture(texSrc, fUV/2) + texture(texSrc, fUV/2 + px);
    //gColor = texture(texSrc, fUV) + texture(texSrc, fUV + vec2(.01, .01));
    //gColor = texture(texSrc, fUV + 0.005 * vec2(sin(time+1024.0*fUV.x),cos(time+768.0*fUV.y)) );
}
