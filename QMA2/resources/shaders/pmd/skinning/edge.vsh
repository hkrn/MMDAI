/* pmd/edge.vsh */
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
attribute vec3 inPosition;
attribute vec3 inNormal; // unused
varying vec4 outColor;
const float kOne = 1.0;
invariant gl_Position;

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
    outColor = color;
    vec4 position = performLinearBlendSkinning(vec4(inPosition, kOne));
    gl_Position = modelViewProjectionMatrix * position;
}

