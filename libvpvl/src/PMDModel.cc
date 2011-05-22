#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDModel.h"

namespace vpvl
{

const float PMDModel::kMinBoneWeight = 0.0001f;
const float PMDModel::kMinFaceWeight = 0.001f;

PMDModel::PMDModel()
    : m_boundingSphereStep(kBoundingSpherePointsMin),
      m_edgeOffset(0.03f),
      m_selfShadowDensityCoef(0.0f),
      m_enableSimulation(true)
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
    m_rootBone.setCurrentRotation(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_rootBone.updateTransform();
}

PMDModel::~PMDModel()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
    m_vertices.clear();
    m_indices.clear();
    m_bones.clear();
    m_IKs.clear();
    m_faces.clear();
    m_rigidBodies.clear();
    m_constraints.clear();
    m_skinningTransform.clear();
    m_skinnedVertices.clear();
    m_edgeVertices.clear();
    m_edgeIndices.clear();
    m_toonTextureCoords.clear();
    m_rotatedBones.clear();
    m_isIKSimulated.clear();
}

void PMDModel::prepare()
{
    m_skinningTransform.reserve(m_bones.size());
    int nVertices = m_vertices.size();
    m_skinnedVertices.reserve(nVertices);
    m_edgeVertices.reserve(nVertices);
    m_toonTextureCoords.reserve(nVertices);
    uint32_t from = 0, to = 0;
    int nMaterials = m_materials.size();
    for (int i = 0; i < nMaterials; i++) {
        Material &material = m_materials[i];
        uint32_t nindices = material.countIndices();
        if (material.isEdgeEnabled()) {
            for (uint32_t j = 0; j < nindices; j++) {
                m_edgeIndices[from + j] = m_indices[from + j];
            }
            to += nindices;
        }
        from += nindices;
    }
    int nBones = m_bones.size();
    for (int i = 0; i < nBones; i++) {
        BoneType type = m_bones[i].type();
        if (type == kUnderRotate || type == kFollowRotate)
            m_rotatedBones.push_back(i);
    }
    int nIKs = m_IKs.size();
    m_isIKSimulated.reserve(nIKs);
    for (int i = 0; i < nIKs; i++) {
        m_isIKSimulated[i] = m_IKs[i].isSimulated();
    }
    m_boundingSphereStep = nVertices / kBoundingSpherePoints;
    uint32_t max = kBoundingSpherePointsMax;
    uint32_t min = kBoundingSpherePointsMin;
    btClamp(m_boundingSphereStep, max, min);
}

void PMDModel::updateBone()
{
    int nBones = m_bones.size(), nIKs = m_IKs.size();
    /* TODO: ordered bone list  */
    for (int i = 0; i < nBones; i++)
        m_bones[i].updateTransform();
    if (m_enableSimulation) {
        for (int i = 0; i < nIKs; i++) {
            if (!m_isIKSimulated[i])
                m_IKs[i].solve();
        }
    }
    else {
        for (int i = 0; i < nIKs; i++)
            m_IKs[i].solve();
    }
    int nRotatedBones = m_rotatedBones.size();
    for (int i = 0; i < nRotatedBones; i++)
        m_bones[m_rotatedBones[i]].updateRotation();
}

void PMDModel::updateBoneFromSimulation()
{
    int nRigidBodies = m_rigidBodies.size();
    for (int i = 0; i < nRigidBodies; i++)
        m_rigidBodies[i].transformToBone();
}

void PMDModel::updateFace()
{
    int nFaces = m_faces.size();
    for (int i = 0; i < nFaces; i++) {
        Face &face = m_faces[i];
        if (face.type() == kBase) {
            face.applyToVertices(m_vertices);
            break;
        }
    }
    for (int i = 0; i < nFaces; i++) {
        Face &face = m_faces[i];
        float weight = face.weight();
        if (weight > kMinFaceWeight)
            face.addToVertices(m_vertices, weight);
    }
}

