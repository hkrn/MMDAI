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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"
#include "vpvl2/internal/ParallelVertexProcessor.h"

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

namespace {

using namespace vpvl2;

#pragma pack(push, 1)

struct Header
{
    uint8_t signature[4];
    float version;
};

struct Flags
{
    uint8_t codec;
    uint8_t additionalUVSize;
    uint8_t vertexIndexSize;
    uint8_t textureIndexSize;
    uint8_t materialIndexSize;
    uint8_t boneIndexSize;
    uint8_t morphIndexSize;
    uint8_t rigidBodyIndexSize;
    void copy(pmx::Model::DataInfo &info) {
        info.codec = codec == 1 ? IString::kUTF8 : IString::kUTF16;
        info.additionalUVSize = additionalUVSize;
        info.vertexIndexSize = vertexIndexSize;
        info.textureIndexSize = textureIndexSize;
        info.materialIndexSize = materialIndexSize;
        info.boneIndexSize = boneIndexSize;
        info.morphIndexSize = morphIndexSize;
        info.rigidBodyIndexSize = rigidBodyIndexSize;
    }
    static int estimateSize(int value) {
        if (value < 128) {
            return 1;
        }
        else if (value < 32768) {
            return 2;
        }
        else {
            return 4;
        }
    }
};

#pragma pack(pop)

struct StaticVertexBuffer : public IModel::IStaticVertexBuffer {
    struct Unit {
        Unit() {}
        void update(const IVertex *vertex) {
            IBone *bone1 = vertex->bone(0),
                    *bone2 = vertex->bone(1),
                    *bone3 = vertex->bone(2),
                    *bone4 = vertex->bone(3);
            boneIndices.setValue(Scalar(bone1 ? bone1->index() : -1),
                                 Scalar(bone2 ? bone2->index() : -1),
                                 Scalar(bone3 ? bone3->index() : -1),
                                 Scalar(bone4 ? bone4->index() : -1));
            boneWeights.setValue(vertex->weight(0),
                                 vertex->weight(1),
                                 vertex->weight(2),
                                 vertex->weight(3));
            texcoord = vertex->textureCoord();
        }
        Vector3 texcoord;
        Vector4 boneIndices;
        Vector4 boneWeights;
    };
    static const Unit kIdent;

    StaticVertexBuffer(const pmx::Model *model)
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
        return sizeof(kIdent);
    }
    void update(void *address) const {
        Unit *unitPtr = static_cast<Unit *>(address);
        const Array<pmx::Vertex *> &vertices = modelRef->vertices();
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            unitPtr[i].update(vertices[i]);
        }
    }
    const void *ident() const {
        return &kIdent;
    }

    const pmx::Model *modelRef;
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
            updateMorph(vertex);
        }
        void update(const IVertex *vertex, float materialEdgeSize, int index, Vector3 &p) {
            Vector3 n;
            const float edgeSize = vertex->edgeSize() * materialEdgeSize;
            vertex->performSkinning(p, n);
            position = p;
            normal = n;
            normal[3] = edgeSize;
            edge = position + normal * edgeSize;
            edge[3] = Scalar(index);
            updateMorph(vertex);
        }
        void updateMorph(const IVertex *vertex) {
            delta = vertex->delta();
            uva0 = vertex->uv(0);
            uva1 = vertex->uv(1);
            uva2 = vertex->uv(2);
            uva3 = vertex->uv(3);
            uva4 = vertex->uv(4);
        }
        Vector3 position;
        Vector3 normal;
        Vector3 delta;
        Vector3 edge;
        Vector4 uva0;
        Vector4 uva1;
        Vector4 uva2;
        Vector4 uva3;
        Vector4 uva4;
    };
    static const Unit kIdent;

    typedef internal::ParallelSkinningVertexProcessor<pmx::Model, pmx::Vertex, Unit>
    ParallelSkinningVertexProcessor;
    typedef internal::ParallelInitializeVertexProcessor<pmx::Model, pmx::Vertex, Unit>
    ParallelInitializeVertexProcessor;

    DynamicVertexBuffer(const pmx::Model *model, const IModel::IIndexBuffer *indexBuffer)
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
            return reinterpret_cast<const uint8_t *>(&kIdent.uva1) - base;
        case kUVA2Stride:
            return reinterpret_cast<const uint8_t *>(&kIdent.uva2) - base;
        case kUVA3Stride:
            return reinterpret_cast<const uint8_t *>(&kIdent.uva3) - base;
        case kUVA4Stride:
            return reinterpret_cast<const uint8_t *>(&kIdent.uva4) - base;
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
    const void *ident() const {
        return &kIdent;
    }
    void update(void *address, const Vector3 &cameraPosition, Vector3 &aabbMin, Vector3 &aabbMax) const {
        const Array<pmx::Vertex *> &vertices = modelRef->vertices();
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
                const Array<pmx::Material *> &materials = modelRef->materials();
                const Scalar &esf = modelRef->edgeScaleFactor(cameraPosition);
                const int nmaterials = materials.count();
                Vector3 position;
                int offset = 0;
                for (int i = 0; i < nmaterials; i++) {
                    const IMaterial *material = materials[i];
                    const int nindices = material->indexRange().count, offsetTo = offset + nindices;
                    const float materialEdgeSize = material->edgeSize() * esf;
                    for (int j = offset; j < offsetTo; j++) {
                        const int index = indexBufferRef->indexAt(j);
                        const IVertex *vertex = vertices[index];
                        Unit &v = bufferPtr[index];
                        v.update(vertex, materialEdgeSize, i, position);
                        aabbMin.setMin(position);
                        aabbMax.setMax(position);
                    }
                    offset += nindices;
                }
            }
        }
        else {
#if defined(VPVL2_LINK_INTEL_TBB) || defined(VPVL2_ENABLE_OPENMP)
            if (enableParallelUpdate) {
#if defined(VPVL2_LINK_INTEL_TBB)
                static tbb::affinity_partitioner affinityPartitioner;
                tbb::parallel_for(tbb::blocked_range<int>(0, vertices.count()),
                                  ParallelInitializeVertexProcessor(&modelRef->vertices(), address),
                                  affinityPartitioner);
#elif defined(VPVL2_ENABLE_OPENMP)
                internal::InitializeModelVerticesOMP(vertices, bufferPtr);
#endif
            }
            else
#endif
            {
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
    }
    void setSkinningEnable(bool value) {
        enableSkinning = value;
    }
    void setParallelUpdateEnable(bool value) {
        enableParallelUpdate = value;
    }

    const pmx::Model *modelRef;
    const IModel::IIndexBuffer *indexBufferRef;
    bool enableSkinning;
    bool enableParallelUpdate;
};
const DynamicVertexBuffer::Unit DynamicVertexBuffer::kIdent = DynamicVertexBuffer::Unit();

