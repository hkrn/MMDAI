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
VPVL_DECLARE_HANDLE(btDiscreteDynamicsWorld)
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

#pragma pack(push, 1)
struct Header
{
    uint8_t signature[3];
    float version;
    uint8_t name[PMDModel::kNameSize];
    uint8_t comment[PMDModel::kCommentSize];
};
#pragma pack(pop)

struct SkinVertex
{
    Vector3 position;
    Vector3 normal;
    Vector4 textureCoord;
    Vector3 bone;
    Vector3 edge;
};

struct State
{
    const PMDModel *model;
    Array<Vector3> positions;
    Array<Quaternion> rotations;
    Array<float> weights;
};

PMDModel::PMDModel()
    : m_baseBone(0),
      m_baseFace(0),
      m_orderedBones(0),
      m_skinnedVertices(0),
      m_world(0),
      m_indicesPointer(0),
      m_edgeIndicesPointer(0),
      m_edgeIndicesCount(0),
      m_rotationOffset(0.0f, 0.0f, 0.0f, 1.0f),
      m_edgeColor(0.0f, 0.0f, 0.0f, 1.0f),
      m_positionOffset(0.0f, 0.0f, 0.0f),
      m_lightPosition(0.0f, 0.0f, 0.0f),
      m_error(kNoError),
      m_edgeOffset(0.03f),
      m_selfShadowDensityCoef(0.0f),
      m_enableSimulation(false),
      m_enableSoftwareSkinning(true),
      m_enableToon(true),
      m_visible(false)
{
    internal::zerofill(&m_name, sizeof(m_name));
    internal::zerofill(&m_comment, sizeof(m_comment));
    internal::zerofill(&m_englishName, sizeof(m_englishName));
    internal::zerofill(&m_englishComment, sizeof(m_englishComment));
    m_rootBone.setRotation(Quaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_rootBone.updateTransform();
}

PMDModel::~PMDModel()
{
    release();
}

void PMDModel::prepare()
{
    m_skinningTransform.resize(m_bones.count());
    const int nvertices = m_vertices.count();
    m_skinnedVertices = new SkinVertex[nvertices];
    m_edgeIndicesPointer = new uint16_t[m_indices.count()];
    uint16_t *from = m_indicesPointer, *to = m_edgeIndicesPointer;
    const int nmaterials = m_materials.count();
    // Create edge vertices (copy model vertices if the material is enabled)
    for (int i = 0; i < nmaterials; i++) {
        const Material *material = m_materials[i];
        int nindices = material->countIndices();
        if (material->isEdgeEnabled()) {
            memcpy(to, from, sizeof(uint16_t) * nindices);
            to += nindices;
            m_edgeIndicesCount += nindices;
        }
        from += nindices;
    }
    // Initialize skin vertices and set values from vertex
    for (int i = 0; i < nvertices; i++) {
        const Vertex *vertex = m_vertices[i];
        SkinVertex &skinnedVertex = m_skinnedVertices[i];
        skinnedVertex.position = vertex->position();
        skinnedVertex.normal = vertex->normal();
        skinnedVertex.textureCoord.setValue(vertex->u(), vertex->v(), 0.0f, 0.0f);
        skinnedVertex.bone.setValue(vertex->bone1(), vertex->bone2(), vertex->weight());
        skinnedVertex.edge.setZero();
    }
    // Find kUnderRotate or kFollowRotate bones to update rotation value after IK
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        const Bone::Type type = bone->type();
        if (type == Bone::kUnderRotate || type == Bone::kFollowRotate)
            m_rotatedBones.add(bone);
    }
    const int nIKs = m_IKs.count();
    m_isIKSimulated.reserve(nIKs);
    // IK simulation: enabled => physic engine / disabled => IK#solve
    for (int i = 0; i < nIKs; i++) {
        m_isIKSimulated.add(m_IKs[i]->isSimulated());
    }
}

void PMDModel::addMotion(VMDMotion *motion)
{
    motion->attachModel(this);
    m_motions.add(motion);
    // FIXME: priority issue of compatibility with MMDAgent
    // m_motions.sort(VMDMotionPriorityPredication());
}

