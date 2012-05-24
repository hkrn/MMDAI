/* pmx/zplot.vsh */
uniform mat4 modelViewProjectionMatrix;
attribute vec4 inPosition;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(inPosition.xyz, 1.0);
}

