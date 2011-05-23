#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDParser.h"

namespace vpvl
{

PMDParser::PMDParser(const char *data, size_t size)
    : m_data(const_cast<char *>(data)),
      m_size(size)
{
    memset(&m_result, 0, sizeof(PMDParserResult));
}

PMDParser::~PMDParser()
{
    memset(&m_result, 0, sizeof(PMDParserResult));
    m_data = 0;
    m_size = 0;
}

bool PMDParser::preparse()
{
    size_t rest = m_size;
    /* header + version + name + comment */
    if (283 > rest)
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
    rest -= 283;

    size_t nVertices = 0, nIndices = 0, nMaterials = 0, nBones = 0, nIKs = 0, nFaces = 0,
            nFaceNames = 0, nBoneFrames = 0, nBoneNames = 0, nRigidBodies = 0, nConstranits = 0;
    /* vertex */
    if (!vpvlDataGetSize32(ptr, rest, nVertices))
        return false;
    m_result.verticesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Vertex::stride(ptr), nVertices, rest))
        return false;
    m_result.verticesCount = nVertices;

    /* index */
    if (!vpvlDataGetSize32(ptr, rest, nIndices))
        return false;
    m_result.indicesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, sizeof(uint16_t), nIndices, rest))
        return false;
    m_result.indicesCount = nIndices;

    /* material */
    if (!vpvlDataGetSize32(ptr, rest, nMaterials))
        return false;
    m_result.materialsPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Material::stride(ptr), nMaterials, rest))
        return false;
    m_result.materialsCount = nMaterials;

    /* bone */
    if (!vpvlDataGetSize16(ptr, rest, nBones))
        return false;
    m_result.bonesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Bone::stride(ptr), nBones, rest))
        return false;
    m_result.bonesCount = nMaterials;

    /* IK */
    if (!vpvlDataGetSize16(ptr, rest, nIKs))
        return false;
    m_result.IKsPtr = ptr;
    if (!vpvlDataValidateSize(ptr, IK::totalSize(ptr, nIKs), 1, rest))
        return false;
    m_result.IKsCount = nIKs;

    /* face */
    if (!vpvlDataGetSize16(ptr, rest, nFaces))
        return false;
    m_result.facesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Face::totalSize(ptr, nFaces), 1, rest))
        return false;
    m_result.facesCount = nFaces;

    /* face display names */
    if (!vpvlDataGetSize8(ptr, rest, nFaceNames))
        return false;
    m_result.faceDisplayNamesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, sizeof(uint16_t), nFaceNames, rest))
        return false;
    m_result.faceDisplayNamesCount = nFaceNames;

    /* bone frame names */
    if (!vpvlDataGetSize8(ptr, rest, nBoneFrames))
        return false;
    m_result.boneFrameNamesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, 50, nBoneFrames, rest))
        return false;
    m_result.boneFrameNamesCount = nBoneFrames;

    /* bone display names */
    if (!vpvlDataGetSize32(ptr, rest, nBoneNames))
        return false;
    m_result.boneDisplayNamesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, sizeof(uint16_t) + sizeof(uint8_t), nBoneNames, rest))
        return false;
    m_result.boneDisplayNamesCount = nBoneNames;

    if (rest == 0) {
        rest = m_size;
        return true;
    }

    /* english names */
    size_t english;
    vpvlDataGetSize8(ptr, rest, english);
    if (english == 1) {
        size_t tmp = nFaces > 0 ? (nFaces - 1) * 20 : 0;
        const size_t required = 20 * nBones + tmp + 50 * nBoneFrames;
        if ((required + 276) > rest)
            return false;
        m_result.englishNamePtr = ptr;
        ptr += 20;
        m_result.englishCommentPtr = ptr;
        ptr += 256;
        ptr += required;
        rest -= required;
    }

    /* extra texture path */
    if (1000 > rest)
        return false;
    m_result.toonTextureNamesPtr = ptr;
    ptr += 1000;
    rest -= 1000;

    if (rest == 0) {
        rest = m_size;
        return true;
    }

    /* rigid body */
    if (!vpvlDataGetSize32(ptr, rest, nRigidBodies))
        return false;
    m_result.rigidBodiesPtr = ptr;
    if (!vpvlDataValidateSize(ptr, RigidBody::stride(ptr), nRigidBodies, rest))
        return false;
    m_result.rigidBodiesCount = nRigidBodies;

    /* constranint */
    if (!vpvlDataGetSize32(ptr, rest, nConstranits))
        return false;
    m_result.constraintsPtr = ptr;
    if (!vpvlDataValidateSize(ptr, Constraint::stride(ptr), nConstranits, rest))
        return false;
    m_result.constranitsCount = nConstranits;

    return true;
}

