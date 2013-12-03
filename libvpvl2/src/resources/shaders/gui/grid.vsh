/* gui/grid.vsh */
#ifdef GL_ES
precision lowp float;
#endif
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
uniform mat4 modelViewProjectionMatrix;
in vec3 inPosition;
in vec3 inColor;
out vec3 outColor;

void main() {
    outColor = inColor;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, 1.0);
}

