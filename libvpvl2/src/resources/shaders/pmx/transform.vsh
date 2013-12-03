/* pmx/transform.vsh */
#if __VERSION__ < 130
#define in attribute
#define out varying
#define texture texture2D
#endif
in vec4 vpvl2_inPosition;
in vec4 vpvl2_inNormal;
in vec4 vpvl2_inBoneIndices;
in vec4 vpvl2_inBoneWeights;
out vec4 vpvl2_outNormal;
uniform float numBoneIndices;
uniform sampler2D matrixPalette;

const int kQdef  = 4;
const int kSdef  = 3;
const int kBdef4 = 2;
const int kBdef2 = 1;
const int kBdef1 = 0;

mat4 fetchBoneMatrix(const float index) {
    float newIndex = index / numBoneIndices;
    mat4 matrix = mat4(
        texture(matrixPalette, vec2(0.0,  newIndex)),
        texture(matrixPalette, vec2(0.25, newIndex)),
        texture(matrixPalette, vec2(0.5,  newIndex)),
        texture(matrixPalette, vec2(0.75, newIndex))
    );
    return matrix;
}

vec4 performSkinning(const vec3 position3, const float base, const int type) {
    vec4 position = vec4(position3, base);
    bool bdef4 = any(bvec2(type == kBdef4, type == kQdef));
    bool bdef2 = any(bvec2(type == kBdef2, type == kSdef));
    if (bdef4) {
        mat4 matrix1 = fetchBoneMatrix(vpvl2_inBoneIndices.x);
        mat4 matrix2 = fetchBoneMatrix(vpvl2_inBoneIndices.y);
        mat4 matrix3 = fetchBoneMatrix(vpvl2_inBoneIndices.z);
        mat4 matrix4 = fetchBoneMatrix(vpvl2_inBoneIndices.w);
        return vpvl2_inBoneWeights.xxxx * (matrix1 * position)
             + vpvl2_inBoneWeights.yyyy * (matrix2 * position)
             + vpvl2_inBoneWeights.zzzz * (matrix3 * position)
             + vpvl2_inBoneWeights.wwww * (matrix4 * position);
    }
    else if (bdef2) {
        mat4 matrix1 = fetchBoneMatrix(vpvl2_inBoneIndices.x);
        mat4 matrix2 = fetchBoneMatrix(vpvl2_inBoneIndices.y);
        vec4 p1 = matrix2 * position;
        vec4 p2 = matrix1 * position;
        return mix(p1, p2, vpvl2_inBoneWeights.x);
    }
    else if (type == kBdef1) {
        mat4 matrix = fetchBoneMatrix(vpvl2_inBoneIndices.x);
        return matrix * position;
    }
    else {
        return position;
    }
}

void main() {
    vec4 position   = performSkinning(vpvl2_inPosition.xyz, 1.0, int(vpvl2_inPosition.w));
    vec4 normal     = vec4(performSkinning(vpvl2_inNormal.xyz, 0.0, int(vpvl2_inNormal.w)).xyz, 0.0);
    vpvl2_outNormal = normal;
    gl_Position     = position;
}

