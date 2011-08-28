/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

#ifndef VPVL_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
struct btDiscreteDynamicsWorld { int unused; };
#endif

namespace vpvl
{

const float PMDModel::kMinBoneWeight = 0.0001f;
const float PMDModel::kMinFaceWeight = 0.001f;

class VMDMotionPriorityPredication
{
public:
    bool operator()(const VMDMotion *left, const VMDMotion *right) {
        return left->priority() > right->priority();
    }
};

struct SkinVertex
{
    btVector3 position;
    btVector3 normal;
    btVector3 texureCoord;
};

struct State
{
    const PMDModel *model;
    Array<btVector3> positions;
    Array<btQuaternion> rotations;
    Array<float> weights;
};

PMDModel::PMDModel()
    : m_baseFace(0),
      m_orderedBones(0),
      m_skinnedVertices(0),
      m_world(0),
      m_indicesPointer(0),
      m_edgeIndicesPointer(0),
      m_edgeIndicesCount(0),
      m_error(kNoError),
      m_boundingSphereStep(kBoundingSpherePointsMin),
      m_edgeOffset(0.03f),
      m_selfShadowDensityCoef(0.0f),
      m_enableSimulation(false)
{
    internal::zerofill(&m_name, sizeof(m_name));
    internal::zerofill(&m_comment, sizeof(m_comment));
    internal::zerofill(&m_englishName, sizeof(m_englishName));
    internal::zerofill(&m_englishComment, sizeof(m_englishComment));
    m_rootBone.setRotation(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_rootBone.updateTransform();
}

PMDModel::~PMDModel()
{
    release();
}

void PMDModel::prepare()
{
    m_skinningTransform.reserve(m_bones.count());
    uint32_t nVertices = m_vertices.count();
    m_skinnedVertices = new SkinVertex[nVertices];
    m_edgeVertices.reserve(nVertices);
    m_toonTextureCoords.reserve(nVertices);
    m_edgeIndicesPointer = new uint16_t[m_indices.count()];
    uint16_t *from = m_indicesPointer, *to = m_edgeIndicesPointer;
    uint32_t nMaterials = m_materials.count();
    // create edge vertices (copy model vertices if the material is enabled)
    for (uint32_t i = 0; i < nMaterials; i++) {
        const Material *material = m_materials[i];
        const uint32_t nindices = material->countIndices();
        if (material->isEdgeEnabled()) {
            memcpy(to, from, sizeof(uint16_t) * nindices);
            to += nindices;
            m_edgeIndicesCount += nindices;
        }
        from += nindices;
    }
    // Therefore no updating texture coordinates, set texture coordinates here
    for (int i = 0; i < nVertices; i++) {
        const Vertex *vertex = m_vertices[i];
        m_skinnedVertices[i].texureCoord.setValue(vertex->u(), vertex->v(), 0);
    }
    const uint32_t nBones = m_bones.count();
    for (uint32_t i = 0; i < nBones; i++) {
        Bone *bone = m_bones[i];
        const Bone::Type type = bone->type();
        if (type == Bone::kUnderRotate || type == Bone::kFollowRotate)
            m_rotatedBones.add(bone);
    }
    const uint32_t nIKs = m_IKs.count();
    m_isIKSimulated.reserve(nIKs);
    // IK simulation: enabled => physic engine / disabled => IK#solve
    for (uint32_t i = 0; i < nIKs; i++) {
        m_isIKSimulated.add(m_IKs[i]->isSimulated());
    }
    m_boundingSphereStep = nVertices / kBoundingSpherePoints;
    uint32_t max = kBoundingSpherePointsMax;
    uint32_t min = kBoundingSpherePointsMin;
    btClamp(m_boundingSphereStep, max, min);
}

void PMDModel::addMotion(VMDMotion *motion)
{
    motion->attachModel(this);
    m_motions.add(motion);
    m_motions.sort(VMDMotionPriorityPredication());
}

void PMDModel::joinWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL_NO_BULLET
    if (!world)
        return;
    const uint32_t nRigidBodies = m_rigidBodies.count();
    for (uint32_t i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->setKinematic(false);
        world->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->groupMask());
    }
    const uint32_t nConstraints = m_constraints.count();
    for (uint32_t i = 0; i < nConstraints; i++) {
        Constraint *constraint = m_constraints[i];
        world->addConstraint(constraint->constraint());
    }
    m_enableSimulation = true;
    m_world = world;
#endif /* VPVL_NO_BULLET */
}

