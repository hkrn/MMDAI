/* Phong shading implementation for asset vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 normalMatrix;
uniform mat4 projectionMatrix;
uniform mat4 transformMatrix;
attribute vec4 inColor;
attribute vec4 inPosition;
attribute vec4 inNormal;
attribute vec2 inTexCoord;
varying vec4 outColor;
varying vec4 outPosition;
varying vec4 outNormal;
varying vec2 outTexCoord;
const float kOne = 1.0;

void main() {
    vec4 position = inPosition;
    mat4 transformedModelViewMatrix = modelViewMatrix * transformMatrix;
    mat4 transformedNormalMatrix = normalMatrix * transformMatrix;
    outPosition = transformedModelViewMatrix * position;
    outNormal = normalize(transformedNormalMatrix * inNormal);
    outColor = inColor;
    outTexCoord = inTexCoord;
    gl_Position = projectionMatrix * transformedModelViewMatrix * position;
}

