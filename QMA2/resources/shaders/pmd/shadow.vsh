/* pmd/shadow.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 shadowMatrix;
attribute vec4 inPosition;

void main() {
    gl_Position = modelViewProjectionMatrix * inPosition;
}

