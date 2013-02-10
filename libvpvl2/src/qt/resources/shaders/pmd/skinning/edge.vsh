/* pmd/edge.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#endif
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
uniform float edgeWidth;
in vec3 inPosition;
in vec3 inNormal;
in float inEdgeOffset;
out vec4 outColor;
const float kOne = 1.0;
const float kZero = 0.0;

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

void main() {
    outColor = color;
    vec3 position = performLinearBlendSkinning(vec4(inPosition, kOne)).xyz;
    vec3 normal = normalize(performLinearBlendSkinning(vec4(inNormal, kZero)).xyz);
    vec3 edge = (position + normal * edgeWidth) * inEdgeOffset;
    gl_Position = modelViewProjectionMatrix * vec4(edge, kOne);
}