struct IndexBuffer : public IModel::IIndexBuffer {
    static const int kIdent = 0;
    IndexBuffer(const Array<int> &indices, const int nvertices)
        : indexType(kIndex32),
          indices32Ptr(0),
          nindices(indices.count())
    {
        if (nindices < 65536) {
            indexType = kIndex16;
            indices16Ptr = new uint16_t[nindices];
        }
        else if (nindices < 256) {
            indexType = kIndex8;
            indices8Ptr = new uint8_t[nindices];
        }
        else {
            indices32Ptr = new int[nindices];
        }
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
            switch (indexType) {
            case kIndex32:
                btSwap(indices32Ptr[i], indices32Ptr[i + 1]);
                break;
            case kIndex16:
                btSwap(indices16Ptr[i], indices16Ptr[i + 1]);
                break;
            case kIndex8:
                btSwap(indices8Ptr[i], indices8Ptr[i + 1]);
                break;
            case kMaxIndexType:
            default:
                break;
            }
        }
#endif /* VPVL2_COORDINATE_OPENGL */
    }
    ~IndexBuffer() {
        switch (indexType) {
        case kIndex32:
            delete[] indices32Ptr;
            indices32Ptr = 0;
            break;
        case kIndex16:
            delete[] indices16Ptr;
            indices16Ptr = 0;
            break;
        case kIndex8:
            delete[] indices8Ptr;
            indices8Ptr = 0;
            break;
        case kMaxIndexType:
        default:
            break;
        }
    }

    const void *bytes() const {
        switch (indexType) {
        case kIndex32:
            return indices32Ptr;
        case kIndex16:
            return indices16Ptr;
        case kIndex8:
            return indices8Ptr;
        case kMaxIndexType:
        default:
            return 0;
        }
    }
    size_t size() const {
        return strideSize() * nindices;
    }
    size_t strideOffset(StrideType /* type */) const {
        return 0;
    }
    size_t strideSize() const {
        switch (indexType) {
        case kIndex32:
            return sizeof(int);
        case kIndex16:
            return sizeof(uint16_t);
        case kIndex8:
            return sizeof(uint8_t);
        case kMaxIndexType:
        default:
            return 0;
        }
    }
    const void *ident() const {
        return &kIdent;
    }
    int indexAt(int index) const {
        switch (indexType) {
        case kIndex32:
            return indices32Ptr[index];
        case kIndex16:
            return indices16Ptr[index];
        case kIndex8:
            return indices8Ptr[index];
        case kMaxIndexType:
        default:
            return 0;
        }
    }
    Type type() const {
        return indexType;
    }

    void setIndexAt(int i, int value) {
        switch (indexType) {
        case kIndex32:
            indices32Ptr[i] = value;
            break;
        case kIndex16:
            indices16Ptr[i] = uint16_t(value);
            break;
        case kIndex8:
            indices8Ptr[i] = uint8_t(value);
            break;
        case kMaxIndexType:
        default:
            break;
        }
    }
    IModel::IIndexBuffer::Type indexType;
    union {
        int      *indices32Ptr;
        uint16_t *indices16Ptr;
        uint8_t  *indices8Ptr;
    };
    int nindices;
};
const int IndexBuffer::kIdent;

