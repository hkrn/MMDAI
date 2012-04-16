uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;
attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
varying vec3 outPosition;
varying vec3 outNormal;
varying vec2 outTexCoord;

void main() {
    vec4 position = modelViewProjectionMatrix * vec4(inPosition, 1.0);
    outPosition = position.xyz;
    outNormal = normalMatrix * inNormal;
    outTexCoord = inTexCoord;
    gl_Position = position;
}