void PMDModel::leaveWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL_NO_BULLET
    if (!world)
        return;
    const uint32_t nRigidBodies = m_rigidBodies.count();
    for (uint32_t i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->setKinematic(true);
        world->removeCollisionObject(rigidBody->body());
    }
    const uint32_t nConstraints = m_constraints.count();
    for (uint32_t i = 0; i < nConstraints; i++) {
        Constraint *constraint = m_constraints[i];
        world->removeConstraint(constraint->constraint());
    }
    m_enableSimulation = false;
    m_world = 0;
#endif /* VPVL_NO_BULLET */
}

void PMDModel::removeMotion(VMDMotion *motion)
{
    m_motions.remove(motion);
}

void PMDModel::discardState(State *&state) const
{
    delete state;
    state = 0;
}

bool PMDModel::restoreState(State *state)
{
    const uint32_t nBones = m_bones.count(), nFaces = m_faces.count();
    bool ret = false;
    if (state->model == this) {
        for (uint32_t i = 0; i < nBones; i++) {
            Bone *bone = m_bones[i];
            bone->setPosition(state->positions[i]);
            bone->setRotation(state->rotations[i]);
        }
        for (uint32_t i = 0; i < nFaces; i++) {
            Face *face = m_faces[i];
            face->setWeight(state->weights[i]);
        }
        ret = true;
    }
    return ret;
}

PMDModel::State *PMDModel::saveState() const
{
    State *state = new State;
    const uint32_t nBones = m_bones.count(), nFaces = m_faces.count();
    for (uint32_t i = 0; i < nBones; i++) {
        Bone *bone = m_bones[i];
        state->positions.add(bone->position());
        state->rotations.add(bone->rotation());
    }
    for (uint32_t i = 0; i < nFaces; i++) {
        Face *face = m_faces[i];
        state->weights.add(face->weight());
    }
    state->model = this;
    return state;
}

void PMDModel::seekMotion(float deltaFrame)
{
    const uint32_t nMotions = m_motions.count();
    for (uint32_t i = 0; i < nMotions; i++)
        m_motions[i]->seek(deltaFrame);
    updateAllBones();
    updateAllFaces();
    updateBoneFromSimulation();
}

void PMDModel::updateRootBone()
{
    // FIXME: implement associated accessory
    m_rootBone.updateTransform();
}

void PMDModel::updateMotion(float deltaFrame)
{
    const uint32_t nMotions = m_motions.count();
    for (uint32_t i = 0; i < nMotions; i++)
        m_motions[i]->update(deltaFrame);
    updateAllBones();
    updateAllFaces();
    updateBoneFromSimulation();
}

void PMDModel::updateSkins()
{
    updateSkinVertices();
    updateToon(m_lightPosition);
}

void PMDModel::updateAllBones()
{
    const uint32_t nBones = m_bones.count(), nIKs = m_IKs.count();
    for (uint32_t i = 0; i < nBones; i++)
        m_orderedBones[i]->updateTransform();
    if (m_enableSimulation) {
        for (int i = 0; i < nIKs; i++) {
            // Solve IK with physic engine instead of IK class if it's disabled
            if (!m_isIKSimulated[i])
                m_IKs[i]->solve();
        }
    }
    else {
        for (uint32_t i = 0; i < nIKs; i++)
            m_IKs[i]->solve();
    }
    const uint32_t nRotatedBones = m_rotatedBones.count();
    for (uint32_t i = 0; i < nRotatedBones; i++)
        m_rotatedBones[i]->updateRotation();
}

