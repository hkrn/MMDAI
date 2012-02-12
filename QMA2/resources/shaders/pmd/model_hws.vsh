const int kBoneIndices = 128;
uniform mat4 boneMatrices[kBoneIndices];
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform vec4 lightColor;
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec3 lightPosition;
uniform bool isMainSphereMap;
uniform bool isSubSphereMap;
attribute vec4 inPosition;
attribute vec4 inNormal;
attribute vec3 inBoneAttributes;
attribute vec2 inTexCoord;
varying vec4 outColor;
varying vec4 outShadowTexCoord;
varying vec2 outMainTexCoord;
varying vec2 outSubTexCoord;
varying vec2 outToonTexCoord;
const float kTwo = 2.0;
const float kOne = 1.0;
const float kHalf = 0.5;
const vec4 kOne4 = vec4(kOne, kOne, kOne, kOne);

vec4 doSkinning(vec4 position, mat4 matrix1, mat4 matrix2) {
    float weight = inBoneAttributes.z;
    return weight * (matrix1 * position) + (1.0 - weight) * (matrix2 * position);
}

vec2 makeSphereMap(vec3 position, vec3 normal) {
    vec3 R = reflect(position, normal);
    float M = kTwo * sqrt(R.x * R.x + R.y * R.y + (R.z + kOne) * (R.z + kOne));
    return vec2(R.x / M + kHalf, R.y / M + kHalf);
}

void main() {
    mat4 matrix1 = boneMatrices[int(inBoneAttributes.x)];
    mat4 matrix2 = boneMatrices[int(inBoneAttributes.y)];
    vec4 position = modelViewMatrix * doSkinning(inPosition, matrix1, matrix2);
    vec3 view = normalize(position.xyz);
    vec3 normal = normalize(normalMatrix * doSkinning(inNormal, matrix1, matrix2).xyz);
    vec4 color = min(materialAmbient + lightColor * materialDiffuse, kOne4);
    vec4 outPosition = projectionMatrix * position;
    outColor = color;
    outShadowTexCoord = kOne4;
    outMainTexCoord = isMainSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    outSubTexCoord = isSubSphereMap ? makeSphereMap(view, normal) : inTexCoord;
    outToonTexCoord = vec2(0.0, dot(lightPosition, normal) * 0.5);
    gl_Position = outPosition;
}

