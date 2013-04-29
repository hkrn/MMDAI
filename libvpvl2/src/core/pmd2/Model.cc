/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Joint.h"
#include "vpvl2/pmd2/Label.h"
#include "vpvl2/pmd2/Material.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/RigidBody.h"
#include "vpvl2/pmd2/Vertex.h"
#include "vpvl2/internal/ParallelVertexProcessor.h"

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct Header
{
    uint8_t signature[3];
    float version;
    uint8_t name[Model::kNameSize];
    uint8_t comment[Model::kCommentSize];
};

#pragma pack(pop)

struct StaticVertexBuffer : public IModel::IStaticVertexBuffer {
    struct Unit {
        Unit() {}
        void update(const IVertex *vertex) {
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

    StaticVertexBuffer(const Model *model)
        : modelRef(model)
    {
    }
    ~StaticVertexBuffer() {
        modelRef = 0;
    }

    size_t size() const {
        return strideSize() * modelRef->vertices().count();
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
    void update(void *address) const {
        Unit *unitPtr = static_cast<Unit *>(address);
        const PointerArray<Vertex> &vertices = modelRef->vertices();
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            unitPtr[i].update(vertices[i]);
        }
    }
    const void *ident() const {
        return &kIdent;
    }

    const Model *modelRef;
};
const StaticVertexBuffer::Unit StaticVertexBuffer::kIdent = StaticVertexBuffer::Unit();

struct DynamicVertexBuffer : public IModel::IDynamicVertexBuffer {
    struct Unit {
        Unit() {}
        void update(const IVertex *vertex, int index) {
            position = vertex->origin();
            normal = vertex->normal();
            normal[3] = vertex->edgeSize();
            edge[3] = Scalar(index);
            uva0.setValue(0, 0, 0, 1);
        }
        void update(const IVertex *vertex, float materialEdgeSize, int index, Vector3 &p) {
            Vector3 n;
            const float edgeSize = vertex->edgeSize() * materialEdgeSize;
            vertex->performSkinning(p, n);
            position = p;
            normal = n;
            normal[3] = vertex->edgeSize();
            edge = position + normal * edgeSize;
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

    typedef internal::ParallelSkinningVertexProcessor<pmd2::Model, pmd2::Vertex, Unit>
    ParallelSkinningVertexProcessor;
    typedef internal::ParallelInitializeVertexProcessor<pmd2::Model, pmd2::Vertex, Unit>
    ParallelInitializeVertexProcessor;

    DynamicVertexBuffer(const Model *model, const IModel::IIndexBuffer *indexBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          enableSkinning(true),
          enableParallelUpdate(false)
    {
    }
    ~DynamicVertexBuffer() {
        modelRef = 0;
        indexBufferRef = 0;
        enableSkinning = false;
        enableParallelUpdate = false;
    }

    size_t size() const {
        return strideSize() * modelRef->vertices().count();
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
    void update(void *address, const Vector3 &cameraPosition, Vector3 &aabbMin, Vector3 &aabbMax) const {
        const PointerArray<Vertex> &vertices = modelRef->vertices();
        Unit *bufferPtr = static_cast<Unit *>(address);
        if (enableSkinning) {
#if defined(VPVL2_LINK_INTEL_TBB) || defined(VPVL2_ENABLE_OPENMP)
            if (enableParallelUpdate) {
#if defined(VPVL2_LINK_INTEL_TBB)
                ParallelSkinningVertexProcessor proc(modelRef, &modelRef->vertices(), cameraPosition, bufferPtr);
                tbb::parallel_reduce(tbb::blocked_range<int>(0, vertices.count()), proc);
                aabbMin = proc.aabbMin();
                aabbMax = proc.aabbMax();
#elif defined(VPVL2_ENABLE_OPENMP)
                internal::UpdateModelVerticesOMP(modelRef, vertices, cameraPosition, bufferPtr);
#endif
            }
            else
#endif
            {
                const PointerArray<Material> &materials = modelRef->materials();
                const Scalar &esf = modelRef->edgeScaleFactor(cameraPosition);
                const int nmaterials = materials.count();
                Vector3 position;
                int offset = 0;
                for (int i = 0; i < nmaterials; i++) {
                    const IMaterial *material = materials[i];
                    const IMaterial::IndexRange &indexRange = material->indexRange();
                    const int nindices = indexRange.count, offsetTo = offset + nindices;
                    for (int j = offset; j < offsetTo; j++) {
                        const int index = indexBufferRef->indexAt(j);
                        const IVertex *vertex = vertices[index];
                        const float edgeSize = vertex->edgeSize() * esf;
                        Unit &v = bufferPtr[index];
                        v.update(vertex, edgeSize, i, position);
                        aabbMin.setMin(position);
                        aabbMax.setMax(position);
                    }
                    offset += nindices;
                }
            }
        }
        else {
            const int nvertices = vertices.count();
            for (int i = 0; i < nvertices; i++) {
                const IVertex *vertex = vertices[i];
                Unit &v = bufferPtr[i];
                v.update(vertex, i);
            }
            aabbMin.setZero();
            aabbMax.setZero();
        }
    }
    void setSkinningEnable(bool value) {
        enableSkinning = value;
    }
    void setParallelUpdateEnable(bool value) {
        enableParallelUpdate = value;
    }
    const void *ident() const {
        return &kIdent;
    }

    const Model *modelRef;
    const IModel::IIndexBuffer *indexBufferRef;
    bool enableSkinning;
    bool enableParallelUpdate;
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
        nindices = 0;
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
const uint16_t IndexBuffer::kIdent;

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
    ~MatrixBuffer() {
        modelRef = 0;
        indexBufferRef = 0;
        dynamicBufferRef = 0;
    }

    void update(void *address) {
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
        DynamicVertexBuffer::Unit *units = static_cast<DynamicVertexBuffer::Unit *>(address);
        for (int i = 0; i < nvertices; i++) {
            const IVertex *vertex = vertices[i];
            DynamicVertexBuffer::Unit &buffer = units[i];
            buffer.position = vertex->origin();
            buffer.position.setW(vertex->type());
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
            const int nindices = material->indexRange().count;
            for (int j = 0; j < nindices; j++) {
                int vertexIndex = indexBufferRef->indexAt(offset + j);
                meshes.bdef2.push_back(vertexIndex);
            }
            meshes.matrices.append(new Scalar[boneIndices.size() * 16]);
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
    bool operator()(const Bone *left, const Bone *right) const {
        const IBone *parentLeft = left->parentBoneRef(), *parentRight = right->parentBoneRef();
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

static inline bool VPVL2PMDGetBonePosition(const IModel *modelRef,
                                           const IEncoding *encodingRef,
                                           IEncoding::ConstantType value,
                                           Vector3 &position)
{
    if (const IBone *bone = modelRef->findBone(encodingRef->stringConstant(value))) {
        position = bone->localTransform().getOrigin();
        return !position.fuzzyZero();
    }
    return false;
}

}

namespace vpvl2
{
namespace pmd2
{

const int Model::kNameSize;
const int Model::kCommentSize;
const int Model::kCustomToonTextureNameSize;
const int Model::kMaxCustomToonTextures;
const uint8_t *const Model::kFallbackToonTextureName = reinterpret_cast<const uint8_t *>("toon0.bmp");

Model::Model(IEncoding *encodingRef)
    : m_sceneRef(0),
      m_encodingRef(encodingRef),
      m_namePtr(0),
      m_englishNamePtr(0),
      m_commentPtr(0),
      m_englishCommentPtr(0),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(1),
      m_edgeColor(kZeroV3),
      m_aabbMax(kZeroV3),
      m_aabbMin(kZeroV3),
      m_edgeWidth(0),
      m_hasEnglish(false),
      m_visible(false),
      m_physicsEnabled(false)
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
    internal::drainBytes(sizeof(header), ptr, rest);
    // Vertex
    if (!Vertex::preparse(ptr, rest, info)) {
        info.error = kInvalidVerticesError;
        return false;
    }
    // Index
    int nindices;
    size_t indexSize = sizeof(uint16_t);
    if (!internal::getTyped<int>(ptr, rest, nindices) || nindices * indexSize > rest) {
        m_info.error = kInvalidIndicesError;
        return false;
    }
    info.indicesPtr = ptr;
    info.indicesCount = nindices;
    internal::drainBytes(nindices * indexSize, ptr, rest);
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
    if (rest == 0) {
        return true;
    }
    // English info
    uint8_t hasEnglish;
    if (!internal::getTyped<uint8_t>(ptr, rest, hasEnglish)) {
        info.error = kInvalidEnglishNameSizeError;
        return false;
    }
    m_hasEnglish = hasEnglish != 0;
    if (m_hasEnglish) {
        const size_t boneNameSize = Bone::kNameSize * info.bonesCount;
        const size_t morphNameSize =  Morph::kNameSize * info.morphLabelsCount;
        const size_t boneCategoryNameSize = Bone::kCategoryNameSize * info.boneCategoryNamesCount;
        const size_t required = kNameSize + kCommentSize + boneNameSize + morphNameSize + boneCategoryNameSize;
        if (required > rest) {
            m_info.error = kInvalidEnglishNameSizeError;
            return false;
        }
        info.englishNamePtr = ptr;
        internal::drainBytes(kNameSize, ptr, rest);
        info.englishCommentPtr = ptr;
        internal::drainBytes(kCommentSize, ptr, rest);
        info.englishBoneNamesPtr = ptr;
        internal::drainBytes(boneNameSize, ptr, rest);
        info.englishFaceNamesPtr = ptr;
        internal::drainBytes(morphNameSize, ptr, rest);
        info.englishBoneFramesPtr = ptr;
        internal::drainBytes(boneCategoryNameSize, ptr, rest);
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
    if (rest == 0) {
        return true;
    }
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

void Model::save(uint8_t *data, size_t &written) const
{
    Header header;
    header.version = 1.0;
    uint8_t *base = data;
    internal::copyBytes(header.signature, "Pmd", sizeof(header.signature));
    uint8_t *namePtr = header.name, *commentPtr = header.comment;
    internal::writeStringAsByteArray(m_namePtr, IString::kShiftJIS, m_encodingRef, sizeof(header.name), namePtr);
    internal::writeStringAsByteArray(m_commentPtr, IString::kShiftJIS, m_encodingRef, sizeof(header.comment), commentPtr);
    internal::writeBytes(&header, sizeof(header), data);
    Vertex::writeVertices(m_vertices, m_info, data);
    const int nindices = m_indices.count();
    internal::writeBytes(&nindices, sizeof(nindices), data);
    for (int i = 0; i < nindices; i++) {
        int index = m_indices[i];
        internal::writeUnsignedIndex(index, sizeof(uint16_t), data);
    }
    Material::writeMaterials(m_materials, m_info, data);
    Bone::writeBones(m_bones, m_info, data);
    const int nconstraints = m_constraints.count();
    internal::writeUnsignedIndex(nconstraints, sizeof(uint16_t), data);
    for (int i = 0; i < nconstraints; i++) {
        const IKConstraint *constraint = m_constraints[i];
        internal::writeBytes(&constraint->unit, sizeof(constraint->unit), data);
        const Array<Bone *> &effectors = constraint->effectors;
        const int nbonesInIK = effectors.count();
        for (int j = 0; j < nbonesInIK; j++) {
            const Bone *b = effectors[j];
            internal::writeSignedIndex(b->index(), sizeof(uint16_t), data);
        }
    }
    Morph::writeMorphs(m_morphs, m_info, data);
    Label::writeLabels(m_labels, m_info, data);
    internal::writeSignedIndex(m_hasEnglish ? 1 : 0, sizeof(uint8_t), data);
    if (m_hasEnglish) {
        internal::writeStringAsByteArray(m_englishNamePtr, IString::kShiftJIS, m_encodingRef, kNameSize, data);
        internal::writeStringAsByteArray(m_englishCommentPtr, IString::kShiftJIS, m_encodingRef, kCommentSize, data);
        Bone::writeEnglishNames(m_bones, m_info, data);
        Morph::writeEnglishNames(m_morphs, m_info, data);
        Label::writeEnglishNames(m_labels, m_info, data);
    }
    uint8_t customTextureName[kCustomToonTextureNameSize], *customTextureNamePtr = customTextureName;
    for (int i = 0; i < kMaxCustomToonTextures; i++) {
        const IString *customToonTextureRef = m_customToonTextures[i];
        customTextureNamePtr = customTextureName;
        internal::zerofill(customTextureNamePtr, sizeof(customTextureName));
        internal::writeStringAsByteArray(customToonTextureRef, IString::kShiftJIS, m_encodingRef, sizeof(customTextureName), customTextureNamePtr);
        internal::writeBytes(customTextureName, sizeof(customTextureName), data);
    }
    RigidBody::writeRigidBodies(m_rigidBodies, m_info, data);
    Joint::writeJoints(m_joints, m_info, data);
    written = data - base;
    VPVL2_LOG(VLOG(1) << "PMDEOF: base=" << reinterpret_cast<const void *>(base) << " data=" << reinterpret_cast<const void *>(data) << " written=" << written);
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
    size += sizeof(int) + m_indices.count() * sizeof(uint16_t);
    size += Material::estimateTotalSize(m_materials, m_info);
    size += Bone::estimateTotalSize(m_bones, m_info);
    const uint16_t nconstraints = m_constraints.count();
    size += sizeof(nconstraints);
    for (int i = 0; i < nconstraints; i++) {
        const IKConstraint *constraint = m_constraints[i];
        size += sizeof(constraint->unit);
        size += sizeof(uint16_t) * constraint->effectors.count();
    }
    size += Morph::estimateTotalSize(m_morphs, m_info);
    size += Label::estimateTotalSize(m_labels, m_info);
    size += sizeof(uint8_t);
    if (m_hasEnglish) {
        size += kNameSize;
        size += kCommentSize;
        size += Bone::kNameSize * m_info.bonesCount;
        size += Morph::kNameSize * m_info.morphsCount;
        size += Bone::kCategoryNameSize * m_info.boneCategoryNamesCount;
    }
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

void Model::resetMotionState(btDiscreteDynamicsWorld *worldRef)
{
    btOverlappingPairCache *cache = worldRef->getPairCache();
    btDispatcher *dispatcher = worldRef->getDispatcher();
    const int nRigidBodies = m_rigidBodies.count();
    Vector3 basePosition(kZeroV3);
    /* get offset position of the model by the bone of root or center for RigidBody#setKinematic */
    if (!VPVL2PMDGetBonePosition(this, m_encodingRef, IEncoding::kRootBone, basePosition)) {
        VPVL2PMDGetBonePosition(this, m_encodingRef, IEncoding::kCenter, basePosition);
    }
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        if (cache) {
            btRigidBody *body = rigidBody->body();
            cache->cleanProxyFromPairs(body->getBroadphaseHandle(), dispatcher);
        }
        rigidBody->setKinematic(false, basePosition);
    }
    const int njoints = m_joints.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = m_joints[i];
        joint->updateTransform();
    }
}

void Model::performUpdate()
{
    const int nbones = m_sortedBoneRefs.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_sortedBoneRefs[i];
        bone->performTransform();
    }
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_sortedBoneRefs[i];
        bone->solveInverseKinematics();
    }
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->performTransformBone();
    }
}

void Model::joinWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_rigidBodies[i];
            worldRef->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->collisionGroupMask());
        }
        const int njoints = m_joints.count();
        for (int i = 0; i < njoints; i++) {
            Joint *joint = m_joints[i];
            worldRef->addConstraint(joint->constraint());
        }
    }
}

void Model::leaveWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = nRigidBodies - 1; i >= 0; i--) {
            RigidBody *rigidBody = m_rigidBodies[i];
            worldRef->removeCollisionObject(rigidBody->body());
        }
        const int njoints = m_joints.count();
        for (int i = njoints - 1; i >= 0; i--) {
            Joint *joint = m_joints[i];
            worldRef->removeConstraint(joint->constraint());
        }
    }
}

IBone *Model::findBone(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IBone **bone = const_cast<IBone **>(m_name2boneRefs.find(key));
        return bone ? *bone : 0;
    }
    return 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IMorph **morph = const_cast<IMorph **>(m_name2morphRefs.find(key));
        return morph ? *morph : 0;
    }
    return 0;
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
        value.append(bone);
    }
}

