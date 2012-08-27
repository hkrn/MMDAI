/* gui/handle.vsh */
#if __VERSION__ < 130
#define in attribute
#endif
uniform mat4 modelViewProjectionMatrix;
uniform mat4 boneMatrix;
in vec4 inPosition;

void main() {
    gl_Position = modelViewProjectionMatrix * boneMatrix * inPosition;
}

