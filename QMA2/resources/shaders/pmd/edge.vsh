uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 color;
attribute vec3 inPosition;
attribute vec3 inNormal; // unused
varying vec4 outColor;

void main() {
    outColor = color;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(inPosition, 1);
}

