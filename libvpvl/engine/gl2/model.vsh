/* Phong shading implementation for vertex shader */

uniform mat4 modelViewMatrix;
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

vec2 makeSphereMap(vec3 position, vec3 normal) {
    vec3 R = reflect(position, normal);
    float M = 2.0 + sqrt(R.x * R.x + R.y * R.y + (R.z + 1.0) * (R.z + 1.0));
    return vec2((R.x / M + 0.5), (R.y / M + 0.5));
}

void main() {
    vec4 position4 = vec4(inPosition, 1.0);
    outPosition = vec3(modelViewMatrix * position4);
    outNormal = inNormal; // normalize(transpose(inverse(mat3(modelViewMatrix))), inNormal);
    outMainTexCoord = hasSingleSphereMap ? makeSphereMap(outPosition, outNormal) : inTexCoord;
    outSubTexCoord = hasMultipleSphereMap ? makeSphereMap(outPosition, outNormal) : inTexCoord;
    outToonTexCoord = inToonTexCoord;
    gl_Position = projectionMatrix * modelViewMatrix * position4;
}

