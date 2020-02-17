#version 330 core
/* DebugLine.vert - basic line vert shader for debug-style lines */

/* VAO/VBO Inputs */

// the input position of the line, in 3D space
layout (location = 0) in vec3 aPos;

// the color
layout (location = 1) in vec3 aCol;

/* Fragment Shader Outputs */

// the screen position
out vec4 fPos;
out vec4 fCol;

/* Globals */

// the Projection * View matrix
uniform mat4 gPV;

void main() {
    // calculate transformed position
    fPos = gPV * vec4(aPos, 1.0);

    fCol = vec4(aCol, 1.0);

    // update opengl vars
    gl_Position = fPos;

}
