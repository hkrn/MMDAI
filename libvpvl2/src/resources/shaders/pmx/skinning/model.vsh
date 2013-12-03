/* pmx/model.vsh */
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
uniform bool hasSphereTexture;
uniform bool hasDepthTexture;
in vec4 inPosition;
in vec4 inUVA1;
in vec3 inNormal;
in vec2 inTexCoord;
out vec4 outColor;
out vec4 outTexCoord;
out vec4 outShadowCoord;
out vec4 outShadowPosition;
out vec4 outUVA1;
out vec3 outEyeView;
out vec3 outNormal;
const float kOne = 1.0;
const float kZero = 0.0;
const vec4 kOne4 = vec4(kOne);
const vec4 kZero4 = vec4(kZero);

const int kQdef  = 4;
const int kSdef  = 3;
const int kBdef4 = 2;
const int kBdef2 = 1;
const int kBdef1 = 0;

in vec4 inBoneIndices;
in vec4 inBoneWeights;
const int kMaxBones = 50;
uniform mat4 boneMatrices[kMaxBones];

vec4 performSkinning(const vec3 position3, const float base, const int type) {
    vec4 position = vec4(position3, base);
    bool bdef4 = any(bvec2(type == kBdef4, type == kQdef));
    bool bdef2 = any(bvec2(type == kBdef2, type == kSdef));
    if (bdef4) {
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
    else if (bdef2) {
        mat4 matrix1 = boneMatrices[int(inBoneIndices.x)];
        mat4 matrix2 = boneMatrices[int(inBoneIndices.y)];
        float weight = inBoneWeights.x;
        vec4 p1 = matrix2 * position;
        vec4 p2 = matrix1 * position;
        return p1 + (p2 - p1) * weight;
    }
    else if (type == kBdef1) {
        mat4 matrix = boneMatrices[int(inBoneIndices.x)];
        return matrix * position;
    }
    else {
        return position;
    }
}

vec2 makeSphereMap(const vec3 normal) {
    const float kHalf = 0.5;
    return vec2(normal.x * -kHalf, normal.y * -kHalf);
}

void main() {
    int type = int(inPosition.w);
    vec4 position = performSkinning(inPosition.xyz, 1.0, type);
    vec3 normal = normalize(performSkinning(inNormal, 0.0, type).xyz);
    outEyeView = cameraPosition - position.xyz;
    outNormal = normal;
    outColor = clamp(materialColor, kZero4, kOne4);
    outTexCoord.xy = inTexCoord;
    outTexCoord.zw = hasSphereTexture ? makeSphereMap(normal) : inTexCoord;
    outUVA1 = inUVA1;
    if (hasDepthTexture) {
        outShadowCoord = lightViewProjectionMatrix * position;
        outShadowPosition = modelViewProjectionMatrix * position;
    }
    gl_Position = modelViewProjectionMatrix * position;
}