struct MatrixBuffer : public IModel::IMatrixBuffer {
    typedef btAlignedObjectArray<int> BoneIndices;
    typedef btAlignedObjectArray<BoneIndices> MeshBoneIndices;
    typedef btAlignedObjectArray<Transform> MeshLocalTransforms;
    typedef Array<float *> MeshMatrices;
    struct SkinningMeshes {
        MeshBoneIndices bones;
        MeshLocalTransforms transforms;
        MeshMatrices matrices;
        BoneIndices bdef1;
        BoneIndices bdef2;
        BoneIndices bdef4;
        BoneIndices sdef;
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
            buffer.position.setW(Scalar(vertex->type()));
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
                IVertex *vertex = vertices[vertexIndex];
                switch (vertex->type()) {
                case IVertex::kBdef1:
                    meshes.bdef1.push_back(vertexIndex);
                    break;
                case IVertex::kBdef2:
                    meshes.bdef2.push_back(vertexIndex);
                    break;
                case IVertex::kBdef4:
                    meshes.bdef4.push_back(vertexIndex);
                    break;
                case IVertex::kSdef:
                    meshes.sdef.push_back(vertexIndex);
                    break;
                case IVertex::kMaxType:
                default:
                    break;
                }
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

#ifdef VPVL2_LINK_INTEL_TBB

class ParallelUpdateLocalTransformProcessor {
public:
    ParallelUpdateLocalTransformProcessor(Array<pmx::Bone *> *bonesRef)
        : m_bonesRef(bonesRef)
    {
    }
    ~ParallelUpdateLocalTransformProcessor() {
        m_bonesRef = 0;
    }

    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            pmx::Bone *bone = m_bonesRef->at(i);
            bone->performUpdateLocalTransform();
        }
    }

private:
    mutable Array<pmx::Bone *> *m_bonesRef;
};

class ParallelUpdateRigidBodyProcessor {
public:
    ParallelUpdateRigidBodyProcessor(Array<pmx::RigidBody *> *rigidBodiesRef)
        : m_rigidBodiesRef(rigidBodiesRef)
    {
    }
    ~ParallelUpdateRigidBodyProcessor() {
        m_rigidBodiesRef = 0;
    }

    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            pmx::RigidBody *body = m_rigidBodiesRef->at(i);
            body->performTransformBone();
        }
    }

private:
    mutable Array<pmx::RigidBody *> *m_rigidBodiesRef;
};

#endif /* VPVL2_LINK_INTEL_TBB */

