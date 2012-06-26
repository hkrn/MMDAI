/* pmx/model.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialColor;
uniform vec3 cameraPosition;
uniform bool hasSphereTexture;
uniform bool hasDepthTexture;
attribute vec4 inUVA1;
attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonCoord;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec4 outUVA1;
varying vec3 outEyeView;
varying vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);
invariant gl_Position;

vec2 makeSphereMap(const vec3 normal) {
    const float kHalf = 0.5;
    return vec2(normal.x * kHalf + kHalf, normal.y * kHalf + kHalf);
}

void main() {
    vec4 position = vec4(inPosition, kOne);
    vec3 view = normalize(normalMatrix * inPosition);
    vec3 normal = normalMatrix * inNormal;
    outEyeView = cameraPosition - inPosition;
    outNormal = inNormal;
    outColor = max(min(materialColor, kOne4), kZero4);
    outTexCoord.xy = inTexCoord;
    outTexCoord.zw = hasSphereTexture ? makeSphereMap(normal) : inTexCoord;
    outUVA1 = inUVA1;
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * position;
    }
    gl_Position = modelViewProjectionMatrix * position;
}

