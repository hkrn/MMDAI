/* pmd/zplot.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 transformMatrix;
attribute vec4 inPosition;

void main() {
    vec4 position = modelViewProjectionMatrix * transformMatrix * inPosition;
    gl_Position = position;
}

