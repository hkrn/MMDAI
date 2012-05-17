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
attribute vec2 inToonTexCoord;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
varying vec2 outToonTexCoord;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;
const vec3 kOne3 = vec3(kOne, kOne, kOne);

vec2 makeSphereMap(const vec3 position, const vec3 normal) {
    vec3 R = reflect(position, normal);
    float M = kTwo * sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2(R.x / M + kHalf, R.y / M + kHalf);
}

void main() {
    vec3 view = normalize(normalMatrix * inPosition.xyz);
    vec3 lightPosition = normalize(-lightDirection);
    vec3 halfVector = normalize(lightPosition - normalize(inPosition.xyz));
    vec3 color = materialAmbient;
    float hdotn = max(dot(halfVector, inNormal), 0.0);
    color += lightColor * materialDiffuse.rgb;
    color += materialSpecular * pow(hdotn, max(materialShininess, 1.0));
    outColor.rgb = color;
    outColor.a = materialDiffuse.a;
    outTexCoord.xy = isMainSphereMap ? makeSphereMap(view, inNormal) : inTexCoord;
    outTexCoord.zw = isSubSphereMap ? makeSphereMap(view, inNormal) : inTexCoord;
    outToonTexCoord = vec2(0, 1.0 + dot(lightPosition, inNormal) * 0.5);
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * inPosition;
    }
    gl_Position = modelViewProjectionMatrix * inPosition;
}

