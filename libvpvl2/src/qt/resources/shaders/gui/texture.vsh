/* gui/texture.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
uniform mat4 modelViewProjectionMatrix;
in vec2 inPosition;
in vec2 inTexCoord;
out vec2 outTexCoord;

void main() {
    outTexCoord = inTexCoord;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, 0.0, 1.0);
}

