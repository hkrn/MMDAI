/* asset/zplot.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
uniform mat4 modelViewProjectionMatrix;
uniform mat4 transformMatrix;
in vec3 inPosition;
const float kOne = 1.0;

void main() {
    vec4 position = modelViewProjectionMatrix * transformMatrix * vec4(inPosition, kOne);
    gl_Position = position;
}

