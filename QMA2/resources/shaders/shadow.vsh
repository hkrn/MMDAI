uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 shadowMatrix;
attribute vec4 inPosition;

void main() {
    gl_Position = projectionMatrix * modelViewMatrix * shadowMatrix * inPosition;
}

