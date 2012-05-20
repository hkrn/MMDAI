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
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;
const vec3 kOne3 = vec3(kOne, kOne, kOne);

attribute vec2 inBoneIndices;
attribute float inBoneWeight;
const int kMaxBones = 64;
uniform mat4 boneMatrices[kMaxBones];

vec4 performLinearBlendSkinning(const vec4 position, const float weight, const mat4 matrix1, const mat4 matrix2) {
    return weight * (matrix1 * position) + (1.0 - weight) * (matrix2 * position);
}

vec2 makeSphereMap(const vec3 position, const vec3 normal) {
    vec3 R = reflect(position, normal);
    R.z += kOne;
    float M = kTwo * sqrt(dot(R, R));
    return R.xy / M + kHalf;
}

void main() {
    mat4 matrix1 = boneMatrices[int(inBoneIndices.x)];
    mat4 matrix2 = boneMatrices[int(inBoneIndices.y)];
    vec4 position = performLinearBlendSkinning(inPosition, inBoneWeight, matrix1, matrix2);
    vec3 normal = normalize(performLinearBlendSkinning(vec4(inNormal, 0), inBoneWeight, matrix1, matrix2).xyz);
    vec3 view = normalize(normalMatrix * position.xyz);
    vec3 lightPosition = normalize(-lightDirection);
    vec3 halfVector = normalize(lightPosition - normalize(position.xyz));
    vec3 color = materialAmbient;
    float hdotn = max(dot(halfVector, normal.xyz), 0.0);
    color += lightColor * materialDiffuse.rgb;
    color += materialSpecular * pow(hdotn, max(materialShininess, 1.0));
    outColor.rgb = color;
    outColor.a = materialDiffuse.a;
    outTexCoord.xy = isMainSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    outTexCoord.zw = isSubSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    outToonCoord = inToonCoord;
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * position;
    }
    gl_Position = modelViewProjectionMatrix * position;
}

