uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightViewMatrix;
uniform mat4 biasMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightColor;
uniform vec3 materialAmbient;
uniform vec4 materialDiffuse;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonTexCoord;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowTexCoord;
varying vec4 outDepth;
varying vec2 outToonTexCoord;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;
const vec3 kOne3 = vec3(kOne, kOne, kOne);

vec2 makeSphereMap(vec3 position, vec3 normal) {
    vec3 R = reflect(position, normal);
    float M = kTwo * sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2(R.x / M + kHalf, R.y / M + kHalf);
}

void main() {
    vec4 position = modelViewMatrix * inPosition;
    vec4 lightPosition = projectionMatrix * lightViewMatrix * inPosition;
    vec3 view = normalize(position.xyz);
    vec3 normal = normalize(normalMatrix * inNormal);
    vec4 outPosition = projectionMatrix * position;
    vec2 mainTexCoord = isMainSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    vec2 subTexCoord = isSubSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    outColor.rgb = min(materialAmbient + lightColor * materialDiffuse.rgb, kOne3);
    outColor.a = materialDiffuse.a;
    outTexCoord = vec4(mainTexCoord, subTexCoord);
    outShadowTexCoord = biasMatrix * lightPosition;
    outDepth = lightPosition;
    outToonTexCoord = inToonTexCoord;
    gl_Position = outPosition;
}

