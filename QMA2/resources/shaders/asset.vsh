/* Phong shading implementation for asset vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 normalMatrix;
uniform mat4 projectionMatrix;
uniform mat4 transformMatrix;
attribute vec4 inColor;
attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
varying vec4 outColor;
varying vec3 outPosition;
varying vec3 outNormal;
varying vec2 outTexCoord;
const float kOne = 1.0;

void main() {
    vec4 position4 = vec4(inPosition, kOne);
    mat4 transformedModelViewMatrix = modelViewMatrix * transformMatrix;
    mat4 transformedNormalMatrix = normalMatrix * transformMatrix;
    outPosition = (transformedModelViewMatrix * position4).xyz;
    outNormal = normalize(transformedNormalMatrix * vec4(inNormal, kOne)).xyz;
    outColor = inColor;
    outTexCoord = inTexCoord;
    gl_Position = projectionMatrix * transformedModelViewMatrix * position4;
}

