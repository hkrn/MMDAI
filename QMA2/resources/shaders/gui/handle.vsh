/* gui/handle.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 boneMatrix;
attribute vec4 inPosition;

void main() {
    gl_Position = modelViewProjectionMatrix * boneMatrix * inPosition;
}