void PMDModel::joinWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL_NO_BULLET
    if (!world)
        return;
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->setKinematic(false);
        world->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->groupMask());
    }
    const int nconstraints = m_constraints.count();
    for (int i = 0; i < nconstraints; i++) {
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
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->setKinematic(true);
        world->removeCollisionObject(rigidBody->body());
    }
    const int nconstraints = m_constraints.count();
    for (int i = 0; i < nconstraints; i++) {
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
    const int nbones = m_bones.count(), nfaces = m_faces.count();
    bool ret = false;
    if (state->model == this) {
        for (int i = 0; i < nbones; i++) {
            Bone *bone = m_bones[i];
            bone->setPosition(state->positions[i]);
            bone->setRotation(state->rotations[i]);
        }
        for (int i = 0; i < nfaces; i++) {
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
    const int nbones = m_bones.count(), nfaces = m_faces.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        state->positions.add(bone->position());
        state->rotations.add(bone->rotation());
    }
    for (int i = 0; i < nfaces; i++) {
        Face *face = m_faces[i];
        state->weights.add(face->weight());
    }
    state->model = this;
    return state;
}

void PMDModel::resetMotion()
{
    const int nmotions = m_motions.count();
    for (int i = 0; i < nmotions; i++)
        m_motions[i]->reset();
    updateAllBones();
    updateAllFaces();
    updateBoneFromSimulation();
}

float PMDModel::maxFrameIndex() const
{
    const int nmotions = m_motions.count();
    float max = 0.0f;
    for (int i = 0; i < nmotions; i++)
        btSetMax(max, m_motions[i]->maxFrameIndex());
    return max;
}

bool PMDModel::isMotionReachedTo(float frameIndex) const
{
    const int nmotions = m_motions.count();
    bool ret = true;
    for (int i = 0; i < nmotions; i++)
        ret = ret && m_motions[i]->isReachedTo(frameIndex);
    return ret;
}

void PMDModel::seekMotion(float frameIndex)
{
    const int nmotions = m_motions.count();
    for (int i = 0; i < nmotions; i++) {
        vpvl::VMDMotion *motion = m_motions[i];
        motion->seek(frameIndex);
    }
    updateAllBones();
    updateAllFaces();
    updateBoneFromSimulation();
}

void PMDModel::updateRootBone()
{
    m_rootBone.setPosition(m_positionOffset);
    m_rootBone.setRotation(m_rotationOffset);
    m_rootBone.updateTransform();
    if (m_baseBone) {
        m_rootBone.setLocalTransform(m_baseBone->localTransform() * m_rootBone.localTransform());
        m_rootBone.updateTransform();
    }
}

void PMDModel::advanceMotion(float deltaFrame)
{
    const int nmotions = m_motions.count();
    for (int i = 0; i < nmotions; i++) {
        vpvl::VMDMotion *motion = m_motions[i];
        motion->advance(deltaFrame);
    }
    updateAllBones();
    updateAllFaces();
    updateBoneFromSimulation();
}

void PMDModel::updateSkins()
{
    if (m_enableSoftwareSkinning) {
        updateSkinVertices();
        updateToon(m_lightPosition);
    }
    else {
        updateBoneMatrices();
        updatePosition();
    }
}

void PMDModel::updateAllBones()
{
    const int nbones = m_bones.count(), nIKs = m_IKs.count();
    for (int i = 0; i < nbones; i++)
        m_orderedBones[i]->updateTransform();
    if (m_enableSimulation) {
        for (int i = 0; i < nIKs; i++) {
            // Solve IK with physic engine instead of IK class if it's disabled
            if (!m_isIKSimulated[i])
                m_IKs[i]->solve();
        }
    }
    else {
        for (int i = 0; i < nIKs; i++)
            m_IKs[i]->solve();
    }
    const int nRotatedBones = m_rotatedBones.count();
    for (int i = 0; i < nRotatedBones; i++)
        m_rotatedBones[i]->updateRotation();
}

void PMDModel::updateBoneFromSimulation()
{
    if (m_enableSimulation) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++)
            m_rigidBodies[i]->transformBone();
    }
}

