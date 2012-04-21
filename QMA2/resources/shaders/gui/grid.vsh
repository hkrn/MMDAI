/* gui/grid.vsh */
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
attribute vec3 inPosition;
attribute vec3 inColor;
varying vec3 outColor;

void main() {
    outColor = inColor;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(inPosition, 1.0);
}

