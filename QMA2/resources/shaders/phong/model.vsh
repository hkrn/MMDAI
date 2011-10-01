/* Phong shading implementation for model vertex shader */

uniform mat4 modelViewMatrix;
uniform mat4 normalMatrix;
uniform mat4 projectionMatrix;
uniform bool hasSingleSphereMap;
uniform bool hasMultipleSphereMap;
attribute vec4 inPosition;
attribute vec4 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonTexCoord;
varying vec4 outPosition;
varying vec4 outNormal;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;

vec2 makeSphereMap(vec4 position, vec4 normal) {
    vec3 R = reflect(position.xyz, normal.xyz);
    float M = kTwo + sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2((R.x / M + kHalf), (R.y / M + kHalf));
}

void main() {
    vec4 position = inPosition;
    vec4 normal = normalize(normalMatrix * inNormal);
    outPosition = modelViewMatrix * position;
    outNormal = normal;
    outMainTexCoord = hasSingleSphereMap ? makeSphereMap(position, normal) : inTexCoord;
    outSubTexCoord = hasMultipleSphereMap ? makeSphereMap(position, normal) : inTexCoord;
    outToonTexCoord = inToonTexCoord;
    gl_Position = projectionMatrix * modelViewMatrix * position;
}