static inline bool VPVL2PMXGetBonePosition(const IModel *modelRef,
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
namespace pmx
{

Model::Model(IEncoding *encoding)
    : m_encodingRef(encoding),
      m_parentSceneRef(0),
      m_parentModelRef(0),
      m_parentBoneRef(0),
      m_name(0),
      m_englishName(0),
      m_comment(0),
      m_englishComment(0),
      m_aabbMax(kZeroV3),
      m_aabbMin(kZeroV3),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(1),
      m_edgeWidth(0),
      m_visible(false),
      m_enablePhysics(false)
{
    internal::zerofill(&m_info, sizeof(m_info));
}

Model::~Model()
{
    release();
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
        parseTextures(info);
        parseMaterials(info);
        parseBones(info);
        parseMorphs(info);
        parseLabels(info);
        parseRigidBodies(info);
        parseJoints(info);
        if (!Bone::loadBones(m_bones)
                || !Material::loadMaterials(m_materials, m_textures, m_indices.count())
                || !Vertex::loadVertices(m_vertices, m_bones)
                || !Morph::loadMorphs(m_morphs, m_bones, m_materials, m_rigidBodies, m_vertices)
                || !Label::loadLabels(m_labels, m_bones, m_morphs)
                || !RigidBody::loadRigidBodies(m_rigidBodies, m_bones)
                || !Joint::loadJoints(m_joints, m_rigidBodies)) {
            m_info.error = info.error;
            return false;
        }
        Bone::sortBones(m_bones, m_BPSOrderedBones, m_APSOrderedBones);
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
    uint8_t *base = data;
    uint8_t *signature = reinterpret_cast<uint8_t *>(header.signature);
    internal::writeBytes("PMX ", sizeof(header.signature), signature);
    header.version = 2.0;
    internal::writeBytes(&header, sizeof(header), data);
    IString::Codec codec = IString::kUTF8; // TODO: UTF-16 support
    Flags flags;
    DataInfo info = m_info;
    info.codec = codec;
    flags.additionalUVSize = 0;
    flags.boneIndexSize = Flags::estimateSize(m_bones.count());
    flags.codec = codec == IString::kUTF8 ? 1 : 0;
    flags.materialIndexSize = Flags::estimateSize(m_materials.count());
    flags.morphIndexSize = Flags::estimateSize(m_morphs.count());
    flags.rigidBodyIndexSize = Flags::estimateSize(m_rigidBodies.count());
    flags.textureIndexSize = Flags::estimateSize(m_textures.count());
    flags.vertexIndexSize = Flags::estimateSize(m_vertices.count());
    uint8_t flagSize = sizeof(flags);
    internal::writeBytes(&flagSize, sizeof(flagSize), data);
    internal::writeBytes(&flags, sizeof(flags), data);
    internal::writeString(m_name, codec, data);
    internal::writeString(m_englishName, codec, data);
    internal::writeString(m_comment, codec, data);
    internal::writeString(m_englishComment, codec, data);
    Vertex::writeVertices(m_vertices, info, data);
    const int nindices = m_indices.count();
    internal::writeBytes(&nindices, sizeof(nindices), data);
    for (int i = 0; i < nindices; i++) {
        const int index = m_indices[i];
        internal::writeSignedIndex(index, flags.vertexIndexSize, data);
    }
    const int ntextures = m_textures.count();
    internal::writeBytes(&ntextures, sizeof(ntextures), data);
    for (int i = 0; i < ntextures; i++) {
        const IString *texture = *m_textures.value(i);
        internal::writeString(texture, codec, data);
    }
    Material::writeMaterials(m_materials, info, data);
    Bone::writeBones(m_bones, info, data);
    Morph::writeMorphs(m_morphs, info, data);
    Label::writeLabels(m_labels, info, data);
    RigidBody::writeRigidBodies(m_rigidBodies, info, data);
    Joint::writeJoints(m_joints, info, data);
    written = data - base;
    VPVL2_LOG(VLOG(1) << "PMXEOF: base=" << reinterpret_cast<const void *>(base) << " data=" << reinterpret_cast<const void *>(data) << " written=" << written);
}

size_t Model::estimateSize() const
{
    size_t size = 0;
    IString::Codec codec = IString::kUTF8; // TODO: UTF-16 support
    DataInfo info = m_info;
    info.codec = codec;
    size += sizeof(Header);
    size += sizeof(uint8_t) + sizeof(Flags);
    size += internal::estimateSize(m_name, codec);
    size += internal::estimateSize(m_englishName, codec);
    size += internal::estimateSize(m_comment, codec);
    size += internal::estimateSize(m_englishComment, codec);
    size += Vertex::estimateTotalSize(m_vertices, m_info);
    const int nindices = m_indices.count();
    size += sizeof(nindices);
    size += m_info.vertexIndexSize * nindices;
    const int ntextures = m_textures.count();
    size += sizeof(ntextures);
    for (int i = 0; i < ntextures; i++) {
        IString *texture = *m_textures.value(i);
        size += internal::estimateSize(texture, codec);
    }
    size += Material::estimateTotalSize(m_materials, info);
    size += Bone::estimateTotalSize(m_bones, info);
    size += Morph::estimateTotalSize(m_morphs, info);
    size += Label::estimateTotalSize(m_labels, info);
    size += RigidBody::estimateTotalSize(m_rigidBodies, info);
    size += Joint::estimateTotalSize(m_joints, info);
    return size;
}

void Model::joinWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef && m_enablePhysics) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_rigidBodies[i];
            rigidBody->joinWorld(worldRef);
        }
        const int njoints = m_joints.count();
        for (int i = 0; i < njoints; i++) {
            Joint *joint = m_joints[i];
            joint->joinWorld(worldRef);
        }
    }
}

