uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
attribute vec4 inPosition;

void main() {
    gl_Position = projectionMatrix * modelViewMatrix * inPosition;
}

