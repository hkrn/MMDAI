/* asset/zplot.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 transformMatrix;
attribute vec3 inPosition;

void main() {
    vec4 position = modelViewProjectionMatrix * transformMatrix * vec4(inPosition, kOne);
    gl_Position = position;
}