void PMDModel::updateAllFaces()
{
    const int nfaces = m_faces.count();
    if (nfaces > 0)
        m_baseFace->setBaseVertices(m_vertices);
    for (int i = 0; i < nfaces; i++) {
        Face *face = m_faces[i];
        const float weight = face->weight();
        if (weight > kMinFaceWeight)
            face->setVertices(m_vertices, weight);
    }
}

void PMDModel::updateShadowTextureCoords(float coef)
{
    bool update = false;
    const int nvertices = m_vertices.count();
    if (m_shadowTextureCoords.count() == 0) {
        m_shadowTextureCoords.reserve(nvertices);
        update = true;
    }
    else if (coef) {
        update = true;
    }
    if (update) {
        for (int i = 0; i < nvertices; i++)
            m_shadowTextureCoords[i].setValue(0.0f, coef, 0.0f);
        m_selfShadowDensityCoef = coef;
    }
}

void PMDModel::updateSkinVertices()
{
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++)
        m_bones[i]->getSkinTransform(m_skinningTransform[i]);
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        const Vertex *vertex = m_vertices[i];
        SkinVertex &skin = m_skinnedVertices[i];
        const float weight = vertex->weight();
        if (weight >= 1.0f - kMinBoneWeight) {
            const int16_t bone1 = vertex->bone1();
            const Transform &transform = m_skinningTransform[bone1];
            skin.position = transform * vertex->position();
            skin.normal = transform.getBasis() * vertex->normal();
        }
        else if (weight <= kMinBoneWeight) {
            const int16_t bone2 = vertex->bone2();
            const Transform &transform = m_skinningTransform[bone2];
            skin.position = transform * vertex->position();
            skin.normal = transform.getBasis() * vertex->normal();
        }
        else {
            const int16_t bone1 = vertex->bone1();
            const int16_t bone2 = vertex->bone2();
            const Vector3 &position = vertex->position();
            const Vector3 &normal = vertex->normal();
            const Transform &transform1 = m_skinningTransform[bone1];
            const Transform &transform2 = m_skinningTransform[bone2];
            const Vector3 &v1 = transform1 * position;
            const Vector3 &n1 = transform1.getBasis() * normal;
            const Vector3 &v2 = transform2 * position;
            const Vector3 &n2 = transform2.getBasis() * normal;
            skin.position.setInterpolate3(v2, v1, weight);
            skin.normal.setInterpolate3(n2, n1, weight);
        }
        skin.position.setW(1.0f);
    }
}

void PMDModel::updateToon(const Vector3 &lightDirection)
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        SkinVertex &skin = m_skinnedVertices[i];
        const Scalar &v = (1.0f - lightDirection.dot(skin.normal)) * 0.5f;
        skin.textureCoord.setW(v);
        if (!m_vertices[i]->isEdgeEnabled())
            skin.edge = skin.position;
        else
            skin.edge = skin.position + skin.normal * m_edgeOffset;
    }
}

void PMDModel::updateImmediate()
{
    updateRootBone();
    updateAllBones();
    updateAllFaces();
    updateBoneFromSimulation();
    updateSkins();
}

void PMDModel::updateBoneMatrices()
{
    const int nbones = m_bones.count();
    if (m_boneMatrices.count() == 0)
        m_boneMatrices.reserve(nbones << 4);
    Transform transform;
    for (int i = 0; i < nbones; i++) {
        vpvl::Bone *bone = m_bones[i];
        bone->getSkinTransform(transform);
        transform.getOpenGLMatrix(&m_boneMatrices[i << 4]);
    }
}

void PMDModel::updatePosition()
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        const Vertex *vertex = m_vertices[i];
        SkinVertex &skinnedVertex = m_skinnedVertices[i];
        Vector3 &position = skinnedVertex.position;
        position = vertex->position();
        position.setW(1.0f);
        skinnedVertex.edge.setValue(vertex->isEdgeEnabled() ? m_edgeOffset : 0.0f, 0.0f, 0.0f);
    }
}

