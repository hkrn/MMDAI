#include "vpvl/vpvl.h"
#include "PMDModel.h"

namespace vpvl
{

struct ParserResult
{
    char *basePtr;
    char *namePtr;
    char *commentPtr;
    char *verticesPtr;
    size_t verticesCount;
    char *indicesPtr;
    size_t indicesCount;
    char *materialsPtr;
    size_t materialsCount;
    char *bonesPtr;
    size_t bonesCount;
    char *IKsPtr;
    size_t IKsCount;
    char *facesPtr;
    size_t facesCount;
    char *faceDisplayNamesPtr;
    size_t faceDisplayNamesCount;
    char *boneFrameNamesPtr;
    size_t boneFrameNamesCount;
    char *boneDisplayNamesPtr;
    size_t boneDisplayNamesCount;
    char *englishDisplayNamesPtr;
    char *toonTextureNamesPtr;
    char *rigidBodiesPtr;
    size_t rigidBodiesCount;
    char *constraintsPtr;
    size_t constranitsCount;
};

class PMDParser : public IModelParser
{
public:
    PMDParser(const char *data, size_t size);
    virtual ~PMDParser();

    virtual bool preparse();
    virtual IModel *parse();

private:
    void parseHeader(PMDModel *model);
    void parseVertices(PMDModel *model);
    void parseIndices(PMDModel *model);
    void parseMatrials(PMDModel *model);
    void parseBones(PMDModel *model);
    void parseIKs(PMDModel *model);
    void parseFaces(PMDModel *model);
    void parseFaceDisplayNames(PMDModel *model);
    void parseBoneDisplayNames(PMDModel *model);
    void parseEnglishDisplayNames(PMDModel *model);
    void parseToonTextureNames(PMDModel *model);
    void parseRigidBodies(PMDModel *model);
    void parseConstraints(PMDModel *model);

    ParserResult m_result;
    char *m_data;
    size_t m_size;
    size_t m_rest;
};

PMDParser::PMDParser(const char *data, size_t size)
    : m_data(const_cast<char *>(data)),
      m_size(size),
      m_rest(size)
{
    memset(&m_result, 0, sizeof(ParserResult));
}

PMDParser::~PMDParser()
{
    memset(&m_result, 0, sizeof(ParserResult));
    m_data = 0;
    m_size = 0;
    m_rest = 0;
}

bool PMDParser::preparse()
{
    /* header + version + name + comment */
    if (283 > m_rest)
        return false;

    char *ptr = m_data;
    m_result.basePtr = ptr;

    /* header and version check */
    if (memcmp(ptr, "Pmd", 3) != 0)
        return false;
    ptr += 3;
    if (1.0f != *reinterpret_cast<float *>(ptr))
        return false;

    /* name and comment (in Shift-JIS) */
    ptr += sizeof(float);
    m_result.namePtr = ptr;
    ptr += 20;
    m_result.commentPtr = ptr;
    ptr += 256;

    size_t nVertices = 0, nIndices = 0, nMaterials = 0, nBones = 0, nIKs = 0, nFaces = 0,
            nFaceNames = 0, nBoneFrames = 0, nBoneNames = 0, nRigidBodies = 0, nConstranits = 0;
    /* vertex */
    if (!vpvlDataGetSize32(ptr, m_rest, nVertices))
        return false;
    m_result.verticesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Vertex::stride(ptr), nVertices, m_rest))
        return false;
    m_result.verticesCount = nVertices;

    /* index */
    if (!vpvlDataGetSize32(ptr, m_rest, nIndices))
        return false;
    m_result.indicesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, sizeof(uint16_t), nIndices, m_rest))
        return false;
    m_result.indicesCount = nIndices;

    /* material */
    if (!vpvlDataGetSize32(ptr, m_rest, nMaterials))
        return false;
    m_result.materialsPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Material::stride(ptr), nMaterials, m_rest))
        return false;
    m_result.materialsCount = nMaterials;

    /* bone */
    if (!vpvlDataGetSize16(ptr, m_rest, nBones))
        return false;
    m_result.bonesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Bone::stride(ptr), nBones, m_rest))
        return false;
    m_result.bonesCount = nMaterials;

    /* IK */
    if (!vpvlDataGetSize16(ptr, m_rest, nIKs))
        return false;
    m_result.IKsPtr = ptr;
    if (!vpvlDataValidateSize(ptr, IK::stride(ptr), nIKs, m_rest))
        return false;
    m_result.IKsCount = nIKs;

    /* face */
    if (!vpvlDataGetSize16(ptr, m_rest, nFaces))
        return false;
    m_result.facesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Face::stride(ptr), nFaces, m_rest))
        return false;
    m_result.facesCount = nFaces;

    /* face display names */
    if (!vpvlDataGetSize8(ptr, m_rest, nFaceNames))
        return false;
    m_result.faceDisplayNamesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, sizeof(uint16_t), nFaceNames, m_rest))
        return false;
    m_result.faceDisplayNamesCount = nFaceNames;

    /* bone frame names */
    if (!vpvlDataGetSize8(ptr, m_rest, nBoneFrames))
        return false;
    m_result.boneFrameNamesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, 50, nBoneFrames, m_rest))
        return false;
    m_result.boneFrameNamesCount = nBoneFrames;

    /* bone display names */
    if (!vpvlDataGetSize8(ptr, m_rest, nBoneFrames))
        return false;
    m_result.boneDisplayNamesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, sizeof(uint16_t) + sizeof(uint8_t), nBoneNames, m_rest))
        return false;
    m_result.boneDisplayNamesCount = nBoneNames;

    if (m_rest == 0) {
        m_rest = m_size;
        return true;
    }

    /* english names */
    size_t english;
    vpvlDataGetSize8(ptr, m_rest, english);
    if (english != 0) {
        size_t tmp = nFaces > 0 ? (nFaces - 1) * 20 : 0;
        const size_t required = 20 + 256 + 20 * nBones + tmp + 50 * nBoneFrames;
        if (required > m_rest)
            return false;
        m_result.englishDisplayNamesPtr = ptr;
        ptr += required;
    }

    /* extra texture path */
    if (1000 > m_rest)
        return false;
    m_result.toonTextureNamesPtr = ptr;
    m_rest += 1000;

    if (m_rest == 0) {
        m_rest = m_size;
        return true;
    }

    /* rigid body */
    if (!vpvlDataGetSize32(ptr, m_rest, nRigidBodies))
        return false;
    m_result.rigidBodiesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, RigidBody::stride(ptr), nRigidBodies, m_rest))
        return false;
    m_result.rigidBodiesCount = nRigidBodies;

    /* constranint */
    if (!vpvlDataGetSize32(ptr, m_rest, nConstranits))
        return false;
    m_result.constraintsPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Constraint::stride(ptr), nConstranits, m_rest))
        return false;
    m_result.constranitsCount = nConstranits;

    m_rest = m_size;
    return true;
}

