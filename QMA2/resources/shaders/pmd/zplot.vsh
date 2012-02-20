uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
attribute vec4 inPosition;

void main() {
    vec4 position = projectionMatrix * modelViewMatrix * inPosition;
    gl_Position = position;
}

