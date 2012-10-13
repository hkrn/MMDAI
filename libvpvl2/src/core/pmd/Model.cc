/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"
#include "vpvl2/pmd/Bone.h"
#include "vpvl2/pmd/Joint.h"
#include "vpvl2/pmd/Label.h"
#include "vpvl2/pmd/Material.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Morph.h"
#include "vpvl2/pmd/RigidBody.h"
#include "vpvl2/pmd/Vertex.h"

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btDiscreteDynamicsWorld);
#endif

namespace
{

using namespace vpvl2;

#pragma pack(push, 1)

struct Header
{
    uint8_t signature[3];
    float version;
    uint8_t name[pmd::Model::kNameSize];
    uint8_t comment[pmd::Model::kCommentSize];
};

#pragma pack(pop)

struct StaticVertexBuffer : public IModel::IStaticVertexBuffer {
    struct Unit {
        Unit() {}
        Unit(const IVertex *vertex) {
            IBone *bone1 = vertex->bone(0), *bone2 = vertex->bone(1);
            texcoord = vertex->textureCoord();
            boneIndices.setValue(bone1->index(), bone2->index(), 0, 0);
            boneWeights.setValue(vertex->weight(0), 0, 0, 0);
        }
        Vector3 texcoord;
        Vector4 boneIndices;
        Vector4 boneWeights;
    };
    static const Unit kIdent;

    StaticVertexBuffer(const pmd::Model *model)
        : modelRef(model)
    {
        Array<IVertex *> vertices;
        model->getVertexRefs(vertices);
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            const IVertex *vertex = vertices[i];
            units.add(Unit(vertex));
        }
    }

    const void *bytes() const {
        return &units[0];
    }
    size_t size() const {
        return strideSize() * units.count();
    }
    size_t strideOffset(StrideType type) const {
        const uint8_t *base = reinterpret_cast<const uint8_t *>(&kIdent.texcoord);
        switch (type) {
        case kBoneIndexStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.boneIndices) - base;
        case kBoneWeightStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.boneWeights) - base;
        case kTextureCoordStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.texcoord) - base;
        case kVertexStride:
        case kNormalStride:
        case kMorphDeltaStride:
        case kEdgeSizeStride:
        case kEdgeVertexStride:
        case kUVA0Stride:
        case kUVA1Stride:
        case kUVA2Stride:
        case kUVA3Stride:
        case kUVA4Stride:
        case kVertexIndexStride:
        case kIndexStride:
        default:
            return 0;
        }
    }
    size_t strideSize() const {
        return sizeof(Unit);
    }
    const void *ident() const {
        return &kIdent;
    }

    const IModel *modelRef;
    Array<Unit> units;
};
const StaticVertexBuffer::Unit StaticVertexBuffer::kIdent = StaticVertexBuffer::Unit();

struct DynamicVertexBuffer : public IModel::IDynamicVertexBuffer {
    struct Unit {
        Unit() {}
        Unit(const IVertex *vertex, int index) {
            position = vertex->origin();
            normal = vertex->normal();
            normal[3] = vertex->edgeSize();
            delta = vertex->delta();
            edge.setZero();
            edge[3] = Scalar(index);
            uva0.setValue(0, 0, 0, 1);
        }
        Vector3 position;
        Vector3 normal;
        Vector3 delta;
        Vector3 edge;
        Vector4 uva0;
    };
    static const Unit kIdent;

    DynamicVertexBuffer(const IModel *model, const IModel::IIndexBuffer *indexBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          enableSkinning(true)
    {
        model->getMaterialRefs(materials);
        model->getVertexRefs(vertices);
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            const IVertex *vertex = vertices[i];
            units.add(Unit(vertex, i));
        }
    }

