/* pmx/edge.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
//invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
in vec3 inPosition;
out vec4 outColor;
const float kOne = 1.0;

void main() {
    outColor = color;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, kOne);
}