void PMDModel::updateBoneFromSimulation()
{
    if (m_enableSimulation) {
        const uint32_t nRigidBodies = m_rigidBodies.count();
        for (uint32_t i = 0; i < nRigidBodies; i++)
            m_rigidBodies[i]->transformBone();
    }
}

void PMDModel::updateAllFaces()
{
    const uint32_t nFaces = m_faces.count();
    if (nFaces > 0)
        m_baseFace->setVertices(m_vertices);
    for (uint32_t i = 0; i < nFaces; i++) {
        Face *face = m_faces[i];
        const float weight = face->weight();
        if (weight > kMinFaceWeight)
            face->setVertices(m_vertices, weight);
    }
}

void PMDModel::updateShadowTextureCoords(float coef)
{
    bool update = false;
    const uint32_t nVertices = m_vertices.count();
    if (m_shadowTextureCoords.count() == 0) {
        m_shadowTextureCoords.reserve(nVertices);
        update = true;
    }
    else if (coef) {
        update = true;
    }
    if (update) {
        for (uint32_t i = 0; i < nVertices; i++)
            m_shadowTextureCoords[i].setValue(0.0f, coef, 0.0f);
        m_selfShadowDensityCoef = coef;
    }
}

void PMDModel::updateSkinVertices()
{
    const uint32_t nBones = m_bones.count();
    for (uint32_t i = 0; i < nBones; i++)
        m_bones[i]->getSkinTransform(m_skinningTransform[i]);
    const uint32_t nVertices = m_vertices.count();
    for (int i = 0; i < nVertices; i++) {
        const Vertex *vertex = m_vertices[i];
        SkinVertex &skin = m_skinnedVertices[i];
        const float weight = vertex->weight();
        if (weight >= 1.0f - kMinBoneWeight) {
            const int16_t bone1 = vertex->bone1();
            const btTransform &transform = m_skinningTransform[bone1];
            skin.position = transform * vertex->position();
            skin.normal = transform.getBasis() * vertex->normal();
        }
        else if (weight <= kMinBoneWeight) {
            const int16_t bone2 = vertex->bone2();
            const btTransform &transform = m_skinningTransform[bone2];
            skin.position = transform * vertex->position();
            skin.normal = transform.getBasis() * vertex->normal();
        }
        else {
            const int16_t bone1 = vertex->bone1();
            const int16_t bone2 = vertex->bone2();
            const btVector3 &position = vertex->position();
            const btVector3 &normal = vertex->normal();
            const btTransform &transform1 = m_skinningTransform[bone1];
            const btTransform &transform2 = m_skinningTransform[bone2];
            const btVector3 &v1 = transform1 * position;
            const btVector3 &n1 = transform1.getBasis() * normal;
            const btVector3 &v2 = transform2 * position;
            const btVector3 &n2 = transform2.getBasis() * normal;
            skin.position = v2.lerp(v1, weight);
            skin.normal = n2.lerp(n1, weight);
        }
    }
}

void PMDModel::updateToon(const btVector3 &lightDirection)
{
    const uint32_t nVertices = m_vertices.count();
    for (int i = 0; i < nVertices; i++) {
        const SkinVertex &skin = m_skinnedVertices[i];
        m_toonTextureCoords[i].setValue(0.0f, (1.0f - lightDirection.dot(skin.normal)) * 0.5f, 0.0f);
        if (!m_vertices[i]->isEdgeEnabled())
            m_edgeVertices[i] = skin.position;
        else
            m_edgeVertices[i] = skin.position + skin.normal * m_edgeOffset;
    }
}

void PMDModel::updateImmediate()
{
    updateRootBone();
    updateMotion(0);
    updateSkins();
}

void PMDModel::updateIndices()
{
    const uint32_t nIndices = m_indices.count();
    m_indicesPointer = new uint16_t[nIndices];
    internal::copyBytes(reinterpret_cast<uint8_t *>(m_indicesPointer),
                        reinterpret_cast<const uint8_t *>(&m_indices[0]),
                        sizeof(uint16_t) * nIndices);
#ifdef VPVL_COORDINATE_OPENGL
    for (int i = 0; i < nIndices; i += 3) {
        const uint16_t index = m_indicesPointer[i];
        m_indicesPointer[i] = m_indicesPointer[i + 1];
        m_indicesPointer[i + 1] = index;
    }
#endif
}

