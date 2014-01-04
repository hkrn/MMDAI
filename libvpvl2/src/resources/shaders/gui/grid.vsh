/* gui/grid.vsh */
#if defined(GL_ES) || __VERSION__ >= 150
precision lowp float;
#endif
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
uniform mat4 modelViewProjectionMatrix;
in vec4 inColor;
in vec3 inPosition;
out vec4 outColor;

void main() {
    outColor = inColor;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, 1.0);
}
