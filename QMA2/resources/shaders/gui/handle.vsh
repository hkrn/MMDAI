/* gui/handle.vsh */
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 boneMatrix;
attribute vec4 inPosition;

void main() {
    gl_Position = projectionMatrix * modelViewMatrix * boneMatrix * inPosition;
}