void Model::leaveWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = nRigidBodies - 1; i >= 0; i--) {
            RigidBody *rigidBody = m_rigidBodies[i];
            rigidBody->leaveWorld(worldRef);
        }
        const int njoints = m_joints.count();
        for (int i = njoints - 1; i >= 0; i--) {
            Joint *joint = m_joints[i];
            joint->leaveWorld(worldRef);
        }
    }
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
    if (!worldRef || !m_enablePhysics) {
        return;
    }
    /* update worldTransform first to use it at RigidBody#setKinematic */
    const int nbones = m_BPSOrderedBones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_BPSOrderedBones[i];
        bone->resetIKLink();
    }
    updateLocalTransform(m_BPSOrderedBones);
    btOverlappingPairCache *cache = worldRef->getPairCache();
    btDispatcher *dispatcher = worldRef->getDispatcher();
    const int nRigidBodies = m_rigidBodies.count();
    Vector3 basePosition(kZeroV3);
    /* get offset position of the model by the bone of root or center for RigidBody#setKinematic */
    if (!VPVL2PMXGetBonePosition(this, m_encodingRef, IEncoding::kRootBone, basePosition)) {
        VPVL2PMXGetBonePosition(this, m_encodingRef, IEncoding::kCenter, basePosition);
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
    updateLocalTransform(m_APSOrderedBones);
}

void Model::performUpdate()
{
    // update local transform matrix
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        bone->resetIKLink();
    }
    // before physics simulation
    updateLocalTransform(m_BPSOrderedBones);
    if (m_enablePhysics) {
        // physics simulation
        const int nRigidBodies = m_rigidBodies.count();
#ifdef VPVL2_LINK_INTEL_TBB
        tbb::parallel_for(tbb::blocked_range<int>(0, nRigidBodies),
                          ParallelUpdateRigidBodyProcessor(&m_rigidBodies));
#else /* VPVL2_LINK_INTEL_TBB */
#pragma omp parallel for
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_rigidBodies[i];
            rigidBody->performTransformBone();
        }
#endif /* VPVL2_LINK_INTEL_TBB */
    }
    // after physics simulation
    updateLocalTransform(m_APSOrderedBones);
}

IBone *Model::findBone(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IBone *const *bone = m_name2boneRefs.find(key);
        return bone ? *bone : 0;
    }
    return 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IMorph *const *morph = m_name2morphRefs.find(key);
        return morph ? *morph : 0;
    }
    return 0;
}

int Model::count(ObjectType value) const
{
    switch (value) {
    case kBone: {
        return m_bones.count();
    }
    case kIK: {
        const int nbones = m_bones.count();
        int nIK = 0;
        for (int i = 0; i < nbones; i++) {
            Bone *bone = static_cast<Bone *>(m_bones[i]);
            if (bone->hasInverseKinematics())
                nIK++;
        }
        return nIK;
    }
    case kIndex: {
        return m_indices.count();
    }
    case kJoint: {
        return m_joints.count();
    }
    case kMaterial: {
        return m_materials.count();
    }
    case kMorph: {
        return m_morphs.count();
    }
    case kRigidBody: {
        return m_rigidBodies.count();
    }
    case kVertex: {
        return m_vertices.count();
    }
    default:
        return 0;
    }
}

