/* pmx/shadow.vsh */
#if __VERSION__ < 130
#define in attribute
#endif
//invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
in vec3 inPosition;
const float kOne = 1.0;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