PMDModel *PMDParser::parse()
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
        model->prepare();
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
    char *ptr = const_cast<char *>(m_result.verticesPtr);
    int nvertices = m_result.verticesCount;
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        vertex->read(ptr);
        ptr += Vertex::stride(ptr);
        model->addVertex(vertex);
    }
}

void PMDParser::parseIndices(PMDModel *model)
{
    char *ptr = const_cast<char *>(m_result.indicesPtr);
    int nindices = m_result.indicesCount;
    for (int i = 0; i < nindices; i++) {
        model->addIndex(*reinterpret_cast<uint16_t *>(ptr));
        ptr += sizeof(uint16_t);
    }
}

void PMDParser::parseMatrials(PMDModel *model)
{
    char *ptr = const_cast<char *>(m_result.materialsPtr);
    int nmaterials = m_result.materialsCount;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = new Material();
        material->read(ptr);
        ptr += Material::stride(ptr);
        model->addMaterial(material);
    }
}

void PMDParser::parseBones(PMDModel *model)
{
    Bone *mutableRootBone = model->mutableRootBone();
    BoneList *mutableBones = model->mutableBones();
    char *ptr = const_cast<char *>(m_result.bonesPtr);
    int nbones = m_result.bonesCount;
    for (int i = 0; i < nbones; i++) {
        Bone *bone = new Bone();
        bone->read(ptr, mutableBones, mutableRootBone);
        ptr += Bone::stride(ptr);
        model->addBone(bone);
    }
}

void PMDParser::parseIKs(PMDModel *model)
{
    char *ptr = const_cast<char *>(m_result.IKsPtr);
    int nIKs = m_result.IKsCount;
    BoneList *mutableBones = model->mutableBones();
    for (int i = 0; i < nIKs; i++) {
        IK *ik = new IK();
        ik->read(ptr, mutableBones);
        ptr += IK::stride(ptr);
        model->addIK(ik);
    }
}

void PMDParser::parseFaces(PMDModel *model)
{
    Face *baseFace = 0;
    char *ptr = const_cast<char *>(m_result.facesPtr);
    int nfaces = m_result.facesCount;
    for (int i = 0; i < nfaces; i++) {
        Face *face = new Face();
        face->read(ptr);
        ptr += Face::stride(ptr);
        if (face->type() == kBase)
            baseFace = face;
        model->addFace(face);
    }
    /*
    if (baseFace) {
        FaceList faces = model->faces();
        for (int i = 0; i < nfaces; i++) {
            faces[i]->convertIndices(baseFace);
        }
    }
    */
}

void PMDParser::parseFaceDisplayNames(PMDModel * /* model */)
{
}

void PMDParser::parseBoneDisplayNames(PMDModel * /* model */)
{
}

void PMDParser::parseEnglishDisplayNames(PMDModel *model)
{
    model->setEnglishName(m_result.englishNamePtr);
    model->setEnglishComment(m_result.englishCommentPtr);
}

void PMDParser::parseToonTextureNames(PMDModel *model)
{
    model->setToonTextures(m_result.toonTextureNamesPtr);
}

void PMDParser::parseRigidBodies(PMDModel *model)
{
    BoneList *mutableBones = model->mutableBones();
    char *ptr = const_cast<char *>(m_result.rigidBodiesPtr);
    int nrigidBodies = m_result.rigidBodiesCount;
    for (int i = 0; i < nrigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody();
        rigidBody->read(ptr, mutableBones);
        ptr += RigidBody::stride(ptr);
        model->addRigidBody(rigidBody);
    }
}

void PMDParser::parseConstraints(PMDModel *model)
{
    RigidBodyList rigidBodies = model->rigidBodies();
    btVector3 offset = model->rootBone().offset();
    char *ptr = const_cast<char *>(m_result.constraintsPtr);
    int nconstraints = m_result.constranitsCount;
    for (int i = 0; i < nconstraints; i++) {
        Constraint *constraint = new Constraint();
        constraint->read(ptr, rigidBodies, offset);
        ptr += Constraint::stride(ptr);
        model->addConstraint(constraint);
    }
}

}