void Model::getLabelRefs(Array<ILabel *> &value) const
{
    const int nlabels = m_labels.count();
    for (int i = 0; i < nlabels; i++) {
        ILabel *label = m_labels[i];
        value.append(label);
    }
}

void Model::getMaterialRefs(Array<IMaterial *> &value) const
{
    const int nmaterials = m_materials.count();
    for (int i = 0; i < nmaterials; i++) {
        IMaterial *material = m_materials[i];
        value.append(material);
    }
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    const int nmorphs = m_morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = m_morphs[i];
        value.append(morph);
    }
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        IVertex *vertex = m_vertices[i];
        value.append(vertex);
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
    internal::setString(value, m_namePtr);
}

void Model::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishNamePtr);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_commentPtr);
}

void Model::setEnglishComment(const IString *value)
{
    internal::setString(value, m_englishCommentPtr);
}

void Model::setWorldPosition(const Vector3 &value)
{
    m_position = value;
}

void Model::setWorldRotation(const Quaternion &value)
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

void Model::setParentSceneRef(Scene *value)
{
    m_sceneRef = value;
}

void Model::setVisible(bool value)
{
    m_visible = value;
}

void Model::getAabb(Vector3 &min, Vector3 &max) const
{
    min = m_aabbMin;
    max = m_aabbMax;
}

