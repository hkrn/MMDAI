/* asset/zplot.vsh */
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 transformMatrix;
attribute vec3 inPosition;
const float kOne = 1.0;

void main() {
    vec4 position = modelViewProjectionMatrix * transformMatrix * vec4(inPosition, kOne);
    gl_Position = position;
}