float PMDModel::boundingSphereRange(btVector3 &center)
{
    float max = 0.0f;
    Bone *bone = Bone::centerBone(&m_bones);
    btVector3 pos = bone->localTransform().getOrigin();
    const uint32_t nVertices = m_vertices.count();
    for (int i = 0; i < nVertices; i++) {
        const float r2 = pos.distance2(m_skinnedVertices[i].position);
        if (max < r2)
            max = r2;
    }
    max = sqrtf(max) * 1.1f;
    center = pos;
    return max;
}

void PMDModel::resetAllBones()
{
    m_rootBone.reset();
    m_rootBone.updateTransform();
    const uint32_t nBones = m_bones.count();
    for (int i = 0; i < nBones; i++) {
        Bone *bone = m_bones[i];
        bone->reset();
    }
}

void PMDModel::resetAllFaces()
{
    smearAllFacesToDefault(0.0f);
}

void PMDModel::smearAllBonesToDefault(float rate)
{
    const uint32_t nBones = m_bones.count();
    for (uint32_t i = 0; i < nBones; i++) {
        Bone *bone = m_bones[i];
        bone->setPosition(internal::kZeroV.lerp(bone->position(), rate));
        bone->setRotation(internal::kZeroQ.slerp(bone->rotation(), rate));
    }
}

void PMDModel::smearAllFacesToDefault(float rate)
{
    const uint32_t nFaces = m_faces.count();
    for (uint32_t i = 0; i < nFaces; i++) {
        Face *face = m_faces[i];
        face->setWeight(face->weight() * rate);
    }
}