void Model::setAabb(const Vector3 &min, const Vector3 &max)
{
    m_aabbMin = min;
    m_aabbMax = max;
}


IBone *Model::createBone()
{
    return new Bone(this, m_encodingRef);
}

ILabel *Model::createLabel()
{
    return new Label(this, m_encodingRef, reinterpret_cast<const uint8_t *>(""), Label::kBoneCategoryLabel);
}

IMaterial *Model::createMaterial()
{
    return new Material(this, m_encodingRef);
}

IMorph *Model::createMorph()
{
    return new Morph(this, m_encodingRef);
}

IVertex *Model::createVertex()
{
    return new Vertex(this);
}

IBone *Model::findBoneAt(int value) const
{
    return internal::checkBound(value, 0, m_bones.count()) ? m_bones[value] : 0;
}

ILabel *Model::findLabelAt(int value) const
{
    return internal::checkBound(value, 0, m_labels.count()) ? m_labels[value] : 0;
}

IMaterial *Model::findMaterialAt(int value) const
{
    return internal::checkBound(value, 0, m_materials.count()) ? m_materials[value] : 0;
}

IMorph *Model::findMorphAt(int value) const
{
    return internal::checkBound(value, 0, m_morphs.count()) ? m_morphs[value] : 0;
}

IVertex *Model::findVertexAt(int value) const
{
    return internal::checkBound(value, 0, m_vertices.count()) ? m_vertices[value] : 0;
}

