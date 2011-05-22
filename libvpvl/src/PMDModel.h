#include "vpvl/vpvl.h"

namespace vpvl
{

class PMDModel : public IModel
{
public:
    PMDModel();
    virtual ~PMDModel();

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
};

}
