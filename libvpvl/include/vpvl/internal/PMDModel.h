#ifndef VPVL_PMDMODEL_H_
#define VPVL_PMDMODEL_H_

#include "vpvl/vpvl.h"

namespace vpvl
{

struct PMDModelDataInfo
{
    const char *basePtr;
    const char *namePtr;
    const char *commentPtr;
    const char *verticesPtr;
    size_t verticesCount;
    const char *indicesPtr;
    size_t indicesCount;
    const char *materialsPtr;
    size_t materialsCount;
    const char *bonesPtr;
    size_t bonesCount;
    const char *IKsPtr;
    size_t IKsCount;
    const char *facesPtr;
    size_t facesCount;
    const char *faceDisplayNamesPtr;
    size_t faceDisplayNamesCount;
    const char *boneFrameNamesPtr;
    size_t boneFrameNamesCount;
    const char *boneDisplayNamesPtr;
    size_t boneDisplayNamesCount;
    const char *englishNamePtr;
    const char *englishCommentPtr;
    const char *toonTextureNamesPtr;
    const char *rigidBodiesPtr;
    size_t rigidBodiesCount;
    const char *constraintsPtr;
    size_t constranitsCount;
};

struct SkinVertex
{
    btVector3 position;
    btVector3 normal;
    btVector3 texureCoord;
};

class PMDModel
{
public:
    PMDModel(const char *data, size_t size);
    ~PMDModel();

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

    bool preparse();
    bool parse();

    const char *name() const {
        return m_name;
    }
    const char *comment() const {
        return m_comment;
    }
    const char *englishName() const {
        return m_englishName;
    }
    const char *englishComment() const {
        return m_englishComment;
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
    const PMDModelDataInfo &result() const {
        return m_result;
    }
    const char *data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
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
    void parseHeader();
    void parseVertices();
    void parseIndices();
    void parseMatrials();
    void parseBones();
    void parseIKs();
    void parseFaces();
    void parseFaceDisplayNames();
    void parseBoneDisplayNames();
    void parseEnglishDisplayNames();
    void parseToonTextureNames();
    void parseRigidBodies();
    void parseConstraints();

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
    PMDModelDataInfo m_result;
    btAlignedObjectArray<btTransform> m_skinningTransform;
    btAlignedObjectArray<SkinVertex> m_skinnedVertices;
    btAlignedObjectArray<btVector3> m_edgeVertices;
    btAlignedObjectArray<uint16_t> m_edgeIndices;
    btAlignedObjectArray<btVector3> m_toonTextureCoords;
    btAlignedObjectArray<btVector3> m_shadowTextureCoords;
    btAlignedObjectArray<uint16_t> m_rotatedBones;
    btAlignedObjectArray<bool> m_isIKSimulated;
    size_t m_size;
    const char *m_data;
    uint32_t m_boundingSphereStep;
    float m_edgeOffset;
    float m_selfShadowDensityCoef;
    bool m_enableSimulation;
};

}

#endif
