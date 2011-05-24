#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDModel.h"

namespace vpvl
{

const float PMDModel::kMinBoneWeight = 0.0001f;
const float PMDModel::kMinFaceWeight = 0.001f;

PMDModel::PMDModel(const char *data, size_t size)
    : m_size(size),
      m_data(data),
      m_boundingSphereStep(kBoundingSpherePointsMin),
      m_edgeOffset(0.03f),
      m_selfShadowDensityCoef(0.0f),
      m_enableSimulation(true)
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
    memset(&m_result, 0, sizeof(PMDModelDataInfo));
    m_rootBone.setCurrentRotation(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_rootBone.updateTransform();
}

PMDModel::~PMDModel()
{
    memset(&m_name, 0, sizeof(m_name));
    memset(&m_comment, 0, sizeof(m_comment));
    memset(&m_englishName, 0, sizeof(m_englishName));
    memset(&m_englishComment, 0, sizeof(m_englishComment));
    memset(&m_result, 0, sizeof(PMDModelDataInfo));
    m_data = 0;
    m_size = 0;
}

void PMDModel::prepare()
{
    m_skinningTransform.reserve(m_bones.size());
    int nVertices = m_vertices.size();
    m_skinnedVertices.reserve(nVertices);
    m_edgeVertices.reserve(nVertices);
    m_toonTextureCoords.reserve(nVertices);
    uint32_t from = 0;
    int nMaterials = m_materials.size();
    for (int i = 0; i < nMaterials; i++) {
        Material *material = m_materials[i];
        uint32_t nindices = material->countIndices();
        if (material->isEdgeEnabled()) {
            for (uint32_t j = 0; j < nindices; j++) {
                m_edgeIndices.push_back(m_indices[from + j]);
            }
        }
        from += nindices;
    }
    int nBones = m_bones.size();
    for (int i = 0; i < nBones; i++) {
        BoneType type = m_bones[i]->type();
        if (type == kUnderRotate || type == kFollowRotate)
            m_rotatedBones.push_back(i);
    }
    int nIKs = m_IKs.size();
    m_isIKSimulated.reserve(nIKs);
    for (int i = 0; i < nIKs; i++) {
        m_isIKSimulated.push_back(m_IKs[i]->isSimulated());
    }
    m_boundingSphereStep = nVertices / kBoundingSpherePoints;
    uint32_t max = kBoundingSpherePointsMax;
    uint32_t min = kBoundingSpherePointsMin;
    btClamp(m_boundingSphereStep, max, min);
}

void PMDModel::updateBone()
{
    int nBones = m_bones.size(), nIKs = m_IKs.size();
    /* FIXME: ordered bone list  */
    for (int i = 0; i < nBones; i++)
        m_bones[i]->updateTransform();
    if (m_enableSimulation) {
        for (int i = 0; i < nIKs; i++) {
            if (!m_isIKSimulated[i])
                m_IKs[i]->solve();
        }
    }
    else {
        for (int i = 0; i < nIKs; i++)
            m_IKs[i]->solve();
    }
    int nRotatedBones = m_rotatedBones.size();
    for (int i = 0; i < nRotatedBones; i++)
        m_bones[m_rotatedBones[i]]->updateRotation();
}

void PMDModel::updateBoneFromSimulation()
{
    int nRigidBodies = m_rigidBodies.size();
    for (int i = 0; i < nRigidBodies; i++)
        m_rigidBodies[i]->transformToBone();
}

void PMDModel::updateFace()
{
    int nFaces = m_faces.size();
    for (int i = 0; i < nFaces; i++) {
        Face *face = m_faces[i];
        if (face->type() == kBase) {
            face->applyToVertices(m_vertices);
            break;
        }
    }
    for (int i = 0; i < nFaces; i++) {
        Face *face = m_faces[i];
        float weight = face->weight();
        if (weight > kMinFaceWeight)
            face->addToVertices(m_vertices, weight);
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
        m_bones[i]->getSkinTransform(m_skinningTransform[i]);
    int nVertices = m_vertices.size();
    for (int i = 0; i < nVertices; i++) {
        Vertex *vertex = m_vertices[i];
        SkinVertex *skin = &m_skinnedVertices[i];
        const float weight = vertex->weight();
        if (weight >= 1.0f - kMinBoneWeight) {
            const int16_t bone1 = vertex->bone1();
            skin->position = m_skinningTransform[bone1] * vertex->position();
            skin->normal = m_skinningTransform[bone1].getBasis() * vertex->normal();
        }
        else if (weight <= kMinBoneWeight) {
            const int16_t bone2 = vertex->bone2();
            skin->position = m_skinningTransform[bone2] * vertex->position();
            skin->normal = m_skinningTransform[bone2].getBasis() * vertex->normal();
        }
        else {
            const int16_t bone1 = vertex->bone1();
            const int16_t bone2 = vertex->bone2();
            const btVector3 position = vertex->position();
            const btVector3 normal = vertex->normal();
            const btVector3 v1 = m_skinningTransform[bone1] * position;
            const btVector3 n1 = m_skinningTransform[bone1].getBasis() * normal;
            const btVector3 v2 = m_skinningTransform[bone2] * position;
            const btVector3 n2 = m_skinningTransform[bone2].getBasis() * normal;
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
        if (m_vertices[i]->isEdgeEnabled())
            m_edgeVertices[i] = skin.position;
        else
            m_edgeVertices[i] = skin.position + skin.normal * m_edgeOffset;
    }
}

float PMDModel::boundingSphereRange(btVector3 &center)
{
    float max = 0.0f;
    Bone *bone = Bone::centerBone(&m_bones);
    btVector3 pos = bone->currentTransform().getOrigin();
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
        Bone *bone = m_bones[i];
        bone->setCurrentPosition(v.lerp(bone->currentPosition(), rate));
        bone->setCurrentRotation(q.slerp(bone->currentRotation(), rate));
    }
    for (int i = 0; i < nFaces; i++) {
        Face *face = m_faces[i];
        face->setWeight(face->weight() * rate);
    }
}

bool PMDModel::preparse()
{
    size_t rest = m_size;
    /* header + version + name + comment */
    if (283 > rest)
        return false;

    char *ptr = const_cast<char *>(m_data);
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
    m_result.bonesCount = nBones;

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

    if (rest == 0)
        return true;

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

    if (rest == 0)
        return true;

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

bool PMDModel::parse()
{
    if (preparse()) {
        parseHeader();
        parseVertices();
        parseIndices();
        parseMatrials();
        parseBones();
        parseIKs();
        parseFaces();
        parseFaceDisplayNames();
        parseBoneDisplayNames();
        parseEnglishDisplayNames();
        parseToonTextureNames();
        parseRigidBodies();
        parseConstraints();
        prepare();
        return true;
    }
    return false;
}

void PMDModel::parseHeader()
{
    setName(m_result.namePtr);
    setComment(m_result.commentPtr);
}

void PMDModel::parseVertices()
{
    char *ptr = const_cast<char *>(m_result.verticesPtr);
    int nvertices = m_result.verticesCount;
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        vertex->read(ptr);
        ptr += Vertex::stride(ptr);
        m_vertices.push_back(vertex);
    }
}

void PMDModel::parseIndices()
{
    char *ptr = const_cast<char *>(m_result.indicesPtr);
    int nindices = m_result.indicesCount;
    for (int i = 0; i < nindices; i++) {
        m_indices.push_back(*reinterpret_cast<uint16_t *>(ptr));
        ptr += sizeof(uint16_t);
    }
}

void PMDModel::parseMatrials()
{
    char *ptr = const_cast<char *>(m_result.materialsPtr);
    int nmaterials = m_result.materialsCount;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = new Material();
        material->read(ptr);
        ptr += Material::stride(ptr);
        m_materials.push_back(material);
    }
}

void PMDModel::parseBones()
{
    Bone *mutableRootBone = this->mutableRootBone();
    BoneList *mutableBones = this->mutableBones();
    char *ptr = const_cast<char *>(m_result.bonesPtr);
    int nbones = m_result.bonesCount;
    for (int i = 0; i < nbones; i++) {
        Bone *bone = new Bone();
        bone->read(ptr, mutableBones, mutableRootBone);
        ptr += Bone::stride(ptr);
        m_bones.push_back(bone);
    }
}

void PMDModel::parseIKs()
{
    char *ptr = const_cast<char *>(m_result.IKsPtr);
    int nIKs = m_result.IKsCount;
    BoneList *mutableBones = this->mutableBones();
    for (int i = 0; i < nIKs; i++) {
        IK *ik = new IK();
        ik->read(ptr, mutableBones);
        ptr += IK::stride(ptr);
        m_IKs.push_back(ik);
    }
}

void PMDModel::parseFaces()
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
        m_faces.push_back(face);
    }
    if (baseFace) {
        for (int i = 0; i < nfaces; i++) {
            m_faces[i]->convertIndices(baseFace);
        }
    }
}