    void update(const Vector3 &cameraPosition, Vector3 &aabbMin, Vector3 &aabbMax) {
        update(cameraPosition, &units[0], aabbMin, aabbMax);
    }
    const void *bytes() const {
        return &units[0];
    }
    size_t size() const {
        return strideSize() * units.count();
    }
    size_t strideOffset(StrideType type) const {
        const uint8_t *base = reinterpret_cast<const uint8_t *>(&kIdent.position);
        switch (type) {
        case kVertexStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.position) - base;
        case kNormalStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.normal) - base;
        case kMorphDeltaStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.delta) - base;
        case kEdgeVertexStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.edge) - base;
        case kEdgeSizeStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.normal[3]) - base;
        case kVertexIndexStride:
            return reinterpret_cast<const uint8_t *>(&kIdent.edge[3]) - base;
        case kUVA0Stride:
            return reinterpret_cast<const uint8_t *>(&kIdent.uva0) - base;
        case kUVA1Stride:
        case kUVA2Stride:
        case kUVA3Stride:
        case kUVA4Stride:
        case kBoneIndexStride:
        case kBoneWeightStride:
        case kTextureCoordStride:
        case kIndexStride:
        default:
            return 0;
        }
    }
    size_t strideSize() const {
        return sizeof(kIdent);
    }
    void update(const Vector3 &cameraPosition, void *address, Vector3 &aabbMin, Vector3 &aabbMax) {
        Unit *bufferPtr = static_cast<Unit *>(address);
        if (enableSkinning) {
            const Scalar &esf = modelRef->edgeScaleFactor(cameraPosition);
            const int nmaterials = materials.count();
            Vector3 position, normal;
            int offset = 0;
            for (int i = 0; i < nmaterials; i++) {
                const IMaterial *material = materials[i];
                const int nindices = material->indices(), offsetTo = offset + nindices;
                for (int j = offset; j < offsetTo; j++) {
                    const int index = indexBufferRef->indexAt(j);
                    const IVertex *vertex = vertices[index];
                    const float edgeSize = vertex->edgeSize() * esf;
                    Unit &v = bufferPtr[index];
                    vertex->performSkinning(position, normal);
                    v.position = position;
                    v.normal = normal;
                    v.delta = vertex->delta();
                    v.edge = position + normal * edgeSize;
                    v.edge[3] = Scalar(i);
                    aabbMin.setMin(position);
                    aabbMax.setMax(position);
                }
                offset += nindices;
            }
        }
        else {
            const int nvertices = vertices.count();
            for (int i = 0; i < nvertices; i++) {
                const IVertex *vertex = vertices[i];
                Unit &v = bufferPtr[i];
                v.delta = vertex->delta();
            }
            aabbMin.setZero();
            aabbMax.setZero();
        }
    }
    void setSkinningEnable(bool value) {
        enableSkinning = value;
    }
    const void *ident() const {
        return &kIdent;
    }

    const IModel *modelRef;
    const IModel::IIndexBuffer *indexBufferRef;
    Array<Unit> units;
    Array<IMaterial *> materials;
    Array<IVertex *> vertices;
    bool enableSkinning;
};
const DynamicVertexBuffer::Unit DynamicVertexBuffer::kIdent = DynamicVertexBuffer::Unit();

struct IndexBuffer : public IModel::IIndexBuffer {
    static const uint16_t kIdent = 0;

    IndexBuffer(const Array<int> &indices, const int nvertices)
        : nindices(indices.count())
    {
        indicesPtr.resize(nindices);
        for (int i = 0; i < nindices; i++) {
            int index = indices[i];
            if (index >= 0 && index < nvertices) {
                setIndexAt(i, index);
            }
            else {
                setIndexAt(i, 0);
            }
        }
#ifdef VPVL2_COORDINATE_OPENGL
        for (int i = 0; i < nindices; i += 3) {
            btSwap(indicesPtr[i], indicesPtr[i + 1]);
        }
#endif
    }
    ~IndexBuffer() {
    }

    const void *bytes() const {
        return &indicesPtr[0];
    }
    size_t size() const {
        return strideSize() * nindices;
    }
    size_t strideOffset(StrideType /* type */) const {
        return 0;
    }
    size_t strideSize() const {
        return sizeof(kIdent);
    }
    const void *ident() const {
        return &kIdent;
    }
    int indexAt(int index) const {
        return indicesPtr[index];
    }
    Type type() const {
        return kIndex16;
    }

    void setIndexAt(int i, uint16_t value) {
        indicesPtr[i] = value;
    }
    Array<uint16_t> indicesPtr;
    int nindices;
};

