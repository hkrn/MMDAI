/* gui/grid.vsh */
uniform mat4 modelViewProjectionMatrix;
attribute vec3 inPosition;
attribute vec3 inColor;
varying vec3 outColor;

void main() {
    outColor = inColor;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, 1.0);
}