void PMDModel::updateShadow(float coef)
{
    bool update = false;
    int nVertices = m_vertices.size();
    if (m_shadowTextureCoords.size() == 0) {
        m_shadowTextureCoords.reserve(nVertices);
        update = true;
    }
    else if (coef) {
        update = true;
    }
    if (update) {
        for (int i = 0; i < nVertices; i++)
            m_shadowTextureCoords[i].setValue(0.0f, coef, 0.0f);
        m_selfShadowDensityCoef = coef;
    }
}

void PMDModel::updateSkin()
{
    int nBones = m_bones.size();
    for (int i = 0; i < nBones; i++)
        m_bones[i].getSkinTransform(m_skinningTransform[i]);
    int nVertices = m_vertices.size();
    for (int i = 0; i < nVertices; i++) {
        Vertex &vertex = m_vertices[i];
        SkinVertex *skin = &m_skinnedVertices[i];
        const float weight = vertex.weight();
        if (weight >= 1.0f - kMinBoneWeight) {
            const int16_t bone1 = vertex.bone1();
            skin->position = m_skinningTransform[bone1] * vertex.position();
            skin->normal = m_skinningTransform[bone1].getBasis() * vertex.normal();
        }
        else if (weight <= kMinBoneWeight) {
            const int16_t bone2 = vertex.bone2();
            skin->position = m_skinningTransform[bone2] * vertex.position();
            skin->normal = m_skinningTransform[bone2].getBasis() * vertex.normal();
        }
        else {
            const int16_t bone1 = vertex.bone1();
            const int16_t bone2 = vertex.bone2();
            const btVector3 v1 = m_skinningTransform[bone1] * vertex.position();
            const btVector3 n1 = m_skinningTransform[bone1].getBasis() * vertex.normal();
            const btVector3 v2 = m_skinningTransform[bone2] * vertex.position();
            const btVector3 n2 = m_skinningTransform[bone2].getBasis() * vertex.normal();
            skin->position = v2.lerp(v1, weight);
            skin->normal = n2.lerp(n1, weight);
        }
    }
}

void PMDModel::updateToon(const btVector3 &light)
{
    int nVertices = m_vertices.size();
    for (int i = 0; i < nVertices; i++) {
        m_toonTextureCoords[i].setValue(0.0f, (1.0f - light.dot(m_skinnedVertices[i].normal)) * 0.5f, 0.0f);
        SkinVertex skin = m_skinnedVertices[i];
        if (m_vertices[i].isEdgeEnabled())
            m_edgeVertices[i] = skin.position;
        else
            m_edgeVertices[i] = skin.position + skin.normal * m_edgeOffset;
    }
}

float PMDModel::boundingSphereRange(btVector3 &center)
{
    float max = 0.0f;
    Bone *bone = &Bone::centerBone(&m_bones);
    btVector3 pos = bone->transform().getOrigin();
    int nVertices = m_vertices.size();
    for (int i = 0; i < nVertices; i++) {
        const float r2 = pos.distance2(m_skinnedVertices[i].position);
        if (max < r2)
            max = r2;
    }
    max = sqrtf(max) * 1.1f;
    center = pos;
    return max;
}

void PMDModel::smearAllBonesToDefault(float rate)
{
    static const btVector3 v(0.0f, 0.0f, 0.0f);
    static const btQuaternion q(0.0f, 0.0f, 0.0f, 1.0f);
    int nBones = m_bones.size(), nFaces = m_faces.size();
    for (int i = 0; i < nBones; i++) {
        Bone &bone = m_bones[i];
        btVector3 tmpv = bone.currentPosition();
        m_bones[i].setCurrentPosition(v.lerp(tmpv, rate));
        btQuaternion tmpq = bone.currentRotation();
        m_bones[i].setCurrentRotation(q.slerp(tmpq, rate));
    }
    for (int i = 0; i < nFaces; i++) {
        Face &face = m_faces[i];
        face.setWeight(face.weight() * rate);
    }
}

}
