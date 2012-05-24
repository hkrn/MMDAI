/* pmx/model.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewInverseMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialDiffuse;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 materialAmbient;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform bool hasSphereTexture;
uniform bool hasDepthTexture;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonCoord;
attribute vec4 inUVA1;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec2 outToonCoord;
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
    R.z += kOne;
    float M = kTwo * sqrt(dot(R, R));
    return R.xy / M + kHalf;
}

void main() {
    vec3 position3 = inPosition.xyz;
    vec4 position = vec4(position3, 1.0);
    vec3 view = normalize(normalMatrix * position3);
    vec3 lightPosition = normalize(-lightDirection);
    vec3 halfVector = normalize(lightPosition - normalize(position3));
    vec3 color = materialAmbient;
    float hdotn = max(dot(halfVector, inNormal), 0.0);
    color += lightColor * materialDiffuse.rgb;
    color += materialSpecular * pow(hdotn, max(materialShininess, 1.0));
    outColor.rgb = max(min(color, kOne3), kZero3);
    outColor.a = max(min(materialDiffuse.a, kOne), kZero);
    outTexCoord.xy = inTexCoord;
    outTexCoord.zw = hasSphereTexture ? makeSphereMap(view, inNormal) : inTexCoord;
    outToonCoord = inToonCoord;
    outUVA1 = inUVA1;
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * position;
    gl_Position = modelViewProjectionMatrix * position;
}

