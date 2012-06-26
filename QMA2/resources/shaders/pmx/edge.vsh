/* pmx/edge.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
attribute vec3 inPosition;
varying vec4 outColor;
const float kOne = 1.0;
invariant gl_Position;

void main() {
    outColor = color;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

