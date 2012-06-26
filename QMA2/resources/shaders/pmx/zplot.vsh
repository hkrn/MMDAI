/* pmx/zplot.vsh */
uniform mat4 modelViewProjectionMatrix;
attribute vec3 inPosition;
const float kOne = 1.0;
invariant gl_Position;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