struct MatrixBuffer : public IModel::IMatrixBuffer {
    typedef btAlignedObjectArray<int> BoneIndices;
    typedef btAlignedObjectArray<BoneIndices> MeshBoneIndices;
    typedef btAlignedObjectArray<Transform> MeshLocalTransforms;
    typedef Array<float *> MeshMatrices;
    struct SkinningMeshes {
        MeshBoneIndices bones;
        MeshLocalTransforms transforms;
        MeshMatrices matrices;
        BoneIndices bdef2;
        ~SkinningMeshes() { matrices.releaseArrayAll(); }
    };

    MatrixBuffer(const IModel *model, const IndexBuffer *indexBuffer, DynamicVertexBuffer *dynamicBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          dynamicBufferRef(dynamicBuffer)
    {
        model->getBoneRefs(bones);
        model->getMaterialRefs(materials);
        model->getVertexRefs(vertices);
        initialize();
    }

    void update() {
        const int nbones = bones.count();
        MeshLocalTransforms &transforms = meshes.transforms;
        for (int i = 0; i < nbones; i++) {
            const IBone *bone = bones[i];
            transforms[i] = bone->localTransform();
        }
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++) {
            const BoneIndices &boneIndices = meshes.bones[i];
            const int nBoneIndices = boneIndices.size();
            Scalar *matrices = meshes.matrices[i];
            for (int j = 0; j < nBoneIndices; j++) {
                const int boneIndex = boneIndices[j];
                const Transform &transform = transforms[boneIndex];
                transform.getOpenGLMatrix(&matrices[j * 16]);
            }
        }
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            const IVertex *vertex = vertices[i];
            DynamicVertexBuffer::Unit &buffer = dynamicBufferRef->units[i];
            buffer.position = vertex->origin();
            buffer.delta = vertex->delta();
        }
    }
    const float *bytes(int materialIndex) const {
        int nmatrices = meshes.matrices.count();
        return internal::checkBound(materialIndex, 0, nmatrices) ? meshes.matrices[materialIndex] : 0;
    }
    size_t size(int materialIndex) const {
        int nbones = meshes.bones.size();
        return internal::checkBound(materialIndex, 0, nbones) ? meshes.bones[materialIndex].size() : 0;
    }

    void initialize() {
        const int nmaterials = materials.count();
        BoneIndices boneIndices;
        meshes.transforms.resize(bones.count());
        int offset = 0;
        for (int i = 0; i < nmaterials; i++) {
            const IMaterial *material = materials[i];
            const int nindices = material->indices();
            for (int j = 0; j < nindices; j++) {
                int vertexIndex = indexBufferRef->indexAt(offset + j);
                IVertex *vertex = vertices[vertexIndex];
                meshes.bdef2.push_back(vertexIndex);
                dynamicBufferRef->units[vertexIndex].position.setW(Scalar(vertex->type()));
            }
            meshes.matrices.add(new Scalar[boneIndices.size() * 16]);
            meshes.bones.push_back(boneIndices);
            boneIndices.clear();
            offset += nindices;
        }
    }

    const IModel *modelRef;
    const IndexBuffer *indexBufferRef;
    DynamicVertexBuffer *dynamicBufferRef;
    Array<IBone *> bones;
    Array<IMaterial *> materials;
    Array<IVertex *> vertices;
    SkinningMeshes meshes;
};

class BonePredication {
public:
    bool operator()(const pmd::Bone *left, const pmd::Bone *right) const {
        const IBone *parentLeft = left->parentBone(), *parentRight = right->parentBone();
        if (parentLeft && parentRight) {
            return parentLeft->index() < parentRight->index();
        }
        else if (!parentLeft && parentRight) {
            return true;
        }
        else if (parentLeft && !parentRight) {
            return false;
        }
        else {
            return left->index() < right->index();
        }
    }
};

}