void PMDModel::updateIndices()
{
    const int nindices = m_indices.count();
    if (nindices > 0) {
        m_indicesPointer = new uint16_t[nindices];
        internal::copyBytes(reinterpret_cast<uint8_t *>(m_indicesPointer),
                            reinterpret_cast<const uint8_t *>(&m_indices[0]),
                            sizeof(uint16_t) * nindices);
#ifdef VPVL_COORDINATE_OPENGL
        for (int i = 0; i < nindices; i += 3) {
            const uint16_t index = m_indicesPointer[i];
            m_indicesPointer[i] = m_indicesPointer[i + 1];
            m_indicesPointer[i + 1] = index;
        }
#endif
    }
}

void PMDModel::getBoundingSphere(Vector3 &center, Scalar &radius) const
{
    Scalar max = 0.0f;
    Bone *centerBone = Bone::centerBone(&m_bones);
    const Vector3 &centerBoneOrigin = centerBone->localTransform().getOrigin();
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++)
        btSetMax(max, centerBoneOrigin.distance2(m_skinnedVertices[i].position));
    radius = btSqrt(max) * 1.1f;
    center = centerBoneOrigin;
}

void PMDModel::resetAllBones()
{
    m_rootBone.reset();
    m_rootBone.updateTransform();
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
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
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        bone->setPosition(internal::kZeroV.lerp(bone->position(), rate));
        bone->setRotation(internal::kZeroQ.slerp(bone->rotation(), rate));
    }
}

void PMDModel::smearAllFacesToDefault(float rate)
{
    const int nfaces = m_faces.count();
    for (int i = 0; i < nfaces; i++) {
        Face *face = m_faces[i];
        face->setWeight(face->weight() * rate);
    }
}

