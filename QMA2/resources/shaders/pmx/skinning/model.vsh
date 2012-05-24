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

attribute vec4 inBoneIndices;
attribute vec4 inBoneWeights;
const int kMaxBones = 128;
uniform mat4 boneMatrices[kMaxBones];

vec4 performSkinning(const vec3 position3, const int type) {
    const int kSdef  = 3;
    const int kBdef4 = 2;
    const int kBdef2 = 1;
    const int kBdef1 = 0;
    vec4 position = vec4(position3, 1.0);
    bvec2 bdef2 = bvec2(type == kBdef2, type == kSdef);
    if (type == kBdef4) {
        mat4 matrix1 = boneMatrices[int(inBoneIndices.x)];
        mat4 matrix2 = boneMatrices[int(inBoneIndices.y)];
        mat4 matrix3 = boneMatrices[int(inBoneIndices.z)];
        mat4 matrix4 = boneMatrices[int(inBoneIndices.w)];
        float weight1 = inBoneWeights.x;
        float weight2 = inBoneWeights.y;
        float weight3 = inBoneWeights.z;
        float weight4 = inBoneWeights.w;
        return weight1 * (matrix1 * position) + weight2 * (matrix2 * position)
                       + weight3 * (matrix3 * position) + weight4 * (matrix4 * position);
    }
    else if (any(bdef2)) {
        mat4 matrix1 = boneMatrices[int(inBoneIndices.x)];
        mat4 matrix2 = boneMatrices[int(inBoneIndices.y)];
        float weight = inBoneWeights.x;
        return weight * (matrix1 * position) + (1.0 - weight) * (matrix2 * position);
    }
    else if (type == kBdef1) {
        mat4 matrix = boneMatrices[int(inBoneIndices.x)];
        return matrix * position;
    }
}

vec2 makeSphereMap(const vec3 position, const vec3 normal) {
    vec3 R = reflect(position, normal);
    R.z += kOne;
    float M = kTwo * sqrt(dot(R, R));
    return R.xy / M + kHalf;
}

void main() {
    int type = int(inPosition.w);
    vec4 position = performSkinning(inPosition.xyz, type);
    vec3 normal = performSkinning(inNormal, type).xyz;
    vec3 view = normalize(normalMatrix * position.xyz);
    vec3 lightPosition = normalize(-lightDirection);
    vec3 halfVector = normalize(lightPosition - normalize(position.xyz));
    vec3 color = materialAmbient;
    float hdotn = max(dot(halfVector, normal), 0.0);
    color += lightColor * materialDiffuse.rgb;
    color += materialSpecular * pow(hdotn, max(materialShininess, 1.0));
    outColor.rgb = max(min(color, kOne3), kZero3);
    outColor.a = max(min(materialDiffuse.a, kOne), kZero);
    outTexCoord.xy = inTexCoord;
    outTexCoord.zw = hasSphereTexture ? makeSphereMap(view, normal) : inTexCoord;
    outToonCoord = inToonCoord;
    outUVA1 = inUVA1;
    if (hasDepthTexture)
        outShadowCoord = lightViewProjectionMatrix * position;
    gl_Position = modelViewProjectionMatrix * position;
}