void Model::getBoneRefs(Array<IBone *> &value) const
{
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
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
        Morph *morph = m_morphs[i];
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

bool Model::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    if (!data || sizeof(Header) > rest) {
        VPVL2_LOG(LOG(WARNING) << "Data is null or PMX header not satisfied: " << size);
        m_info.error = kInvalidHeaderError;
        return false;
    }
    /* header */
    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header header;
    internal::getData(ptr, header);
    info.basePtr = ptr;
    VPVL2_LOG(VLOG(1) << "PMXBasePtr: ptr=" << static_cast<const void*>(ptr) << " size=" << size);

    /* Check the signature and version is correct */
    if (memcmp(header.signature, "PMX ", 4) != 0) {
        VPVL2_LOG(LOG(WARNING) << "Invalid PMX signature detected: " << header.signature);
        m_info.error = kInvalidSignatureError;
        return false;
    }

    /* version */
    if (header.version != 2.0) {
        VPVL2_LOG(LOG(WARNING) << "Invalid PMX version detected: " << header.version);
        m_info.error = kInvalidVersionError;
        return false;
    }

    /* flags */
    Flags flags;
    uint8_t flagSize;
    internal::drainBytes(sizeof(Header), ptr, rest);
    if (!internal::getTyped<uint8_t>(ptr, rest, flagSize) || flagSize != sizeof(flags)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid PMX flag size: " << flagSize);
        m_info.error = kInvalidFlagSizeError;
        return false;
    }
    if (!internal::getTyped<Flags>(ptr, rest, flags)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid PMX flag data: " << flagSize);
        m_info.error = kInvalidFlagSizeError;
        return false;
    }
    flags.copy(info);
    VPVL2_LOG(VLOG(1) << "PMXFlags(codec): " << info.codec);
    VPVL2_LOG(VLOG(1) << "PMXFlags(additionalUVSize): " << info.additionalUVSize);
    VPVL2_LOG(VLOG(1) << "PMXFlags(vertexIndexSize): " << info.vertexIndexSize);
    VPVL2_LOG(VLOG(1) << "PMXFlags(textureIndexSize): " << info.textureIndexSize);
    VPVL2_LOG(VLOG(1) << "PMXFlags(materialIndexSize): " << info.materialIndexSize);
    VPVL2_LOG(VLOG(1) << "PMXFlags(boneIndexSize): " << info.boneIndexSize);
    VPVL2_LOG(VLOG(1) << "PMXFlags(morphIndexSize): " << info.morphIndexSize);
    VPVL2_LOG(VLOG(1) << "PMXFlags(rigidBodyIndexSize): " << info.rigidBodyIndexSize);

    /* name in Japanese */
    if (!internal::getText(ptr, rest, info.namePtr, info.nameSize)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of name in Japanese detected: " << info.nameSize);
        m_info.error = kInvalidNameSizeError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXName(Japanese): ptr=" << static_cast<const void*>(info.namePtr) << " size=" << info.nameSize);

    /* name in English */
    if (!internal::getText(ptr, rest, info.englishNamePtr, info.englishNameSize)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of name in English detected: " << info.englishNameSize);
        m_info.error = kInvalidEnglishNameSizeError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXName(English): ptr=" << static_cast<const void*>(info.englishNamePtr) << " size=" << info.englishNameSize);

    /* comment in Japanese */
    if (!internal::getText(ptr, rest, info.commentPtr, info.commentSize)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of comment in Japanese detected: " << info.commentSize);
        m_info.error = kInvalidCommentSizeError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXComment(Japanese): ptr=" << static_cast<const void*>(info.commentPtr) << " size=" << info.commentSize);

    /* comment in English */
    if (!internal::getText(ptr, rest, info.englishCommentPtr, info.englishCommentSize)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid size of comment in English detected: " << info.englishCommentSize);
        m_info.error = kInvalidEnglishCommentSizeError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXComment(English): ptr=" << static_cast<const void*>(info.englishCommentPtr) << " size=" << info.englishCommentSize);

    /* vertex */
    if (!Vertex::preparse(ptr, rest, info)) {
        m_info.error = kInvalidVerticesError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXVertices: ptr=" << static_cast<const void*>(info.verticesPtr) << " size=" << info.verticesCount);

    /* indices */
    int nindices;
    if (!internal::getTyped<int>(ptr, rest, nindices) || nindices * info.vertexIndexSize > rest) {
        m_info.error = kInvalidIndicesError;
        return false;
    }
    info.indicesPtr = ptr;
    info.indicesCount = nindices;
    internal::drainBytes(nindices * info.vertexIndexSize, ptr, rest);
    VPVL2_LOG(VLOG(1) << "PMXIndices: ptr=" << static_cast<const void*>(info.indicesPtr) << " size=" << info.indicesCount);

    /* texture lookup table */
    int ntextures;
    if (!internal::getTyped<int>(ptr, rest, ntextures)) {
        m_info.error = kInvalidTextureSizeError;
        return false;
    }

    info.texturesPtr = ptr;
    for (int i = 0; i < ntextures; i++) {
        int nNameSize;
        uint8_t *namePtr;
        if (!internal::getText(ptr, rest, namePtr, nNameSize)) {
            m_info.error = kInvalidTextureError;
            return false;
        }
    }
    info.texturesCount = ntextures;
    VPVL2_LOG(VLOG(1) << "PMXTextures: ptr=" << static_cast<const void*>(info.texturesPtr) << " size=" << info.texturesCount);

    /* material */
    if (!Material::preparse(ptr, rest, info)) {
        m_info.error = kInvalidMaterialsError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXMaterials: ptr=" << static_cast<const void*>(info.materialsPtr) << " size=" << info.materialsCount);

    /* bone */
    if (!Bone::preparse(ptr, rest, info)) {
        m_info.error = kInvalidBonesError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXBones: ptr=" << static_cast<const void*>(info.bonesPtr) << " size=" << info.bonesCount);

    /* morph */
    if (!Morph::preparse(ptr, rest, info)) {
        m_info.error = kInvalidMorphsError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXMorphs: ptr=" << static_cast<const void*>(info.morphsPtr) << " size=" << info.morphsCount);

    /* display name table */
    if (!Label::preparse(ptr, rest, info)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXLabels: ptr=" << static_cast<const void*>(info.labelsPtr) << " size=" << info.labelsCount);

    /* rigid body */
    if (!RigidBody::preparse(ptr, rest, info)) {
        m_info.error = kInvalidRigidBodiesError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXRigidBodies: ptr=" << static_cast<const void*>(info.rigidBodiesPtr) << " size=" << info.rigidBodiesCount);

    /* constraint */
    if (!Joint::preparse(ptr, rest, info)) {
        m_info.error = kInvalidJointsError;
        return false;
    }
    VPVL2_LOG(VLOG(1) << "PMXJoints: ptr=" << static_cast<const void*>(info.jointsPtr) << " size=" << info.jointsCount);

    info.endPtr = ptr;
    info.encoding = m_encodingRef;
    VPVL2_LOG(VLOG(1) << "PMXEOD: ptr=" << static_cast<const void*>(ptr) << " rest=" << rest);

    return rest == 0;
}

void Model::setVisible(bool value)
{
    m_visible = value;
}

Scalar Model::edgeScaleFactor(const Vector3 &cameraPosition) const
{
    Scalar length = 0;
    if (m_bones.count() > 1) {
        IBone *bone = m_bones.at(1);
        length = (cameraPosition - bone->worldTransform().getOrigin()).length() * m_edgeWidth;
    }
    return length / 1000.0f;
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

void Model::release()
{
    m_textures.releaseAll();
    m_vertices.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_labels.releaseAll();
    m_rigidBodies.releaseAll();
    m_joints.releaseAll();
    internal::zerofill(&m_info, sizeof(m_info));
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    delete m_comment;
    m_comment = 0;
    delete m_englishComment;
    m_englishComment = 0;
    m_parentSceneRef = 0;
    m_parentModelRef = 0;
    m_parentBoneRef = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
    m_opacity = 1;
    m_scaleFactor = 1;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(info.namePtr, info.nameSize, info.codec), m_name);
    internal::setStringDirect(m_encodingRef->toString(info.englishNamePtr, info.englishNameSize, info.codec), m_englishName);
    internal::setStringDirect(m_encodingRef->toString(info.commentPtr, info.commentSize, info.codec), m_comment);
    internal::setStringDirect(m_encodingRef->toString(info.englishCommentPtr, info.englishCommentSize, info.codec), m_englishComment);
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
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.indicesPtr;
    size_t size = info.vertexIndexSize;
    for (int i = 0; i < nindices; i++) {
        int index = internal::readUnsignedIndex(ptr, size);
        if (index >= 0 && index < nvertices) {
            m_indices.append(index);
        }
        else {
            m_indices.append(0);
        }
    }
}

void Model::parseTextures(const DataInfo &info)
{
    const int ntextures = info.texturesCount;
    size_t rest = SIZE_MAX;
    uint8_t *ptr = info.texturesPtr;
    uint8_t *texturePtr;
    int size;
    for (int i = 0; i < ntextures; i++) {
        internal::getText(ptr, rest, texturePtr, size);
        IString *value = m_encodingRef->toString(texturePtr, size, info.codec);
        m_textures.insert(value->toHashString(), value);
    }
}

void Model::parseMaterials(const DataInfo &info)
{
    const int nmaterials = info.materialsCount, nindices = m_indices.count();
    uint8_t *ptr = info.materialsPtr;
    size_t size, offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        Material *material = m_materials.append(new Material(this));
        material->read(ptr, info, size);
        ptr += size;
        IMaterial::IndexRange range = material->indexRange();
        int offsetTo = offset + range.count;
        range.start = nindices;
        range.end = 0;
        for (int j = offset; j < offsetTo; j++) {
            const int index = m_indices.at(j);
            IVertex *vertex = m_vertices[index];
            vertex->setMaterial(material);
            btSetMin(range.start, index);
            btSetMax(range.end, index);
        }
        material->setIndexRange(range);
        offset = offsetTo;
    }
}

void Model::parseBones(const DataInfo &info)
{
    const int nbones = info.bonesCount;
    uint8_t *ptr = info.bonesPtr;
    size_t size;
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones.append(new Bone(this));
        bone->read(ptr, info, size);
        bone->performTransform();
        bone->performUpdateLocalTransform();
        m_name2boneRefs.insert(bone->name()->toHashString(), bone);
        m_name2boneRefs.insert(bone->englishName()->toHashString(), bone);
        ptr += size;
    }
}

void Model::parseMorphs(const DataInfo &info)
{
    const int nmorphs = info.morphsCount;
    uint8_t *ptr = info.morphsPtr;
    size_t size;
    for(int i = 0; i < nmorphs; i++) {
        Morph *morph = m_morphs.append(new Morph(this));
        morph->read(ptr, info, size);
        m_name2morphRefs.insert(morph->name()->toHashString(), morph);
        m_name2morphRefs.insert(morph->englishName()->toHashString(), morph);
        ptr += size;
    }
}

void Model::parseLabels(const DataInfo &info)
{
    const int nlabels = info.labelsCount;
    uint8_t *ptr = info.labelsPtr;
    size_t size;
    for(int i = 0; i < nlabels; i++) {
        Label *label = m_labels.append(new Label(this));
        label->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseRigidBodies(const DataInfo &info)
{
    const int nRigidBodies = info.rigidBodiesCount;
    uint8_t *ptr = info.rigidBodiesPtr;
    size_t size;
    for(int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies.append(new RigidBody());
        rigidBody->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseJoints(const DataInfo &info)
{
    const int nJoints = info.jointsCount;
    uint8_t *ptr = info.jointsPtr;
    size_t size;
    for(int i = 0; i < nJoints; i++) {
        Joint *joint = m_joints.append(new Joint());
        joint->read(ptr, info, size);
        ptr += size;
    }
}

void Model::updateLocalTransform(Array<Bone *> &bones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        bone->performFullTransform();
        bone->solveInverseKinematics();
    }
#ifdef VPVL2_LINK_INTEL_TBB
    static tbb::affinity_partitioner updateLocalTransformAffinityPartitioner;
    tbb::parallel_for(tbb::blocked_range<int>(0, nbones),
                      ParallelUpdateLocalTransformProcessor(&bones),
                      updateLocalTransformAffinityPartitioner);
#else /* VPVL2_LINK_INTEL_TBB */
#pragma omp parallel for
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        bone->performUpdateLocalTransform();
    }
#endif /* VPVL2_LINK_INTEL_TBB */
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

void Model::setAabb(const Vector3 &min, const Vector3 &max)
{
    m_aabbMin = min;
    m_aabbMax = max;
}

void Model::getAabb(Vector3 &min, Vector3 &max) const
{
    min = m_aabbMin;
    max = m_aabbMax;
}

IBone *Model::createBone()
{
    return new Bone(this);
}

ILabel *Model::createLabel()
{
    return new Label(this);
}

IMaterial *Model::createMaterial()
{
    return new Material(this);
}

IMorph *Model::createMorph()
{
    return new Morph(this);
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
    if (value && value->index() == -1 && value->parentModelRef() == this) {
        Bone *bone = static_cast<Bone *>(value);
        bone->setIndex(m_bones.count());
        m_bones.append(bone);
        Bone::sortBones(m_bones, m_BPSOrderedBones, m_APSOrderedBones);
    }
}

void Model::addLabel(ILabel *value)
{
    if (value && value->index() == -1 && value->parentModelRef() == this) {
        Label *label = static_cast<Label *>(value);
        label->setIndex(m_labels.count());
        m_labels.append(label);
    }
}

void Model::addMaterial(IMaterial *value)
{
    if (value && value->index() == -1 && value->parentModelRef() == this) {
        Material *material = static_cast<Material *>(value);
        material->setIndex(m_materials.count());
        m_materials.append(material);
    }
}

void Model::addMorph(IMorph *value)
{
    if (value && value->index() == -1 && value->parentModelRef() == this) {
        Morph *morph = static_cast<Morph *>(value);
        morph->setIndex(m_morphs.count());
        m_morphs.append(morph);
    }
}

void Model::addVertex(IVertex *value)
{
    if (value && value->index() == -1 && value->parentModelRef() == this) {
        Vertex *vertex = static_cast<Vertex *>(value);
        vertex->setIndex(m_vertices.count());
        m_vertices.append(vertex);
    }
}

void Model::removeBone(IBone *value)
{
    if (value && value->parentModelRef() == this) {
        Bone *bone = static_cast<Bone *>(value);
        m_bones.remove(bone);
        bone->setIndex(-1);
    }
}

void Model::removeLabel(ILabel *value)
{
    if (value && value->parentModelRef() == this) {
        Label *label = static_cast<Label *>(value);
        m_labels.remove(label);
        label->setIndex(-1);
    }
}

void Model::removeMaterial(IMaterial *value)
{
    if (value && value->parentModelRef() == this) {
        Material *material = static_cast<Material *>(value);
        m_materials.remove(material);
        material->setIndex(-1);
    }
}

void Model::removeMorph(IMorph *value)
{
    if (value && value->parentModelRef() == this) {
        Morph *morph = static_cast<Morph *>(value);
        m_morphs.remove(morph);
        morph->setIndex(-1);
    }
}

void Model::removeVertex(IVertex *value)
{
    if (value && value->parentModelRef() == this) {
        Vertex *vertex = static_cast<Vertex *>(value);
        m_vertices.remove(vertex);
        vertex->setIndex(-1);
    }
}

void Model::addTexture(const IString *value)
{
    if (value) {
        const HashString &key = value->toHashString();
        if (!m_textures.find(key)) {
            m_textures.insert(key, value->clone());
        }
    }
}

}
}