namespace vpvl2
{
namespace pmd
{

const int Model::kNameSize;
const int Model::kCommentSize;
const int Model::kCustomToonTextureNameSize;
const int Model::kMaxCustomToonTextures;
const uint8_t *const Model::kFallbackToonTextureName = reinterpret_cast<const uint8_t *>("toon0.bmp");

Model::Model(IEncoding *encodingRef)
    : m_encodingRef(encodingRef),
      m_name(0),
      m_englishName(0),
      m_comment(0),
      m_englishComment(0),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(1),
      m_edgeColor(kZeroV3),
      m_edgeWidth(0),
      m_visible(false)
{
    m_edgeColor.setW(1);
}

Model::~Model()
{
    release();
}

bool Model::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    Header header;
    if (!data || sizeof(header) > rest) {
        m_info.error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    internal::getData(ptr, header);
    info.encoding = m_encodingRef;
    info.basePtr = ptr;

    // Check the signature and version is correct
    if (memcmp(header.signature, "Pmd", sizeof(header.signature)) != 0) {
        m_info.error = kInvalidSignatureError;
        return false;
    }
    if (header.version != 1.0) {
        m_info.error = kInvalidVersionError;
        return false;
    }

    // Name and Comment (in Shift-JIS)
    info.namePtr = header.name;
    info.commentPtr = header.comment;
    internal::readBytes(sizeof(header), ptr, rest);

    // Vertex
    if (!Vertex::preparse(ptr, rest, info)) {
        info.error = kInvalidVerticesError;
        return false;
    }
    // Index
    size_t nindices, indexSize = sizeof(uint16_t);
    if (!internal::size32(ptr, rest, nindices) || nindices * indexSize > rest) {
        m_info.error = kInvalidIndicesError;
        return false;
    }
    info.indicesPtr = ptr;
    info.indicesCount = nindices;
    internal::readBytes(nindices * indexSize, ptr, rest);
    // Material
    if (!Material::preparse(ptr, rest, info)) {
        info.error = kInvalidMaterialsError;
        return false;
    }
    // Bone
    if (!Bone::preparseBones(ptr, rest, info)) {
        info.error = kInvalidBonesError;
        return false;
    }
    // IK
    if (!Bone::preparseIKConstraints(ptr, rest, info)) {
        info.error = kInvalidBonesError;
        return false;
    }
    // Morph
    if (!Morph::preparse(ptr, rest, info)) {
        info.error = kInvalidMorphsError;
        return false;
    }
    // Label
    if (!Label::preparse(ptr, rest, info)) {
        info.error = kInvalidLabelsError;
        return false;
    }
    if (rest == 0)
        return true;

    // English info
    size_t hasEnglish;
    if (!internal::size8(ptr, rest, hasEnglish)) {
        info.error = kInvalidEnglishNameSizeError;
        return false;
    }
    if (hasEnglish != 0) {
        const size_t boneNameSize = Bone::kNameSize * info.bonesCount;
        const size_t morphNameSize =  Morph::kNameSize * info.morphLabelsCount;
        const size_t boneCategoryNameSize = Bone::kCategoryNameSize * info.boneCategoryNamesCount;
        const size_t required = kNameSize + kCommentSize + boneNameSize + morphNameSize + boneCategoryNameSize;
        if (required > rest) {
            m_info.error = kInvalidEnglishNameSizeError;
            return false;
        }
        info.englishNamePtr = ptr;
        internal::readBytes(kNameSize, ptr, rest);
        info.englishCommentPtr = ptr;
        internal::readBytes(kCommentSize, ptr, rest);
        info.englishBoneNamesPtr = ptr;
        internal::readBytes(boneNameSize, ptr, rest);
        info.englishFaceNamesPtr = ptr;
        internal::readBytes(morphNameSize, ptr, rest);
        info.englishBoneFramesPtr = ptr;
        internal::readBytes(boneCategoryNameSize, ptr, rest);
    }
    // Custom toon textures
    size_t customToonTextureNameSize = kMaxCustomToonTextures * kCustomToonTextureNameSize;
    if (customToonTextureNameSize > rest) {
        m_info.error = kInvalidTextureSizeError;
        return false;
    }
    info.customToonTextureNamesPtr = ptr;
    ptr += customToonTextureNameSize;
    rest -= customToonTextureNameSize;
    if (rest == 0)
        return true;

    // RigidBody
    if (!RigidBody::preparse(ptr, rest, info)) {
        info.error = kInvalidRigidBodiesError;
        return false;
    }
    // Joint
    if (!Joint::preparse(ptr, rest, info)) {
        info.error = kInvalidJointsError;
        return false;
    }

    return rest == 0;
}

bool Model::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        release();
        parseNamesAndComments(info);
        parseVertices(info);
        parseIndices(info);
        parseMaterials(info);
        parseBones(info);
        parseIKConstraints(info);
        parseMorphs(info);
        parseLabels(info);
        parseCustomToonTextures(info);
        parseRigidBodies(info);
        parseJoints(info);
        if (!Material::loadMaterials(m_materials, m_customToonTextures, m_indices.count())
                || !Vertex::loadVertices(m_vertices, m_bones)
                || !Morph::loadMorphs(m_morphs, m_vertices)
                || !Label::loadLabels(m_labels, m_bones, m_morphs)
                || !RigidBody::loadRigidBodies(m_rigidBodies, m_bones)
                || !Joint::loadJoints(m_joints, m_rigidBodies)) {
            m_info.error = info.error;
            return false;
        }
        m_info = info;
        return true;
    }
    else {
        m_info.error = info.error;
    }
    return false;
}

