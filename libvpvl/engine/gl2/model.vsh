/* Phong shading implementation for model vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 normalMatrix;
uniform mat4 projectionMatrix;
uniform bool hasSingleSphereMap;
uniform bool hasMultipleSphereMap;
attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonTexCoord;
varying vec3 outPosition;
varying vec3 outNormal;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;

vec2 makeSphereMap(vec3 position, vec3 normal) {
    vec3 R = reflect(position, normal);
    float M = kTwo + sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2((R.x / M + kHalf), (R.y / M + kHalf));
}

void main() {
    vec4 position4 = vec4(inPosition, kOne);
    outPosition = vec3(modelViewMatrix * position4);
    outNormal = vec3(normalize(normalMatrix * vec4(inNormal, kOne)));
    outMainTexCoord = hasSingleSphereMap ? makeSphereMap(outPosition, outNormal) : inTexCoord;
    outSubTexCoord = hasMultipleSphereMap ? makeSphereMap(outPosition, outNormal) : inTexCoord;
    outToonTexCoord = inToonTexCoord;
    gl_Position = projectionMatrix * modelViewMatrix * position4;
}

