/* gui/handle.vsh */
#if defined(GL_ES) || __VERSION__ >= 150
precision highp float;
#endif
#if __VERSION__ < 130
#define in attribute
#endif
uniform mat4 modelViewProjectionMatrix;
in vec4 inPosition;

void main() {
    gl_Position = modelViewProjectionMatrix * inPosition;
}

