uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 inColor;
attribute vec3 inPosition;
attribute vec3 inNormal;
varying vec4 outColor;

void main() {
    outColor = inColor;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(inPosition, 1);
}

