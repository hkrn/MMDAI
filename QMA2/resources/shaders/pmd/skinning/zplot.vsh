/* pmd/zplot.vsh */
uniform mat4 modelViewProjectionMatrix;
attribute vec4 inPosition;

attribute vec3 inBoneIndicesAndWeights;
const int kMaxBones = 128;
uniform mat4 boneMatrices[kMaxBones];

vec4 performLinearBlendSkinning(const vec4 position) {
    mat4 matrix1 = boneMatrices[int(inBoneIndicesAndWeights.x)];
    mat4 matrix2 = boneMatrices[int(inBoneIndicesAndWeights.y)];
    float weight = inBoneIndicesAndWeights.z;
    return weight * (matrix1 * position) + (1.0 - weight) * (matrix2 * position);
}

void main() {
    vec4 position = performLinearBlendSkinning(inPosition);
    gl_Position = modelViewProjectionMatrix * position;
}

