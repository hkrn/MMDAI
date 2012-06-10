/* pmd/shadow.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 shadowMatrix;
attribute vec3 inPosition;
const float kOne = 1.0;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

