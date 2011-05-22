#ifndef VPVL_PMDMODEL_H_
#define VPVL_PMDMODEL_H_

#include "vpvl/vpvl.h"

namespace vpvl
{

struct SkinVertex
{
    btVector3 position;
    btVector3 normal;
    btVector3 texureCoord;
};

class PMDModel : public IModel
{
public:
    PMDModel();
    virtual ~PMDModel();

    static const uint32_t kBoundingSpherePoints = 1000;
    static const uint32_t kBoundingSpherePointsMax = 20;
    static const uint32_t kBoundingSpherePointsMin = 5;
    static const float kMinBoneWeight;
    static const float kMinFaceWeight;

    void prepare();
    void updateBone();
    void updateBoneFromSimulation();
    void updateFace();
    void updateShadow(float coef);
    void updateSkin();
    void updateToon(const btVector3 &light);
    float boundingSphereRange(btVector3 &center);
    void smearAllBonesToDefault(float rate);

    const char *name() const {
        return m_name;
    }
    const char *comment() const {
        return m_comment;
    }
    const char *englishName() const {
        return m_name;
    }
    const char *englishComment() const {
        return m_comment;
    }
    const VertexList &vertices() const {
        return m_vertices;
    }
    const IndexList &indices() const {
        return m_indices;
    }
    const MaterialList &materials() const {
        return m_materials;
    }
    const BoneList &bones() const {
        return m_bones;
    }
    const IKList &IKs() const {
        return m_IKs;
    }
    const FaceList &faces() const {
        return m_faces;
    }
    const RigidBodyList &rigidBodies() const {
        return m_rigidBodies;
    }
    const ConstraintList &constraints() const {
        return m_constraints;
    }
    BoneList *mutableBones() {
        return &m_bones;
    }
    const char *toonTexture(unsigned int index) const {
        if (index >= 10)
            return NULL;
        return m_textures[index];
    }
    const Bone &rootBone() const {
        return m_rootBone;
    }
    Bone *mutableRootBone() {
        return &m_rootBone;
    }
    float edgeOffset() const {
        return m_edgeOffset;
    }
    bool isSimulationEnabled() const {
        return m_enableSimulation;
    }

    void setName(const char *value) {
        vpvlStringCopySafe(m_name, value, sizeof(m_name));
    }
    void setComment(const char *value) {
        vpvlStringCopySafe(m_comment, value, sizeof(m_comment));
    }
    void setEnglishName(const char *value) {
        vpvlStringCopySafe(m_englishName, value, sizeof(m_englishName));
    }
    void setEnglishComment(const char *value) {
        vpvlStringCopySafe(m_englishComment, value, sizeof(m_englishComment));
    }
    void setVertices(const VertexList &value) {
        m_vertices = value;
    }
    void setIndices(const IndexList &value) {
        m_indices = value;
    }
    void setMaterials(const MaterialList &value) {
        m_materials = value;
    }
    void setBones(const BoneList &value) {
        m_bones = value;
    }
    void setIKs(const IKList &value) {
        m_IKs = value;
    }
    void setFaces(const FaceList &value) {
        m_faces = value;
    }
    void setRigidBodies(const RigidBodyList &value) {
        m_rigidBodies = value;
    }
    void setConstraints(const ConstraintList &value) {
        m_constraints = value;
    }
    void setToonTextures(const char *ptr) {
        char *p = const_cast<char *>(ptr);
        for (int i = 0; i < 10; i++) {
            vpvlStringCopySafe(m_textures[i], ptr, sizeof(m_textures[i]));
            p += 100;
        }
    }
    void setEdgeOffset(float value) {
        m_edgeOffset = value * 0.03f;
    }
    void setEnableSimulation(bool value) {
        m_enableSimulation = value;
    }

private:
    char m_name[20];
    char m_comment[256];
    char m_englishName[20];
    char m_englishComment[256];
    char m_textures[10][100];
    VertexList m_vertices;
    IndexList m_indices;
    MaterialList m_materials;
    BoneList m_bones;
    IKList m_IKs;
    FaceList m_faces;
    RigidBodyList m_rigidBodies;
    ConstraintList m_constraints;
    Bone m_rootBone;
    btAlignedObjectArray<btTransform> m_skinningTransform;
    btAlignedObjectArray<SkinVertex> m_skinnedVertices;
    btAlignedObjectArray<btVector3> m_edgeVertices;
    btAlignedObjectArray<uint16_t> m_edgeIndices;
    btAlignedObjectArray<btVector3> m_toonTextureCoords;
    btAlignedObjectArray<btVector3> m_shadowTextureCoords;
    btAlignedObjectArray<uint16_t> m_rotatedBones;
    btAlignedObjectArray<bool> m_isIKSimulated;
    uint32_t m_boundingSphereStep;
    float m_edgeOffset;
    float m_selfShadowDensityCoef;
    bool m_enableSimulation;
};

}

#endif
