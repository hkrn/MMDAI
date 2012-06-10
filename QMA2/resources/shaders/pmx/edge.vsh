/* pmx/edge.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
attribute vec4 inPosition;
varying vec4 outColor;

void main() {
    outColor = color;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition.xyz, 1.0);
}

