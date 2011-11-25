const int kBoneIndices = 128;
uniform mat4 boneMatrices[kBoneIndices];
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 color;
attribute vec4 inPosition;
attribute vec4 inNormal;
attribute vec3 inBoneAttributes;
attribute float inEdgeOffset;
varying vec4 outColor;

vec4 doSkinning(vec4 position, mat4 matrix1, mat4 matrix2) {
    float weight = inBoneAttributes.z;
    return weight * (matrix1 * position) + (1.0 - weight) * (matrix2 * position);
}

void main() {
    mat4 matrix1 = boneMatrices[int(inBoneAttributes.x)];
    mat4 matrix2 = boneMatrices[int(inBoneAttributes.y)];
    vec4 position = modelViewMatrix * doSkinning(inPosition, matrix1, matrix2);
    vec4 normal = normalize(modelViewMatrix * doSkinning(inNormal, matrix1, matrix2));
    outColor = color;
    gl_Position = projectionMatrix * (position + normal * inEdgeOffset);
}

