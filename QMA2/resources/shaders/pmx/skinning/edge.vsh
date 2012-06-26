/* pmx/edge.vsh */
invariant gl_Position;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 color;
uniform float edgeSize;
attribute vec4 inPosition;
attribute vec3 inNormal;
attribute float inEdgeSize;
varying vec4 outColor;

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

void main() {
    outColor = color;
    int type = int(inPosition.w);
    vec3 position = performSkinning(inPosition.xyz, type).xyz;
    vec3 normal = performSkinning(inNormal, type).xyz;
    vec3 edge = position + normal * inEdgeSize * edgeSize * 0.03;
    gl_Position = modelViewProjectionMatrix * vec4(edge, 0.0); // disabled
}