void Model::addBone(IBone *value)
{
    if (value->parentModelRef() == this) {
        Bone *bone = static_cast<Bone *>(value);
        m_bones.append(bone);
    }
}

void Model::addLabel(ILabel *value)
{
    if (value->parentModelRef() == this) {
        Label *label = static_cast<Label *>(value);
        m_labels.append(label);
    }
}

void Model::addMaterial(IMaterial *value)
{
    if (value->parentModelRef() == this) {
        Material *material = static_cast<Material *>(value);
        m_materials.append(material);
    }
}

void Model::addMorph(IMorph *value)
{
    if (value->parentModelRef() == this) {
        Morph *morph = static_cast<Morph *>(value);
        m_morphs.append(morph);
    }
}

void Model::addVertex(IVertex *value)
{
    if (value->parentModelRef() == this) {
        Vertex *vertex = static_cast<Vertex *>(value);
        m_vertices.append(vertex);
    }
}

void Model::removeBone(IBone *value)
{
    if (value->parentModelRef() == this) {
        Bone *bone = static_cast<Bone *>(value);
        m_bones.remove(bone);
    }
}

void Model::removeLabel(ILabel *value)
{
    if (value->parentModelRef() == this) {
        Label *label = static_cast<Label *>(value);
        m_labels.remove(label);
    }
}

