uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
attribute vec2 inPosition;
attribute vec2 inTexCoord;
varying vec2 outTexCoord;

void main() {
    outTexCoord = inTexCoord;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(inPosition, 0.0, 1.0);
}

