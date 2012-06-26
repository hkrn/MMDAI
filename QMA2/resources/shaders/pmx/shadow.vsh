/* pmx/shadow.vsh */
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
attribute vec3 inPosition;
const float kOne = 1.0;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