void PMDModel::parseFaceDisplayNames()
{
}

void PMDModel::parseBoneDisplayNames()
{
}

void PMDModel::parseEnglishDisplayNames()
{
    setEnglishName(m_result.englishNamePtr);
    setEnglishComment(m_result.englishCommentPtr);
}

void PMDModel::parseToonTextureNames()
{
    setToonTextures(m_result.toonTextureNamesPtr);
}

void PMDModel::parseRigidBodies()
{
    BoneList *mutableBones = this->mutableBones();
    char *ptr = const_cast<char *>(m_result.rigidBodiesPtr);
    int nrigidBodies = m_result.rigidBodiesCount;
    for (int i = 0; i < nrigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody();
        rigidBody->read(ptr, mutableBones);
        ptr += RigidBody::stride(ptr);
        m_rigidBodies.push_back(rigidBody);
    }
}

void PMDModel::parseConstraints()
{
    RigidBodyList rigidBodies = this->rigidBodies();
    btVector3 offset = this->rootBone().offset();
    char *ptr = const_cast<char *>(m_result.constraintsPtr);
    int nconstraints = m_result.constranitsCount;
    for (int i = 0; i < nconstraints; i++) {
        Constraint *constraint = new Constraint();
        constraint->read(ptr, rigidBodies, offset);
        ptr += Constraint::stride(ptr);
        m_constraints.push_back(constraint);
    }
}


}