bool PMDModel::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header[3] + Version[4] + Name[20] + Comment[256]
    if (!data || 283 > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    info.basePtr = ptr;

    // Check the signature and version is correct
    if (memcmp(ptr, "Pmd", 3) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    ptr += 3;
    if (1.0f != *reinterpret_cast<float *>(ptr)) {
        m_error = kInvalidVersionError;
        return false;
    }

    // Name and Comment (in Shift-JIS)
    ptr += sizeof(float);
    info.namePtr = ptr;
    ptr += kNameSize;
    info.commentPtr = ptr;
    ptr += kDescriptionSize;
    rest -= 283;

    size_t nVertices = 0, nIndices = 0, nMaterials = 0, nBones = 0, nIKs = 0, nFaces = 0,
            nFaceNames = 0, nBoneFrames = 0, nBoneNames = 0, nRigidBodies = 0, nConstranits = 0;
    // Vertices
    if (!internal::size32(ptr, rest, nVertices)) {
        m_error = kVerticesSizeError;
        return false;
    }
    info.verticesPtr = ptr;
    if (!internal::validateSize(ptr, Vertex::stride(), nVertices, rest)) {
        m_error = kVerticesError;
        return false;
    }
    info.verticesCount = nVertices;

    // Indices
    if (!internal::size32(ptr, rest, nIndices)) {
        m_error = kIndicesSizeError;
        return false;
    }
    info.indicesPtr = ptr;
    if (!internal::validateSize(ptr, sizeof(uint16_t), nIndices, rest)) {
        m_error = kIndicesError;
        return false;
    }
    info.indicesCount = nIndices;

    // Materials
    if (!internal::size32(ptr, rest, nMaterials)) {
        m_error = kMaterialsSizeError;
        return false;
    }
    info.materialsPtr = ptr;
    if (!internal::validateSize(ptr, Material::stride(), nMaterials, rest)) {
        m_error = kMaterialsError;
        return false;
    }
    info.materialsCount = nMaterials;

    // Bones
    if (!internal::size16(ptr, rest, nBones)) {
        m_error = kBonesSizeError;
        return false;
    }
    info.bonesPtr = ptr;
    if (!internal::validateSize(ptr, Bone::stride(), nBones, rest)) {
        m_error = kBonesError;
        return false;
    }
    info.bonesCount = nBones;

    // IKs
    if (!internal::size16(ptr, rest, nIKs)) {
        m_error = kIKsSizeError;
        return false;
    }
    info.IKsPtr = ptr;

    bool ok = false;
    size_t s = IK::totalSize(ptr, rest, nIKs, ok);
    if (!ok || !internal::validateSize(ptr, s, 1, rest)) {
        m_error = kIKsError;
        return false;
    }
    info.IKsCount = nIKs;

    // Faces
    if (!internal::size16(ptr, rest, nFaces)) {
        m_error = kFacesSizeError;
        return false;
    }
    info.facesPtr = ptr;

    ok = false;
    s = Face::totalSize(ptr, rest, nFaces, ok);
    if (!ok || !internal::validateSize(ptr, s, 1, rest)) {
        m_error = kFacesError;
        return false;
    }
    info.facesCount = nFaces;

    // Face display names
    if (!internal::size8(ptr, rest, nFaceNames)) {
        m_error = kFacesForDisplaySizeError;
        return false;
    }
    info.facesForUIPtr = ptr;
    if (!internal::validateSize(ptr, sizeof(uint16_t), nFaceNames, rest)) {
        m_error = kFacesForDisplayError;
        return false;
    }
    info.facesForUICount = nFaceNames;

    // Bone frame names
    if (!internal::size8(ptr, rest, nBoneFrames)) {
        m_error = kBoneFrameNamesSizeError;
        return false;
    }
    info.boneCategoryNamesPtr = ptr;
    if (!internal::validateSize(ptr, kBoneCategoryNameSize, nBoneFrames, rest)) {
        m_error = kBoneFrameNamesError;
        return false;
    }
    info.boneCategoryNamesCount = nBoneFrames;

    // Bone display names
    if (!internal::size32(ptr, rest, nBoneNames)) {
        m_error = kBoneDisplayNamesSizeError;
        return false;
    }
    info.bonesForUIPtr = ptr;
    if (!internal::validateSize(ptr, sizeof(uint16_t) + sizeof(uint8_t), nBoneNames, rest)) {
        m_error = kBoneDisplayNamesError;
        return false;
    }
    info.bonesForUICount = nBoneNames;

    if (rest == 0)
        return true;

    // English names
    size_t english;
    internal::size8(ptr, rest, english);
    if (english == 1) {
        const size_t englishBoneNamesSize = Bone::kNameSize * nBones;
        // In english names, the base face is not includes.
        const size_t englishFaceNamesSize = nFaces > 0 ? (nFaces - 1) * Face::kNameSize : 0;
        const size_t englishBoneCategoryNameSize = kBoneCategoryNameSize * nBoneFrames;
        const size_t required = englishBoneNamesSize + englishFaceNamesSize + englishBoneCategoryNameSize;
        if ((required + kNameSize + kDescriptionSize) > rest) {
            m_error = kEnglishNamesError;
            return false;
        }
        info.englishNamePtr = ptr;
        ptr += kNameSize;
        info.englishCommentPtr = ptr;
        ptr += kDescriptionSize;
        info.englishBoneNamesPtr = ptr;
        ptr += englishBoneNamesSize;
        info.englishFaceNamesPtr = ptr;
        ptr += englishFaceNamesSize;
        info.englishBoneFramesPtr = ptr;
        ptr += englishBoneCategoryNameSize;
        rest -= required;
    }

    // Extra texture path (100 * 10)
    if (1000 > rest) {
        m_error = kExtraTextureNamesError;
        return false;
    }
    info.toonTextureNamesPtr = ptr;
    ptr += 1000;
    rest -= 1000;

    if (rest == 0)
        return true;

    // Rigid body
    if (!internal::size32(ptr, rest, nRigidBodies)) {
        m_error = kRigidBodiesSizeError;
        return false;
    }
    info.rigidBodiesPtr = ptr;
    if (!internal::validateSize(ptr, RigidBody::stride(), nRigidBodies, rest)) {
        m_error = kRigidBodiesError;
        return false;
    }
    info.rigidBodiesCount = nRigidBodies;

    // Constranint
    if (!internal::size32(ptr, rest, nConstranits)) {
        m_error = kConstraintsSizeError;
        return false;
    }
    info.constraintsPtr = ptr;
    if (!internal::validateSize(ptr, Constraint::stride(), nConstranits, rest)) {
        m_error = kConstraintsError;
        return false;
    }
    info.constranitsCount = nConstranits;

    return true;
}

bool PMDModel::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        release();
        parseHeader(info);
        parseVertices(info);
        parseIndices(info);
        parseMatrials(info);
        parseBones(info);
        parseIKs(info);
        parseFaces(info);
        parseFacesForUI(info);
        parseBoneCategoryNames(info);
        parseBonesForUI(info);
        parseEnglishDisplayNames(info);
        parseToonTextureNames(info);
        parseRigidBodies(info);
        parseConstraints(info);
        prepare();
        return true;
    }
    return false;
}

