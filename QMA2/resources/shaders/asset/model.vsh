/* asset/model.vsh */
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialDiffuse;
uniform vec3 materialColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
uniform bool hasDepthTexture;
attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec3 outEyeView;
varying vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

vec2 makeSphereMap(vec3 normal) {
    const float kHalf = 0.5;
    return vec2(normal.x * kHalf + kHalf, normal.y * -kHalf + kHalf);
}

void main() {
    vec3 view = normalize(normalMatrix * inPosition);
    vec3 normal = normalize(normalMatrix * inNormal);
    vec4 position = vec4(inPosition, kOne);
    float ldotn = max(dot(normal, -lightDirection), 0.0);
    vec4 color = vec4(materialColor + ldotn * materialDiffuse.rgb, materialDiffuse.a);
    outEyeView = cameraPosition - inPosition;
    outNormal = inNormal;
    outColor = max(min(color, kOne4), kZero4);
    outTexCoord.xy = isMainSphereMap ? makeSphereMap(normal) : inTexCoord;
    outTexCoord.zw = isSubSphereMap ? makeSphereMap(normal) : inTexCoord;
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * position;
    gl_Position = modelViewProjectionMatrix * position;
}

