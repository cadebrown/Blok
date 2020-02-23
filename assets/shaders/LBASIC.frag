#version 330 core
// LBASIC.frag - LBASIC lighting pass fragment shader

// input from vertex shader
in vec2 fUV;

// outputs to frame buffers
layout (location = 0) out vec4 gColor;


// screen position texture
uniform sampler2D texWPos;


// the depth map from the light's perspective
uniform sampler2D texSunShadow;


// the diffuse texture from the geometry pass
uniform sampler2D texDiffuse;

// the normal texture from the geometry pass
uniform sampler2D texNormal;

// inverse matrix transform for the sun
uniform mat4 gPV_sun;

void main() {

    // 1: capture diffuse/normal textures
    vec4 wpos = texture(texWPos, fUV);
    vec4 diff = texture(texDiffuse, fUV);

    vec3 N = normalize(texture(texNormal, fUV).xyz);

    // 2: do lighting calculations
    // amount of ambient light/sun light
    float la_amb = 0.5, la_sun = 1.0;

    // simulate a light direction
    // TODO: add uniform support for lighting
    // TODO: move this shader to a deferred pass, have lighting happen later
    vec3 ldir = normalize(vec3(.5, -1, 1));

    // so, now, calculate the sun's contribution
    vec4 sun_col = diff * (la_amb + la_sun * max(0, -dot(N, ldir)));


    // 3: do depth fog
    float dep = wpos.w - 1;
    if (dep < 0) dep = 1;
    dep = dep * 0.45;



/*
    vec4 shadowCoord = gPV_sun * vec4(wpos.xyz, 1);
    float vis = 1.0;

    if (texture(texSunShadow, (1+shadowCoord.xy)/2).z < shadowCoord.z-.1) {
        vis = 0.15;
    }
    //vis = shadowCoord.z;
    //vis = texture(texSunShadow, (1+shadowCoord.xy)/2).z;
*/

    // compute color
    gColor = sun_col * (1 - dep) + vec4(dep, dep, dep, 1);
    //gColor = vec4(N, 1.0);
    //gColor = vec4(vec3(wpos.w), 1.0);
    //gColor = vis * diff;
    //gColor = vec4(vec3(sunDep), 1);

}