void Model::removeMaterial(IMaterial *value)
{
    if (value->parentModelRef() == this) {
        Material *material = static_cast<Material *>(value);
        m_materials.remove(material);
    }
}

void Model::removeMorph(IMorph *value)
{
    if (value->parentModelRef() == this) {
        Morph *morph = static_cast<Morph *>(value);
        m_morphs.remove(morph);
    }
}

void Model::removeVertex(IVertex *value)
{
    if (value->parentModelRef() == this) {
        Vertex *vertex = static_cast<Vertex *>(value);
        m_vertices.remove(vertex);
    }
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
    internal::zerofill(&m_info, sizeof(m_info));
    m_vertices.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_constraints.releaseAll();
    m_morphs.releaseAll();
    m_labels.releaseAll();
    m_rigidBodies.releaseAll();
    m_joints.releaseAll();
    m_customToonTextures.releaseAll();
    delete m_namePtr;
    m_namePtr = 0;
    delete m_englishNamePtr;
    m_englishNamePtr = 0;
    delete m_commentPtr;
    m_commentPtr = 0;
    delete m_englishCommentPtr;
    m_englishCommentPtr = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
    m_opacity = 1;
    m_scaleFactor = 1;
    m_edgeColor.setZero();
    m_aabbMax.setZero();
    m_aabbMin.setZero();
    m_edgeWidth = 0;
    m_hasEnglish = false;
    m_visible = false;
    m_physicsEnabled = false;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    internal::setStringDirect(m_encodingRef->toString(info.namePtr, IString::kShiftJIS, kNameSize), m_namePtr);
    internal::setStringDirect(m_encodingRef->toString(info.englishNamePtr, IString::kShiftJIS, kNameSize), m_englishNamePtr);
    internal::setStringDirect(m_encodingRef->toString(info.commentPtr, IString::kShiftJIS, kCommentSize), m_commentPtr);
    internal::setStringDirect(m_encodingRef->toString(info.englishCommentPtr, IString::kShiftJIS, kCommentSize), m_englishCommentPtr);
}