void Model::save(uint8_t * /* data */) const
{
}

IModel::ErrorType Model::error() const
{
    return m_info.error;
}

size_t Model::estimateSize() const
{
    size_t size = 0;
    size += sizeof(Header);
    size += Vertex::estimateTotalSize(m_vertices, m_info);
    size += m_indices.count() * sizeof(uint16_t);
    size += Material::estimateTotalSize(m_materials, m_info);
    size += Bone::estimateTotalSize(m_bones, m_info);
    size += Morph::estimateTotalSize(m_morphs, m_info);
    size += Label::estimateTotalSize(m_labels, m_info);
    size += kNameSize;
    size += kCommentSize;
    size += Bone::kNameSize * m_info.bonesCount;
    size += Morph::kNameSize * m_info.morphsCount;
    size += Bone::kCategoryNameSize * m_info.boneCategoryNamesCount;
    size += kCustomToonTextureNameSize * kMaxCustomToonTextures;
    size += RigidBody::estimateTotalSize(m_rigidBodies, m_info);
    size += Joint::estimateTotalSize(m_joints, m_info);
    return size;
}

void Model::resetVertices()
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertices[i];
        vertex->reset();
    }
}

void Model::resetMotionState()
{
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->setKinematic(false);
    }
}

void Model::performUpdate()
{
    const int nbones = m_sortedBones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_sortedBones[i];
        bone->performTransform();
    }
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_sortedBones[i];
        bone->solveInverseKinematics();
    }
    // physics simulation
    if (m_worldRef) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_rigidBodies[i];
            rigidBody->performTransformBone();
        }
    }
}

void Model::joinWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL2_NO_BULLET
    if (!world)
        return;
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        world->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->collisionGroupMask());
    }
    const int njoints = m_joints.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = m_joints[i];
        world->addConstraint(joint->constraint());
    }
    m_worldRef = world;
#endif /* VPVL2_NO_BULLET */
}

void Model::leaveWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL2_NO_BULLET
    if (!world)
        return;
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = nRigidBodies - 1; i >= 0; i--) {
        RigidBody *rigidBody = m_rigidBodies[i];
        world->removeCollisionObject(rigidBody->body());
    }
    const int njoints = m_joints.count();
    for (int i = njoints - 1; i >= 0; i--) {
        Joint *joint = m_joints[i];
        world->removeConstraint(joint->constraint());
    }
    m_worldRef = 0;
#endif /* VPVL2_NO_BULLET */
}

IBone *Model::findBone(const IString *value) const
{
    IBone **bone = const_cast<IBone **>(m_name2boneRefs.find(value->toHashString()));
    return bone ? *bone : 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    IMorph **morph = const_cast<IMorph **>(m_name2morphRefs.find(value->toHashString()));
    return morph ? *morph : 0;
}

