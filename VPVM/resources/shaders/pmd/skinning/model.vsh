/* pmd/model.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 lightViewProjectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 materialColor;
uniform vec3 cameraPosition;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
uniform bool hasDepthTexture;
in vec3 inPosition;
in vec3 inNormal;
in vec2 inTexCoord;
in vec2 inToonCoord;
out vec4 outColor;
out vec4 outTexCoord;
out vec4 outShadowCoord;
out vec3 outEyeView;
out vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);
const vec4 kZero4 = vec4(kZero, kZero, kZero, kZero);

in vec3 inBoneIndicesAndWeights;
const int kMaxBones = 128;
uniform mat4 boneMatrices[kMaxBones];

vec4 performLinearBlendSkinning(const vec4 position) {
    mat4 matrix1 = boneMatrices[int(inBoneIndicesAndWeights.x)];
    mat4 matrix2 = boneMatrices[int(inBoneIndicesAndWeights.y)];
    float weight = inBoneIndicesAndWeights.z;
    vec4 p1 = matrix2 * position;
    vec4 p2 = matrix1 * position;
    return p1 + (p2 - p1) * weight;
}

vec2 makeSphereMap(const vec3 normal) {
    const float kHalf = 0.5;
    return vec2(normal.x * kHalf + kHalf, normal.y * -kHalf + kHalf);
}

void main() {
    vec4 position = performLinearBlendSkinning(vec4(inPosition, kOne));
    vec3 normal = normalize(performLinearBlendSkinning(vec4(inNormal, kZero)).xyz);
    outEyeView = cameraPosition - inPosition.xyz;
    outNormal = normal;
    outColor = max(min(materialColor, kOne4), kZero4);
    outTexCoord.xy = isMainSphereMap ? makeSphereMap(normal) : inTexCoord;
    outTexCoord.zw = isSubSphereMap ? makeSphereMap(normal) : inTexCoord;
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * position;
    }
    gl_Position = modelViewProjectionMatrix * position;
}