bool PMDModel::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header[3] + Version[4] + Name[20] + Comment[256]
    if (!data || sizeof(Header) > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header *header = reinterpret_cast<Header *>(ptr);
    info.basePtr = ptr;

    // Check the signature and version is correct
    if (memcmp(header->signature, "Pmd", 3) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
#ifdef VPVL_BUILD_IOS
    float version;
    memcpy(&version, &header->version, sizeof(version));
    if (1.0f != version) {
#else
    if (1.0f != header->version) {
#endif
        m_error = kInvalidVersionError;
        return false;
    }

    // Name and Comment (in Shift-JIS)
    info.namePtr = header->name;
    info.commentPtr = header->comment;
    ptr += sizeof(Header);
    rest -= sizeof(Header);

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
    if (nBones == 0) {
        m_error = kNoBoneError;
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
        if ((required + kNameSize + kCommentSize) > rest) {
            m_error = kEnglishNamesError;
            return false;
        }
        info.englishNamePtr = ptr;
        ptr += kNameSize;
        info.englishCommentPtr = ptr;
        ptr += kCommentSize;
        info.englishBoneNamesPtr = ptr;
        ptr += englishBoneNamesSize;
        info.englishFaceNamesPtr = ptr;
        ptr += englishFaceNamesSize;
        info.englishBoneFramesPtr = ptr;
        ptr += englishBoneCategoryNameSize;
        rest -= required;
    }

    // Extra texture path (100 * 10)
    size_t customTextureNameSize = (kCustomTextureMax - 1) * kCustomTextureNameMax;
    if (customTextureNameSize > rest) {
        m_error = kExtraTextureNamesError;
        return false;
    }
    info.toonTextureNamesPtr = ptr;
    ptr += customTextureNameSize;
    rest -= customTextureNameSize;

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

size_t PMDModel::estimateSize() const
{
    const int nIKs = m_IKs.count(), nbones = m_bones.count(),
            nfaces = m_faces.count(), nBoneCategories = m_boneCategoryNames.count();
    size_t size = sizeof(Header)
            + sizeof(uint32_t) + m_vertices.count() * Vertex::stride()
            + sizeof(uint32_t) + m_indices.count() * sizeof(int16_t)
            + sizeof(uint32_t) + m_materials.count() * Material::stride()
            + sizeof(uint16_t) + m_bones.count() * Bone::stride()
            + sizeof(uint16_t)  // IK
            + sizeof(uint16_t)  // face
            + sizeof(uint8_t)  + sizeof(uint16_t) * m_facesForUI.count()
            + sizeof(uint8_t)  + kBoneCategoryNameSize * nBoneCategories
            + sizeof(uint32_t)  // Bones for UI
            + sizeof(uint8_t)   // have english names
            + ((kCustomTextureMax - 1) * kCustomTextureNameMax)
            + sizeof(uint32_t) + m_rigidBodies.count() * RigidBody::stride()
            + sizeof(uint32_t) + m_constraints.count() * Constraint::stride();
    for (int i = 0; i < nIKs; i++)
        size += m_IKs.at(i)->estimateSize();
    for (int i = 0; i < nfaces; i++)
        size += m_faces.at(i)->estimateSize();
    for (int i = 0; i < nBoneCategories; i++) {
        BoneList *bones = m_bonesForUI.at(i);
        size += (sizeof(uint16_t) + sizeof(uint8_t)) * bones->count();
    }
    const size_t englishBoneNamesSize = Bone::kNameSize * nbones;
    const size_t englishBoneCategoryNameSize = kBoneCategoryNameSize * nBoneCategories;
    const size_t englishFaceNamesSize = nfaces > 0 ? (nfaces - 1) * Face::kNameSize : 0;
    size += kNameSize + kCommentSize + englishBoneNamesSize
            + englishFaceNamesSize + englishBoneCategoryNameSize;
    return size;
}

void PMDModel::save(uint8_t *data) const
{
    Header header;
    header.version = 1.0f;
    internal::copyBytes(header.signature, reinterpret_cast<const uint8_t *>("Pmd"), sizeof(header.signature));
    internal::copyBytes(header.name, m_name, sizeof(header.name));
    internal::copyBytes(header.comment, m_comment, sizeof(header.comment));
    uint8_t *ptr = data;
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&header), sizeof(header));
    ptr += sizeof(header);
    int nvertices = m_vertices.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nvertices), sizeof(nvertices));
    ptr += sizeof(nvertices);
    for (int i = 0; i < nvertices; i++) {
        m_vertices.at(i)->write(ptr);
        ptr += Vertex::stride();
    }
    int nindices = m_indices.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nindices), sizeof(nindices));
    ptr += sizeof(nindices);
    for (int i = 0; i < nindices; i++) {
        internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&m_indices[i]), sizeof(uint16_t));
        ptr += sizeof(uint16_t);
    }
    int nmaterilals = m_materials.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nmaterilals), sizeof(nmaterilals));
    ptr += sizeof(nmaterilals);
    for (int i = 0; i < nmaterilals; i++) {
        m_materials.at(i)->write(ptr);
        ptr += Material::stride();
    }
    uint16_t nbones = m_bones.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nbones), sizeof(nbones));
    ptr += sizeof(nbones);
    for (int i = 0; i < nbones; i++) {
        m_bones.at(i)->write(ptr);
        ptr += Bone::stride();
    }
    uint16_t nIKs = m_IKs.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nIKs), sizeof(nIKs));
    ptr += sizeof(nIKs);
    for (int i = 0; i < nIKs; i++) {
        IK *IK = m_IKs[i];
        IK->write(ptr);
        ptr += IK->estimateSize();
    }
    uint16_t nfaces = m_faces.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nfaces), sizeof(nfaces));
    ptr += sizeof(nfaces);
    for (int i = 0; i < nfaces; i++) {
        Face *face = m_faces[i];
        face->write(ptr);
        ptr += face->estimateSize();
    }
    uint8_t nFacesForUI = m_facesForUI.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nFacesForUI), sizeof(nFacesForUI));
    ptr += sizeof(nFacesForUI);
    for (int i = 0; i < nFacesForUI; i++) {
        uint16_t index = m_facesForUIIndices.at(i);
        internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&index), sizeof(index));
        ptr += sizeof(index);
    }
    uint8_t nBoneCategoryNames = m_boneCategoryNames.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nBoneCategoryNames), sizeof(nBoneCategoryNames));
    ptr += sizeof(nBoneCategoryNames);
    int nBonesForUI = 0;
    for (int i = 0; i < nBoneCategoryNames; i++) {
        const uint8_t *name = m_boneCategoryNames.at(i);
        internal::copyBytes(ptr, name, kBoneCategoryNameSize);
        ptr += kBoneCategoryNameSize;
        nBonesForUI += m_bonesForUI[i]->count();
    }
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nBonesForUI), sizeof(nBonesForUI));
    ptr += sizeof(nBonesForUI);
    for (int i = 0; i < nBoneCategoryNames; i++) {
        BoneList *bones = m_bonesForUI.at(i);
        int nBonesInCategory = bones->count();
        for (int j = 0; j < nBonesInCategory; j++) {
            uint16_t boneID = bones->at(j)->id();
            internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&boneID), sizeof(boneID));
            ptr += sizeof(boneID);
            uint8_t categoryIndex = i + 1;
            internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&categoryIndex), sizeof(categoryIndex));
            ptr += sizeof(categoryIndex);
        }
    }
    uint8_t hasEnglish = 0;
    if (m_englishName[0]) {
        hasEnglish = 1;
        internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&hasEnglish), sizeof(hasEnglish));
        ptr += sizeof(hasEnglish);
        internal::copyBytes(ptr, m_englishName, kNameSize);
        ptr += kNameSize;
        internal::copyBytes(ptr, m_englishComment, kCommentSize);
        ptr += kCommentSize;
        for (int i = 0; i < nbones; i++) {
            internal::copyBytes(ptr, m_bones.at(i)->englishName(), Bone::kNameSize);
            ptr += Bone::kNameSize;
        }
        for (int i = 0; i < nfaces; i++) {
            Face *face = m_faces[i];
            if (face->type() != Face::kBase) {
                internal::copyBytes(ptr, face->englishName(), Face::kNameSize);
                ptr += Face::kNameSize;
            }
        }
        for (int i = 0; i < nBoneCategoryNames; i++) {
            const uint8_t *name = m_boneCategoryEnglishNames.at(i);
            internal::copyBytes(ptr, name, kBoneCategoryNameSize);
            ptr += kBoneCategoryNameSize;
        }
    }
    else {
        internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&hasEnglish), sizeof(hasEnglish));
        ptr += sizeof(hasEnglish);
    }
    for (int i = 0; i < kCustomTextureMax - 1; i++) {
        internal::copyBytes(ptr, m_textures[i], kCustomTextureNameMax);
        ptr += kCustomTextureNameMax;
    }
    int nRigidBodies = m_rigidBodies.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nRigidBodies), sizeof(nRigidBodies));
    ptr += sizeof(nRigidBodies);
    for (int i = 0; i < nRigidBodies; i++) {
        m_rigidBodies.at(i)->write(ptr);
        ptr += RigidBody::stride();
    }
    int nconstraints = m_constraints.count();
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&nconstraints), sizeof(nconstraints));
    ptr += sizeof(nconstraints);
    for (int i = 0; i < nconstraints; i++) {
        m_constraints.at(i)->write(ptr);
        ptr += Constraint::stride();
    }
}