void Model::parseVertices(const DataInfo &info)
{
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.verticesPtr;
    size_t size;
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertices.append(new Vertex(this));
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
        m_indices.append(index);
    }
}

void Model::parseMaterials(const DataInfo &info)
{
    const int nmaterials = info.materialsCount, nindices = m_indices.count();
    uint8_t *ptr = info.materialsPtr;
    size_t size;
    int indexOffset = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = m_materials.append(new Material(this, m_encodingRef));
        material->read(ptr, info, size);
        IMaterial::IndexRange range = material->indexRange();
        int indexOffsetTo = indexOffset + range.count;
        range.start = nindices;
        range.end = 0;
        for (int j = indexOffset; j < indexOffsetTo; j++) {
            const int index = m_indices[j];
            IVertex *vertex = m_vertices[index];
            vertex->setMaterial(material);
            btSetMin(range.start, index);
            btSetMax(range.end, index);
        }
        material->setIndexRange(range);
        indexOffset = indexOffsetTo;
        ptr += size;
    }
}

void Model::parseBones(const DataInfo &info)
{
    const int nbones = info.bonesCount;
    const uint8_t *englishPtr = info.englishBoneNamesPtr;
    uint8_t *ptr = info.bonesPtr;
    size_t size;
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones.append(new Bone(this, m_encodingRef));
        bone->readBone(ptr, info, size);
        m_sortedBoneRefs.append(bone);
        m_name2boneRefs.insert(bone->name()->toHashString(), bone);
        if (m_hasEnglish) {
            bone->readEnglishName(englishPtr, i);
            m_name2boneRefs.insert(bone->englishName()->toHashString(), bone);
        }
        ptr += size;
    }
    Bone::loadBones(m_bones);
    m_sortedBoneRefs.sort(BonePredication());
    performUpdate();
}

