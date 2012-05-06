/* pmx/model.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewInverseMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialDiffuse;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 materialAmbient;
uniform bool hasSphereTexture;
uniform bool hasDepthTexture;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec4 inUVA1;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec2 outToonTexCoord;
varying vec4 outShadowCoord;
varying vec4 outUVA1;
varying vec4 outUVA2;
varying vec4 outUVA3;
varying vec4 outUVA4;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;
const float kZero = 0.0;
const vec3 kOne3 = vec3(kOne, kOne, kOne);
const vec3 kZero3 = vec3(kZero, kZero, kZero);

vec2 makeSphereMap(const vec3 position, const vec3 normal) {
    vec3 R = reflect(position, normal);
    float M = kTwo * sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2(R.x / M + kHalf, R.y / M + kHalf);
}

void main() {
    vec3 view = normalize(normalMatrix * inPosition.xyz);
    vec3 normal = normalize(normalMatrix * inNormal);
    outColor.rgb = max(min(materialAmbient + lightColor * materialDiffuse.rgb, kOne3), kZero3);
    outColor.a = max(min(materialDiffuse.a, kOne), kZero);
    outTexCoord = vec4(inTexCoord, hasSphereTexture ? makeSphereMap(view, normal) : inTexCoord);
    outToonTexCoord = vec2(0, 1.0 - dot(normalize(lightDirection), normal) * 0.5);
    outUVA1 = inUVA1;
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * inPosition;
    gl_Position = modelViewProjectionMatrix * inPosition;
}

