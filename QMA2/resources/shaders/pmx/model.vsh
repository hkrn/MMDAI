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
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

vec2 makeSphereMap(const vec3 position, const vec3 normal) {
    const float kTwo = 2.0;
    const float kHalf = 0.5;
    vec3 R = reflect(position, normal);
    R.z += kOne;
    float M = kTwo * sqrt(dot(R, R));
    return R.xy / M + kHalf;
}

void main() {
    vec3 position3 = inPosition.xyz;
    vec4 position = vec4(position3, kOne);
    vec3 view = normalize(normalMatrix * position3);
    vec3 lightPosition = normalize(-lightDirection);
    vec3 halfVector = normalize(lightPosition - normalize(position3));
    vec4 color = vec4(materialAmbient, materialDiffuse.a);
    float hdotn = max(dot(halfVector, inNormal), kZero);
    color.rgb += lightColor * materialDiffuse.rgb;
    color.rgb += materialSpecular * pow(hdotn, max(materialShininess, kOne));
    outColor = max(min(color, kOne4), kZero4);
    outTexCoord.xy = inTexCoord;
    outTexCoord.zw = hasSphereTexture ? makeSphereMap(view, inNormal) : inTexCoord;
    outToonCoord = inToonCoord;
    outUVA1 = inUVA1;
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * position;
    gl_Position = modelViewProjectionMatrix * position;
}