int Model::count(ObjectType value) const
{
    switch (value) {
    case kBone:
        return m_bones.count();
    case kIK: {
        int nbones = 0, nIKJoints = 0;
        for (int i = 0; i < nbones; i++) {
            if (m_bones[i]->hasInverseKinematics())
                nIKJoints++;
        }
        return nIKJoints;
    }
    case kIndex:
        return m_indices.count();
    case kJoint:
        return m_joints.count();
    case kMaterial:
        return m_materials.count();
    case kMorph:
        return m_morphs.count();
    case kRigidBody:
        return m_rigidBodies.count();
    case kVertex:
        return m_vertices.count();
    case kMaxObjectType:
    default:
        return 0;
    }
    return 0;
}

void Model::getBoneRefs(Array<IBone *> &value) const
{
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = m_bones[i];
        value.add(bone);
    }
}

void Model::getLabelRefs(Array<ILabel *> &value) const
{
    const int nlabels = m_labels.count();
    for (int i = 0; i < nlabels; i++) {
        ILabel *label = m_labels[i];
        value.add(label);
    }
}

void Model::getMaterialRefs(Array<IMaterial *> &value) const
{
    const int nmaterials = m_materials.count();
    for (int i = 0; i < nmaterials; i++) {
        IMaterial *material = m_materials[i];
        value.add(material);
    }
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    const int nmorphs = m_morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = m_morphs[i];
        value.add(morph);
    }
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        IVertex *vertex = m_vertices[i];
        value.add(vertex);
    }
}

Scalar Model::edgeScaleFactor(const Vector3 &cameraPosition) const
{
    Scalar length = 0;
    if (m_bones.count() > 1) {
        IBone *bone = m_bones.at(1);
        length = (cameraPosition - bone->worldTransform().getOrigin()).length();
    }
    return (length / 1000.0f);
}

void Model::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Model::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_comment);
}

void Model::setEnglishComment(const IString *value)
{
    internal::setString(value, m_englishComment);
}

void Model::setPosition(const Vector3 &value)
{
    m_position = value;
}

void Model::setRotation(const Quaternion &value)
{
    m_rotation = value;
}

void Model::setOpacity(const Scalar &value)
{
    m_opacity = value;
}

void Model::setScaleFactor(const Scalar &value)
{
    m_scaleFactor = value;
}

void Model::setEdgeColor(const Vector3 &value)
{
    m_edgeColor = value;
}

void Model::setEdgeWidth(const Scalar &value)
{
    m_edgeWidth = value;
}

void Model::setVisible(bool value)
{
    m_visible = value;
}

void Model::getIndexBuffer(IIndexBuffer *&indexBuffer) const
{
    delete indexBuffer;
    indexBuffer = new IndexBuffer(m_indices, m_vertices.count());
}

void Model::getStaticVertexBuffer(IStaticVertexBuffer *&staticBuffer) const
{
    delete staticBuffer;
    staticBuffer = new StaticVertexBuffer(this);
}

void Model::getDynamicVertexBuffer(IDynamicVertexBuffer *&dynamicBuffer,
                                   const IIndexBuffer *indexBuffer) const
{
    delete dynamicBuffer;
    if (indexBuffer && indexBuffer->ident() == &IndexBuffer::kIdent) {
        dynamicBuffer = new DynamicVertexBuffer(this, indexBuffer);
    }
    else {
        dynamicBuffer = 0;
    }
}

void Model::getMatrixBuffer(IMatrixBuffer *&matrixBuffer,
                            IDynamicVertexBuffer *dynamicBuffer,
                            const IIndexBuffer *indexBuffer) const
{
    delete matrixBuffer;
    if (indexBuffer && indexBuffer->ident() == &IndexBuffer::kIdent &&
            dynamicBuffer && dynamicBuffer->ident() == &DynamicVertexBuffer::kIdent) {
        matrixBuffer = new MatrixBuffer(this,
                                        static_cast<const IndexBuffer *>(indexBuffer),
                                        static_cast<DynamicVertexBuffer *>(dynamicBuffer));
    }
    else {
        matrixBuffer = 0;
    }
}

