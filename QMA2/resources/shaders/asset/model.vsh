/* asset/model.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat4 transformMatrix;
uniform mat3 normalMatrix;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec3 materialEmission;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
uniform bool hasDepthTexture;
attribute vec3 inPosition;
attribute vec3 inNormal;
attribute vec2 inTexCoord;
varying vec4 outColor;
varying vec4 outTexCoord;
varying vec4 outShadowCoord;
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
    vec3 view = normalize(inPosition);
    vec3 normal = normalize(normalMatrix * inNormal);
    vec4 color = vec4(materialAmbient * lightColor + materialDiffuse.rgb * lightColor, materialDiffuse.a);
    vec2 mainTexCoord = isMainSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    vec2 subTexCoord = isSubSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    outColor = color;
    outTexCoord = vec4(mainTexCoord, subTexCoord);
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * transformMatrix * inPosition;
    gl_Position = modelViewProjectionMatrix * transformMatrix * vec4(inPosition, kOne);
}

