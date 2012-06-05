/* pmd/model.vsh */
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
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
uniform bool hasDepthTexture;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
attribute vec2 inToonCoord;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec2 outToonCoord;
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
    vec3 view = normalize(normalMatrix * inPosition.xyz);
    vec3 lightPosition = normalize(-lightDirection);
    vec3 halfVector = normalize(lightPosition - normalize(inPosition.xyz));
    vec4 color = vec4(materialAmbient, materialDiffuse.a);
    float hdotn = max(dot(halfVector, inNormal), kZero);
    color.rgb += lightColor * materialDiffuse.rgb;
    color.rgb += materialSpecular * pow(hdotn, max(materialShininess, kOne));
    outColor = max(min(color, kOne4), kZero4);
    outTexCoord.xy = isMainSphereMap ? makeSphereMap(view, inNormal) : inTexCoord;
    outTexCoord.zw = isSubSphereMap ? makeSphereMap(view, inNormal) : inTexCoord;
    outToonCoord = inToonCoord;
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * inPosition;
    }
    gl_Position = modelViewProjectionMatrix * inPosition;
}

