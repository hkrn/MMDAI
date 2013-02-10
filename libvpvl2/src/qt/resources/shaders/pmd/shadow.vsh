/* pmd/shadow.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
//invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 shadowMatrix;
in vec3 inPosition;
const float kOne = 1.0;

void main() {
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