void PMDModel::parseHeader(const DataInfo &info)
{
    setName(info.namePtr);
    setComment(info.commentPtr);
}

void PMDModel::parseVertices(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.verticesPtr);
    const int nvertices = info.verticesCount;
    m_vertices.reserve(nvertices);
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        vertex->read(ptr);
        ptr += Vertex::stride();
        m_vertices.add(vertex);
    }
}

void PMDModel::parseIndices(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.indicesPtr);
    const int nindices = info.indicesCount;
    m_indices.reserve(nindices);
    for (int i = 0; i < nindices; i++) {
        m_indices.add(*reinterpret_cast<uint16_t *>(ptr));
        ptr += sizeof(uint16_t);
    }
    updateIndices();
}

void PMDModel::parseMatrials(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.materialsPtr);
    const int nmaterials = info.materialsCount;
    m_materials.reserve(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
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
    const int nbones = info.bonesCount;
    m_bones.reserve(nbones);
    for (int i = 0; i < nbones; i++) {
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
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        bone->build(&m_bones, &m_rootBone);
    }
    sortBones();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_orderedBones[i];
        bone->updateTransform();
    }
}

void PMDModel::parseIKs(const DataInfo &info)
{
    uint8_t *ptr = const_cast<uint8_t *>(info.IKsPtr);
    const int nIKs = info.IKsCount;
    BoneList *mutableBones = this->mutableBones();
    m_IKs.reserve(nIKs);
    for (int i = 0; i < nIKs; i++) {
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
    const int nfaces = info.facesCount;
    m_faces.reserve(nfaces);
    for (int i = 0; i < nfaces; i++) {
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
        for (int i = 0; i < nfaces; i++) {
            m_faces[i]->convertIndices(baseFace);
        }
    }
}

void PMDModel::parseFacesForUI(const DataInfo &info)
{
    const int nFacesForUI = info.facesForUICount;
    const int nfaces = m_faces.count();
    uint16_t *ptr = reinterpret_cast<uint16_t *>(const_cast<uint8_t *>(info.facesForUIPtr));
    for (int i = 0; i < nFacesForUI; i++) {
        uint16_t faceIndex = *ptr;
        if (faceIndex < nfaces) {
            Face *face = m_faces[faceIndex];
            // XXX: out of index risk
            m_facesForUI.add(face);
            m_facesForUIIndices.add(faceIndex);
        }
        ptr++;
    }
}

void PMDModel::parseBoneCategoryNames(const DataInfo &info)
{
    const uint8_t nBoneCategoryNames = info.boneCategoryNamesCount;
    uint8_t *ptr = const_cast<uint8_t *>(info.boneCategoryNamesPtr);
    uint8_t *englishPtr = const_cast<uint8_t *>(info.englishBoneFramesPtr);
    for (int i = 0; i < nBoneCategoryNames; i++) {
        uint8_t *name = new uint8_t[kBoneCategoryNameSize];
        copyBytesSafe(name, ptr, kBoneCategoryNameSize);
        m_boneCategoryNames.add(name);
        ptr += kBoneCategoryNameSize;
        if (englishPtr) {
            uint8_t *englishName = new uint8_t[kBoneCategoryNameSize];
            copyBytesSafe(englishName, englishPtr, kBoneCategoryNameSize);
            m_boneCategoryEnglishNames.add(englishName);
            englishPtr += kBoneCategoryNameSize;
        }
    }
}

void PMDModel::parseBonesForUI(const DataInfo &info)
{
    const int nBonesCategoryNames = m_boneCategoryNames.count();
    const int nBonesForUI = info.bonesForUICount;
    const int nbones = m_bones.count();
    uint8_t *ptr = const_cast<uint8_t *>(info.bonesForUIPtr);
    m_bonesForUI.reserve(nBonesCategoryNames);
    for (int i = 0; i < nBonesCategoryNames; i++)
        m_bonesForUI.add(new BoneList());
    for (int i = 0; i < nBonesForUI; i++) {
        uint16_t boneIndex = *reinterpret_cast<uint16_t *>(ptr);
        ptr += sizeof(uint16_t);
        if (boneIndex < nbones) {
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
    const int nRigidBodies = info.rigidBodiesCount;
    m_rigidBodies.reserve(nRigidBodies);
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody();
        rigidBody->read(ptr, mutableBones);
        ptr += RigidBody::stride();
        m_rigidBodies.add(rigidBody);
    }
}

void PMDModel::parseConstraints(const DataInfo &info)
{
    Vector3 offset = m_rootBone.offset();
    uint8_t *ptr = const_cast<uint8_t *>(info.constraintsPtr);
    const int nconstraints = info.constranitsCount;
    m_constraints.reserve(nconstraints);
    for (int i = 0; i < nconstraints; i++) {
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
    m_rotationOffset.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_edgeColor.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_positionOffset.setZero();
    m_lightPosition.setZero();
    m_vertices.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_IKs.releaseAll();
    m_faces.releaseAll();
    m_rigidBodies.releaseAll();
    m_constraints.releaseAll();
    m_bonesForUI.releaseAll();
    m_boneCategoryNames.releaseArrayAll();
    m_boneCategoryEnglishNames.releaseArrayAll();
    delete[] m_orderedBones;
    delete[] m_skinnedVertices;
    delete[] m_indicesPointer;
    delete[] m_edgeIndicesPointer;
    m_baseBone = 0;
    m_baseFace = 0;
    m_orderedBones = 0;
    m_skinnedVertices = 0;
    m_indicesPointer = 0;
    m_edgeIndicesPointer = 0;
    m_edgeIndicesCount = 0;
    m_error = kNoError;
    m_visible = false;
}

void PMDModel::sortBones()
{
    const int nbones = m_bones.count();
    if (nbones > 0) {
        delete[] m_orderedBones;
        m_orderedBones = new Bone*[nbones];
        int k = 0;
        for (int i = 0; i < nbones; i++) {
            Bone *bone = m_bones[i];
            if (!bone->hasParent())
                m_orderedBones[k++] = bone;
        }
        int l = k;
        for (int i = 0; i < nbones; i++) {
            Bone *bone = m_bones[i];
            if (bone->hasParent())
                m_orderedBones[l++] = bone;
        }
        int i = 0;
        do {
            for (int j = k; j < nbones; j++) {
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

size_t PMDModel::strideSize(StrideType type) const
{
    switch (type) {
    case kVerticesStride:
    case kNormalsStride:
    case kTextureCoordsStride:
    case kToonTextureStride:
    case kEdgeVerticesStride:
    case kBoneAttributesStride:
        return sizeof(SkinVertex);
    case kIndicesStride:
    case kEdgeIndicesStride:
        return sizeof(uint16_t);
    default:
        return 0;
    }
}

size_t PMDModel::strideOffset(StrideType type) const
{
    static SkinVertex v;
    switch (type) {
    case kVerticesStride:
    case kIndicesStride:
    case kEdgeIndicesStride:
        return 0;
    case kNormalsStride:
        return reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position);
    case kTextureCoordsStride:
        return reinterpret_cast<const uint8_t *>(&v.textureCoord.x()) - reinterpret_cast<const uint8_t *>(&v.position);
    case kToonTextureStride:
        return reinterpret_cast<const uint8_t *>(&v.textureCoord.z()) - reinterpret_cast<const uint8_t *>(&v.position);
    case kBoneAttributesStride:
        return reinterpret_cast<const uint8_t *>(&v.bone) - reinterpret_cast<const uint8_t *>(&v.position);
    case kEdgeVerticesStride:
        return reinterpret_cast<const uint8_t *>(&v.edge) - reinterpret_cast<const uint8_t *>(&v.position);
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
    return &m_skinnedVertices[0].textureCoord.x();
}

const void *PMDModel::toonTextureCoordsPointer() const
{
    return &m_skinnedVertices[0].textureCoord.z();
}

const void *PMDModel::edgeVerticesPointer() const
{
    return &m_skinnedVertices[0].edge;
}

const void *PMDModel::boneAttributesPointer() const
{
    return &m_skinnedVertices[0].bone;
}

const float *PMDModel::boneMatricesPointer() const
{
    return &m_boneMatrices[0];
}

const uint8_t *PMDModel::toonTexture(int index) const
{
    if (index >= kCustomTextureMax)
        return 0;
    return m_textures[index];
}

void PMDModel::setToonTextures(const uint8_t *ptr)
{
    uint8_t *p = const_cast<uint8_t *>(ptr);
    if (p) {
        for (int i = 0; i < kCustomTextureMax; i++) {
            copyBytesSafe(m_textures[i], p, sizeof(m_textures[i]));
            p += kCustomTextureNameMax;
        }
    }
    else {
        for (int i = 0; i < kCustomTextureMax; i++)
            internal::snprintf(m_textures[i], kCustomTextureNameMax, "toon%02d.bmp", i + 1);
    }
}

void PMDModel::setSoftwareSkinningEnable(bool value)
{
    m_enableSoftwareSkinning = value;
    updateSkins();
}

Bone *PMDModel::findBone(const uint8_t *name) const
{
    const HashString key(reinterpret_cast<const char *>(name));
    Bone **ptr = const_cast<Bone **>(m_name2bone.find(key));
    return ptr ? *ptr : 0;
}

Face *PMDModel::findFace(const uint8_t *name) const
{
    const HashString key(reinterpret_cast<const char *>(name));
    Face **ptr = const_cast<Face **>(m_name2face.find(key));
    return ptr ? *ptr : 0;
}

}