void Model::parseIKConstraints(const DataInfo &info)
{
    const int njoints = info.IKConstraintsCount;
    uint8_t *ptr = info.IKConstraintsPtr;
    size_t size;
    for (int i = 0; i < njoints; i++) {
        IKConstraint *constraint = m_constraints.append(new IKConstraint());
        Bone::readIKConstraint(ptr, m_bones, constraint, size);
        ptr += size;
    }
}

void Model::parseMorphs(const DataInfo &info)
{
    const int nmorphs = info.morphsCount;
    const uint8_t *englishPtr = info.englishFaceNamesPtr;
    uint8_t *ptr = info.morphsPtr;
    size_t size;
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = m_morphs.append(new Morph(this, m_encodingRef));
        morph->read(ptr, size);
        m_name2morphRefs.insert(morph->name()->toHashString(), morph);
        if (m_hasEnglish) {
            morph->readEnglishName(englishPtr, i);
            m_name2morphRefs.insert(morph->englishName()->toHashString(), morph);
        }
        ptr += size;
    }
}

void Model::parseLabels(const DataInfo &info)
{
    int ncategories = info.boneCategoryNamesCount;
    uint8_t *boneCategoryNamesPtr = info.boneCategoryNamesPtr;
    size_t size = 0;
    for (int i = 0; i < ncategories; i++) {
        Label::Type type = i == 0 ? Label::kSpecialBoneCategoryLabel : Label::kBoneCategoryLabel;
        Label *label = m_labels.append(new Label(this, m_encodingRef, boneCategoryNamesPtr, type));
        label->readEnglishName(info.englishBoneFramesPtr, i);
        boneCategoryNamesPtr += size;
    }
    int nbones = info.boneLabelsCount;
    uint8_t *boneLabelsPtr = info.boneLabelsPtr;
    for (int i = 0; i < nbones; i++) {
        if (Label *label = Label::selectCategory(m_labels, boneLabelsPtr)) {
            label->read(boneLabelsPtr, info, size);
            boneLabelsPtr += size;
        }
    }
    int nmorphs = info.morphLabelsCount;
    uint8_t *morphLabelsPtr = info.morphLabelsPtr;
    Label *morphCategory = m_labels.append(new Label(this, m_encodingRef, 0, Label::kMorphCategoryLabel));
    for (int i = 0; i < nmorphs; i++) {
        morphCategory->read(morphLabelsPtr, info, size);
        morphLabelsPtr += size;
    }
}

void Model::parseCustomToonTextures(const DataInfo &info)
{
    static const uint8_t kFallbackToonTextureName[] = "toon0.bmp";
    uint8_t *ptr = info.customToonTextureNamesPtr;
    m_customToonTextures.append(m_encodingRef->toString(kFallbackToonTextureName,
                                                        sizeof(kFallbackToonTextureName) - 1,
                                                        IString::kUTF8));
    for (int i = 0; i < kMaxCustomToonTextures; i++) {
        IString *customToonTexture = m_encodingRef->toString(ptr, IString::kShiftJIS, kCustomToonTextureNameSize);
        m_customToonTextures.append(customToonTexture);
        ptr += kCustomToonTextureNameSize;
    }
}

void Model::parseRigidBodies(const DataInfo &info)
{
    const int nRigidBodies = info.rigidBodiesCount;
    uint8_t *ptr = info.rigidBodiesPtr;
    size_t size;
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies.append(new RigidBody(m_encodingRef));
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
        Joint *joint = m_joints.append(new Joint(m_encodingRef));
        joint->read(ptr, info, size);
        ptr += size;
    }
}

}
}
