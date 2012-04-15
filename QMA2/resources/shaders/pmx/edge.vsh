uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute float inEdgeSize;
varying vec4 outColor;

void main() {
    outColor = color;
    vec3 position = inPosition.xyz + inNormal * inEdgeSize * 0.03;
    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
}

