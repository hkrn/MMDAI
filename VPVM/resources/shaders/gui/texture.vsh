/* gui/texture.vsh */
uniform mat4 modelViewProjectionMatrix;
attribute vec2 inPosition;
attribute vec2 inTexCoord;
varying vec2 outTexCoord;

void main() {
    outTexCoord = inTexCoord;
    gl_Position = modelViewProjectionMatrix * vec4(inPosition, 0.0, 1.0);
}