void PMDModel::parseHeader(const DataInfo &info)
{
    setName(info.namePtr);
    setComment(info.commentPtr);
}

void PMDModel::parseVertices(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.verticesPtr);
    const uint32_t nvertices = info.verticesCount;
    m_vertices.reserve(nvertices);
    for (uint32_t i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        vertex->read(ptr);
        ptr += Vertex::stride();
        m_vertices.add(vertex);
    }
}

void PMDModel::parseIndices(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.indicesPtr);
    const uint32_t nindices = info.indicesCount;
    m_indices.reserve(nindices);
    for (uint32_t i = 0; i < nindices; i++) {
        m_indices.add(*reinterpret_cast<uint16_t *>(ptr));
        ptr += sizeof(uint16_t);
    }
    updateIndices();
}

void PMDModel::parseMatrials(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.materialsPtr);
    const uint32_t nmaterials = info.materialsCount;
    m_materials.reserve(nmaterials);
    for (uint32_t i = 0; i < nmaterials; i++) {
        Material *material = new Material();
        material->read(ptr);
        ptr += Material::stride();
        m_materials.add(material);
    }
}

void PMDModel::parseBones(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.bonesPtr);
    uint8_t *englishPtr = const_cast<uint8_t *>(info.englishBoneNamesPtr);
    const uint32_t nbones = info.bonesCount;
    m_bones.reserve(nbones);
    for (uint32_t i = 0; i < nbones; i++) {
        Bone *bone = new Bone();
        bone->read(ptr, i);
        if (englishPtr) {
            const uint8_t *englishNamePtr = englishPtr + Bone::kNameSize * i;
            bone->setEnglishName(englishNamePtr);
            m_name2bone.insert(HashString(reinterpret_cast<const char *>(bone->englishName())), bone);
        }
        ptr += Bone::stride();
        m_name2bone.insert(HashString(reinterpret_cast<const char *>(bone->name())), bone);
        m_bones.add(bone);
    }
    for (uint32_t i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        bone->build(&m_bones, &m_rootBone);
    }
    sortBones();
    for (uint32_t i = 0; i < nbones; i++) {
        Bone *bone = m_orderedBones[i];
        bone->updateTransform();
    }
}

void PMDModel::parseIKs(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.IKsPtr);
    const uint32_t nIKs = info.IKsCount;
    BoneList *mutableBones = this->mutableBones();
    m_IKs.reserve(nIKs);
    for (uint32_t i = 0; i < nIKs; i++) {
        IK *ik = new IK();
        ik->read(ptr, mutableBones);
        ptr += IK::stride(ptr);
        m_IKs.add(ik);
    }
}

