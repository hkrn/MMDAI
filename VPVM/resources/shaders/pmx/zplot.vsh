/* pmx/zplot.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
//invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
in vec3 inPosition;
out vec4 outPosition;
const float kOne = 1.0;

void main() {
    vec4 position = modelViewProjectionMatrix * vec4(inPosition, kOne);
    outPosition = position;
    gl_Position = position;
}