IModel *PMDParser::parse()
{
    if (preparse()) {
        PMDModel *model = new PMDModel();
        parseHeader(model);
        parseVertices(model);
        parseIndices(model);
        parseMatrials(model);
        parseBones(model);
        parseIKs(model);
        parseFaces(model);
        parseFaceDisplayNames(model);
        parseBoneDisplayNames(model);
        parseEnglishDisplayNames(model);
        parseToonTextureNames(model);
        parseRigidBodies(model);
        parseConstraints(model);
        return model;
    }
    return 0;
}

void PMDParser::parseHeader(PMDModel *model)
{
    model->setName(m_result.namePtr);
    model->setComment(m_result.commentPtr);
}

void PMDParser::parseVertices(PMDModel *model)
{
    VertexList vertices;
    char *ptr = m_result.verticesPtr;
    int nvertices = m_result.verticesCount;
    vertices.reserve(nvertices);
    for (int i = 0; i < nvertices; i++) {
        vertices[i].read(ptr);
        ptr += Vertex::stride(ptr);
    }
    model->setVertices(vertices);
}

void PMDParser::parseIndices(PMDModel *model)
{
    IndexList indices;
    char *ptr = m_result.indicesPtr;
    int nindices = m_result.indicesCount;
    indices.reserve(nindices);
    for (int i = 0; i < nindices; i++) {
        indices[i] = *reinterpret_cast<uint16_t *>(ptr);
        ptr += sizeof(uint16_t);
    }
    model->setIndices(indices);
}

void PMDParser::parseMatrials(PMDModel *model)
{
    MaterialList materials;
    char *ptr = m_result.materialsPtr;
    int nmaterials = m_result.materialsCount;
    materials.reserve(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        materials[i].read(ptr);
        ptr += Material::stride(ptr);
    }
    model->setMaterials(materials);
}

void PMDParser::parseBones(PMDModel *model)
{
    BoneList bones;
    Bone *mutableRootBone = model->mutableRootBone();
    char *ptr = m_result.bonesPtr;
    int nbones = m_result.bonesCount;
    bones.reserve(nbones);
    for (int i = 0; i < nbones; i++) {
        bones[i].read(ptr, bones, mutableRootBone);
        ptr += Bone::stride(ptr);
    }
    model->setBones(bones);
}

void PMDParser::parseIKs(PMDModel *model)
{
    IKList IKs;
    char *ptr = m_result.IKsPtr;
    int nIKs = m_result.IKsCount;
    IKs.reserve(nIKs);
    for (int i = 0; i < nIKs; i++) {
        IKs[i].read(ptr, model->mutableBones());
        ptr += IK::stride(ptr);
    }
    model->setIKs(IKs);
}

void PMDParser::parseFaces(PMDModel *model)
{
    FaceList faces;
    char *ptr = m_result.facesPtr;
    int nfaces = m_result.facesCount;
    faces.reserve(nfaces);
    for (int i = 0; i < nfaces; i++) {
        faces[i].read(ptr);
        ptr += Face::stride(ptr);
    }
    model->setFaces(faces);
}

void PMDParser::parseFaceDisplayNames(PMDModel * /* model */)
{
}

void PMDParser::parseBoneDisplayNames(PMDModel * /* model */)
{
}

void PMDParser::parseEnglishDisplayNames(PMDModel *model)
{
    model->setEnglishName("");
    model->setEnglishComment("");
}

void PMDParser::parseToonTextureNames(PMDModel *model)
{
    model->setToonTextures(m_result.toonTextureNamesPtr);
}

void PMDParser::parseRigidBodies(PMDModel *model)
{
    RigidBodyList rigidBodies;
    char *ptr = m_result.rigidBodiesPtr;
    int nrigidBodies = m_result.rigidBodiesCount;
    rigidBodies.reserve(nrigidBodies);
    for (int i = 0; i < nrigidBodies; i++) {
        rigidBodies[i].read(ptr, model->mutableBones());
        ptr += RigidBody::stride(ptr);
    }
    model->setRigidBodies(rigidBodies);
}

void PMDParser::parseConstraints(PMDModel *model)
{
    ConstraintList constraints;
    btVector3 offset = model->rootBone().offset();
    char *ptr = m_result.constraintsPtr;
    int nconstraints = m_result.constranitsCount;
    constraints.reserve(nconstraints);
    for (int i = 0; i < nconstraints; i++) {
        constraints[i].read(ptr, model->rigidBodies(), offset);
        ptr += Constraint::stride(ptr);
    }
    model->setConstraints(constraints);
}

}