void PMDModel::parseFaces(const DataInfo &info)
{
    Face *baseFace = 0;
    uint8_t *ptr = const_cast<uint8_t *>(info.facesPtr);
    uint8_t *englishPtr = const_cast<uint8_t *>(info.englishFaceNamesPtr);
    const uint32_t nfaces = info.facesCount;
    m_faces.reserve(nfaces);
    for (uint32_t i = 0; i < nfaces; i++) {
        Face *face = new Face();
        face->read(ptr);
        if (face->type() == Face::kBase)
            m_baseFace = baseFace = face;
        else if (englishPtr) {
            const uint8_t *englishNamePtr = englishPtr + Face::kNameSize * i;
            face->setEnglishName(englishNamePtr);
            m_name2face.insert(HashString(reinterpret_cast<const char *>(face->englishName())), face);
        }
        ptr += Face::stride(ptr);
        m_name2face.insert(HashString(reinterpret_cast<const char *>(face->name())), face);
        m_faces.add(face);
    }
    if (baseFace) {
        for (uint32_t i = 0; i < nfaces; i++) {
            m_faces[i]->convertIndices(baseFace);
        }
    }
}

void PMDModel::parseFacesForUI(const DataInfo &info)
{
    const uint32_t nFacesForUI = info.facesForUICount;
    const uint32_t nFaces = m_faces.count();
    uint16_t *ptr = reinterpret_cast<uint16_t *>(const_cast<uint8_t *>(info.facesForUIPtr));
    for (uint32_t i = 0; i < nFacesForUI; i++) {
        uint16_t faceIndex = *ptr;
        if (faceIndex < nFaces) {
            Face *face = m_faces[faceIndex];
            m_facesForUI.add(face);
        }
        ptr++;
    }
}

void PMDModel::parseBoneCategoryNames(const DataInfo &info)
{
    const uint8_t nBoneCategoryNames = info.boneCategoryNamesCount;
    const uint8_t *ptr = const_cast<uint8_t *>(info.boneCategoryNamesPtr);
    for (uint32_t i = 0; i < nBoneCategoryNames; i++) {
        uint8_t *name = new uint8_t[kBoneCategoryNameSize];
        copyBytesSafe(name, ptr, kBoneCategoryNameSize);
        m_boneCategoryNames.add(name);
        ptr += kBoneCategoryNameSize;
    }
}

void PMDModel::parseBonesForUI(const DataInfo &info)
{
    const uint32_t nBonesCategoryNames = m_boneCategoryNames.count();
    const uint32_t nBonesForUI = info.bonesForUICount;
    const uint32_t nBones = m_bones.count();
    uint8_t *ptr = const_cast<uint8_t *>(info.bonesForUIPtr);
    m_bonesForUI.reserve(nBonesCategoryNames);
    for (uint32_t i = 0; i < nBonesCategoryNames; i++)
        m_bonesForUI.add(new BoneList());
    for (uint32_t i = 0; i < nBonesForUI; i++) {
        uint16_t boneIndex = *reinterpret_cast<uint16_t *>(ptr);
        ptr += sizeof(uint16_t);
        if (boneIndex < nBones) {
            Bone *bone = m_bones[boneIndex];
            uint8_t boneCategoryIndex = *ptr - 1;
            if (boneCategoryIndex < nBonesCategoryNames)
                m_bonesForUI[boneCategoryIndex]->add(bone);
        }
        ptr += sizeof(uint8_t);
    }
}

void PMDModel::parseEnglishDisplayNames(const DataInfo &info)
{
    if (info.englishNamePtr)
        setEnglishName(info.englishNamePtr);
    if (info.englishCommentPtr)
        setEnglishComment(info.englishCommentPtr);
}

void PMDModel::parseToonTextureNames(const DataInfo &info)
{
    setToonTextures(info.toonTextureNamesPtr);
}

void PMDModel::parseRigidBodies(const DataInfo &info)
{
    BoneList *mutableBones = &m_bones;
    uint8_t *ptr = const_cast<uint8_t *>(info.rigidBodiesPtr);
    const uint32_t nrigidBodies = info.rigidBodiesCount;
    m_rigidBodies.reserve(nrigidBodies);
    for (uint32_t i = 0; i < nrigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody();
        rigidBody->read(ptr, mutableBones);
        ptr += RigidBody::stride();
        m_rigidBodies.add(rigidBody);
    }
}