void Model::release()
{
    leaveWorld(m_worldRef);
    internal::zerofill(&m_info, sizeof(m_info));
    m_vertices.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_labels.releaseAll();
    m_rigidBodies.releaseAll();
    m_joints.releaseAll();
    m_customToonTextures.releaseAll();
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    delete m_comment;
    m_comment = 0;
    delete m_englishComment;
    m_englishComment = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
    m_opacity = 1;
    m_scaleFactor = 1;
    m_edgeColor.setZero();
    m_edgeWidth = 0;
    m_visible = false;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    internal::setStringDirect(m_encodingRef->toString(info.namePtr, IString::kShiftJIS, kNameSize), m_name);
    internal::setStringDirect(m_encodingRef->toString(info.englishNamePtr, IString::kShiftJIS, kNameSize), m_englishName);
    internal::setStringDirect(m_encodingRef->toString(info.commentPtr, IString::kShiftJIS, kCommentSize), m_comment);
    internal::setStringDirect(m_encodingRef->toString(info.englishCommentPtr, IString::kShiftJIS, kCommentSize), m_englishComment);
}

void Model::parseVertices(const DataInfo &info)
{
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.verticesPtr;
    size_t size;
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        m_vertices.add(vertex);
        vertex->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseIndices(const DataInfo &info)
{
    const int nindices = info.indicesCount;
    uint8_t *ptr = info.indicesPtr;
    for (int i = 0; i < nindices; i++) {
        uint16_t index = internal::readUnsignedIndex(ptr, sizeof(uint16_t));
        m_indices.add(index);
    }
}

void Model::parseMaterials(const DataInfo &info)
{
    const int nmaterials = info.materialsCount;
    uint8_t *ptr = info.materialsPtr;
    size_t size;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = new Material(m_encodingRef);
        m_materials.add(material);
        material->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseBones(const DataInfo &info)
{
    const int nbones = info.bonesCount;
    uint8_t *ptr = info.bonesPtr;
    size_t size;
    for (int i = 0; i < nbones; i++) {
        Bone *bone = new Bone(m_encodingRef);
        m_bones.add(bone);
        m_sortedBones.add(bone);
        bone->readBone(ptr, info, size);
        m_name2boneRefs.insert(bone->name()->toHashString(), bone);
        ptr += size;
    }
    m_sortedBones.sort(BonePredication());
    Bone::loadBones(m_bones);
    performUpdate();
}

void Model::parseIKConstraints(const DataInfo &info)
{
    const int njoints = info.IKConstraintsCount;
    uint8_t *ptr = info.IKConstraintsPtr;
    size_t size;
    for (int i = 0; i < njoints; i++) {
        Bone::readIKConstraint(ptr, m_bones, size);
        ptr += size;
    }
}

void Model::parseMorphs(const DataInfo &info)
{
    const int nmorphs = info.morphsCount;
    uint8_t *ptr = info.morphsPtr;
    size_t size;
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = new Morph(m_encodingRef);
        m_morphs.add(morph);
        morph->read(ptr, size);
        m_name2morphRefs.insert(morph->name()->toHashString(), morph);
        ptr += size;
    }
}

void Model::parseLabels(const DataInfo &info)
{
}

void Model::parseCustomToonTextures(const DataInfo &info)
{
    static const uint8_t kFallbackToonTextureName[] = "toon0.bmp";
    uint8_t *ptr = info.customToonTextureNamesPtr;
    m_customToonTextures.add(m_encodingRef->toString(kFallbackToonTextureName,
                                                     sizeof(kFallbackToonTextureName),
                                                     IString::kUTF8));
    for (int i = 0; i < kMaxCustomToonTextures; i++) {
        IString *customToonTexture = m_encodingRef->toString(ptr, IString::kShiftJIS, kCustomToonTextureNameSize);
        m_customToonTextures.add(customToonTexture);
        ptr += kCustomToonTextureNameSize;
    }
}

void Model::parseRigidBodies(const DataInfo &info)
{
    const int nRigidBodies = info.rigidBodiesCount;
    uint8_t *ptr = info.rigidBodiesPtr;
    size_t size;
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody(m_encodingRef);
        m_rigidBodies.add(rigidBody);
        rigidBody->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseJoints(const DataInfo &info)
{
    const int njoints = info.jointsCount;
    uint8_t *ptr = info.jointsPtr;
    size_t size;
    for (int i = 0; i < njoints; i++) {
        Joint *joint = new Joint(m_encodingRef);
        m_joints.add(joint);
        joint->read(ptr, info, size);
        ptr += size;
    }
}

}
}