void PMDModel::parseConstraints(const DataInfo &info)
{
    btVector3 offset = m_rootBone.offset();
    uint8_t *ptr = const_cast<uint8_t *>(info.constraintsPtr);
    const uint32_t nconstraints = info.constranitsCount;
    m_constraints.reserve(nconstraints);
    for (uint32_t i = 0; i < nconstraints; i++) {
        Constraint *constraint = new Constraint();
        constraint->read(ptr, m_rigidBodies, offset);
        ptr += Constraint::stride();
        m_constraints.add(constraint);
    }
}

void PMDModel::release()
{
    internal::zerofill(&m_name, sizeof(m_name));
    internal::zerofill(&m_comment, sizeof(m_comment));
    internal::zerofill(&m_englishName, sizeof(m_englishName));
    internal::zerofill(&m_englishComment, sizeof(m_englishComment));
    leaveWorld(m_world);
    m_vertices.clear();
    m_materials.clear();
    m_bones.clear();
    m_IKs.clear();
    m_faces.clear();
    m_rigidBodies.clear();
    m_constraints.clear();
    m_bonesForUI.clear();
    m_boneCategoryNames.clearArray();
    delete[] m_orderedBones;
    delete[] m_skinnedVertices;
    delete[] m_indicesPointer;
    delete[] m_edgeIndicesPointer;
    m_baseFace = 0;
    m_orderedBones = 0;
    m_skinnedVertices = 0;
    m_indicesPointer = 0;
    m_edgeIndicesPointer = 0;
    m_edgeIndicesCount = 0;
    m_error = kNoError;
}

void PMDModel::sortBones()
{
    const uint32_t nbones = m_bones.count();
    if (nbones > 0) {
        delete[] m_orderedBones;
        m_orderedBones = new Bone*[nbones];
        uint32_t k = 0;
        for (uint32_t i = 0; i < nbones; i++) {
            Bone *bone = m_bones[i];
            if (!bone->hasParent())
                m_orderedBones[k++] = bone;
        }
        uint32_t l = k;
        for (uint32_t i = 0; i < nbones; i++) {
            Bone *bone = m_bones[i];
            if (bone->hasParent())
                m_orderedBones[l++] = bone;
        }
        uint32_t i = 0;
        do {
            for (uint32_t j = k; j < nbones; j++) {
                for (l = 0; l < j; l++) {
                    if (m_orderedBones[l] == m_orderedBones[j]->parent())
                        break;
                }
                if (l >= j) {
                    Bone *bone = m_orderedBones[j];
                    if (j < nbones - 1)
                        memmove(m_orderedBones[j], m_orderedBones[j+1], sizeof(Bone *) * (nbones - 1 - j));
                    m_orderedBones[nbones - 1] = bone;
                    i = 1;
                }
            }
        } while (i != 0);
    }
}

size_t PMDModel::stride(StrideType type) const
{
    switch (type) {
    case kVerticesStride:
    case kNormalsStride:
    case kTextureCoordsStride:
        return sizeof(SkinVertex);
    case kEdgeVerticesStride:
    case kToonTextureStride:
        return sizeof(btVector3);
    case kIndicesStride:
    case kEdgeIndicesStride:
        return sizeof(uint16_t);
    default:
        return 0;
    }
}

const void *PMDModel::verticesPointer() const
{
    return &m_skinnedVertices[0].position;
}

const void *PMDModel::normalsPointer() const
{
    return &m_skinnedVertices[0].normal;
}

const void *PMDModel::textureCoordsPointer() const
{
    return &m_skinnedVertices[0].texureCoord;
}

const void *PMDModel::toonTextureCoordsPointer() const
{
    return &m_toonTextureCoords[0];
}

const void *PMDModel::edgeVerticesPointer() const
{
    return &m_edgeVertices[0];
}

}
