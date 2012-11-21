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
#include "vpvl2/pmd/Label.h"
#include "vpvl2/pmd/Material.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Morph.h"
#include "vpvl2/pmd/Vertex.h"
#include "vpvl2/internal/ParallelVertexProcessor.h"

namespace vpvl2
{
namespace pmd
{

struct StaticVertexBuffer : public IModel::IStaticVertexBuffer {
    struct Unit {
        Unit() {}
        void update(const IVertex *vertex) {
            IBone *bone1 = vertex->bone(0), *bone2 = vertex->bone(1);
            texcoord = vertex->textureCoord();
            boneIndices.setValue(Scalar(bone1->index()), Scalar(bone2->index()), 0, 0);
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
        const Array<IVertex *> &vertices = modelRef->vertices();
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

    typedef internal::ParallelSkinningVertexProcessor<pmd::Model, IVertex, Unit>
        ParallelSkinningVertexProcessor;
    typedef internal::ParallelInitializeVertexProcessor<pmd::Model, IVertex, Unit>
        ParallelInitializeVertexProcessor;

    DynamicVertexBuffer(const Model *model, const IModel::IIndexBuffer *indexBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          enableSkinning(true)
    {
    }
    ~DynamicVertexBuffer() {
        modelRef = 0;
        indexBufferRef = 0;
        enableSkinning = false;
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
        const Array<IVertex *> &vertices = modelRef->vertices();
        Unit *bufferPtr = static_cast<Unit *>(address);
        if (enableSkinning) {
#ifdef VPVL2_LINK_INTEL_TBB
            ParallelSkinningVertexProcessor proc(modelRef, &modelRef->vertices(), cameraPosition, bufferPtr);
            tbb::parallel_reduce(tbb::blocked_range<int>(0, vertices.count()), proc);
            aabbMin = proc.aabbMin();
            aabbMax = proc.aabbMax();
#elif defined(VPVL2_ENABLE_OPENMP)
            internal::UpdateModelVerticesOMP(modelRef, vertices, cameraPosition, bufferPtr);
#else
            const Array<IMaterial *> &materials = modelRef->materials();
            const Scalar &esf = modelRef->edgeScaleFactor(cameraPosition);
            const int nmaterials = materials.count();
            Vector3 position;
            int offset = 0;
            for (int i = 0; i < nmaterials; i++) {
                const IMaterial *material = materials[i];
                const int nindices = material->sizeofIndices(), offsetTo = offset + nindices;
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
#endif
        }
        else {
#ifdef VPVL2_LINK_INTEL_TBB
            tbb::parallel_for(tbb::blocked_range<int>(0, vertices.count()),
                              ParallelInitializeVertexProcessor(&modelRef->vertices(), address));
#elif defined(VPVL2_ENABLE_OPENMP)
            internal::InitializeModelVerticesOMP(vertices, bufferPtr);
#else
            const int nvertices = vertices.count();
            for (int i = 0; i < nvertices; i++) {
                const IVertex *vertex = vertices[i];
                Unit &v = bufferPtr[i];
                v.update(vertex, i);
            }
#endif
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

    const Model *modelRef;
    const IModel::IIndexBuffer *indexBufferRef;
    bool enableSkinning;
};
const DynamicVertexBuffer::Unit DynamicVertexBuffer::kIdent = DynamicVertexBuffer::Unit();

struct IndexBuffer : public IModel::IIndexBuffer {
    static const uint16_t kIdent = 0;

    IndexBuffer(const vpvl::Array<uint16_t> &indices, const int nvertices)
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
            const int nindices = material->sizeofIndices();
            for (int j = 0; j < nindices; j++) {
                int vertexIndex = indexBufferRef->indexAt(offset + j);
                meshes.bdef2.push_back(vertexIndex);
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

#ifdef VPVL2_LINK_INTEL_TBB

class ParallelUpdateLocalTransformProcessor {
public:
    ParallelUpdateLocalTransformProcessor(Array<IBone *> *bonesRef)
        : m_bonesRef(bonesRef)
    {
    }
    ~ParallelUpdateLocalTransformProcessor() {
        m_bonesRef = 0;
    }

    void operator()(const tbb::blocked_range<int> &range) const {
        for (int i = range.begin(); i != range.end(); ++i) {
            Bone *bone = static_cast<Bone *>(m_bonesRef->at(i));
            bone->updateLocalTransform();
        }
    }

private:
    mutable Array<IBone *> *m_bonesRef;
};

#endif /* VPVL2_LINK_INTEL_TBB */

Model::Model(IEncoding *encoding)
    : m_encodingRef(encoding),
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
      m_edgeColor(kZeroV3),
      m_edgeWidth(0),
      m_enableSkinning(true)
{
    m_model.setSoftwareSkinningEnable(false);
    m_edgeColor.setW(1);
}

Model::~Model()
{
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_labels.releaseAll();
    m_encodingRef = 0;
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
    m_opacity = 0;
    m_scaleFactor = 0;
    m_edgeColor.setZero();
    m_edgeWidth = 0;
    m_enableSkinning = false;
}

bool Model::load(const uint8_t *data, size_t size)
{
    bool ret = m_model.load(data, size);
    if (ret) {
        Hash<HashPtr, Bone *> bone2bone;
        loadBones(bone2bone);
        loadIKEffectors(bone2bone);
        loadLabels(bone2bone);
        loadVertices();
        loadMaterials();
        loadMorphs();
        /* set vertex ID to bone attribute */
        const int nvertices = m_model.vertices().count();
        uint8_t *ptr = static_cast<uint8_t *>(const_cast<void *>(m_model.boneAttributesPointer()));
        size_t stride = m_model.strideSize(vpvl::PMDModel::kVerticesStride);
        for (int i = 0; i < nvertices; i++) {
            Vector3 *v = reinterpret_cast<Vector3 *>(ptr + i * stride);
            v->setW(Scalar(i));
        }
        delete m_name;
        m_name = m_encodingRef->toString(m_model.name(), IString::kShiftJIS, vpvl::PMDModel::kNameSize);
        delete m_englishName;
        m_englishName = m_encodingRef->toString(m_model.englishName(), IString::kShiftJIS, vpvl::PMDModel::kNameSize);
        delete m_comment;
        m_comment = m_encodingRef->toString(m_model.comment(), IString::kShiftJIS, vpvl::PMDModel::kCommentSize);
        delete m_englishComment;
        m_englishComment = m_encodingRef->toString(m_model.englishComment(), IString::kShiftJIS, vpvl::PMDModel::kCommentSize);
        const vpvl::Color &edgeColor = m_model.edgeColor();
        m_edgeColor.setValue(edgeColor.x(), edgeColor.y(), edgeColor.z());
        m_edgeColor.setW(1);
        m_edgeWidth = m_model.edgeOffset();
        m_model.setVisible(true);
    }
    return ret;
}

void Model::save(uint8_t *data) const
{
    m_model.save(data);
}

size_t Model::estimateSize() const
{
    return m_model.estimateSize();
}

void Model::resetVertices()
{
}

void Model::performUpdate()
{
    m_model.updateImmediate();
    const int nbones = m_bones.count();
#ifdef VPVL2_LINK_INTEL_TBB
    tbb::parallel_for(tbb::blocked_range<int>(0, nbones),
                      ParallelUpdateLocalTransformProcessor(&m_bones));
#else
    for (int i = 0; i < nbones; i++) {
        Bone *bone = static_cast<Bone *>(m_bones[i]);
        bone->updateLocalTransform();
    }
#endif
}

void Model::joinWorld(btDiscreteDynamicsWorld *world)
{
    m_model.joinWorld(world);
}

void Model::leaveWorld(btDiscreteDynamicsWorld *world)
{
    m_model.leaveWorld(world);
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
        return m_model.bones().count();
    case kIK:
        return m_model.IKs().count();
    case kIndex:
        return m_model.indices().count();
    case kJoint:
        return m_model.constraints().count();
    case kMaterial:
        return m_model.materials().count();
    case kMorph:
        return m_model.faces().count();
    case kRigidBody:
        return m_model.rigidBodies().count();
    case kVertex:
        return m_model.vertices().count();
    default:
        return 0;
    }
}

void Model::getBoundingBox(Vector3 &min, Vector3 &max) const
{
    min.setZero();
    max.setZero();
    const uint8_t *verticesPtr = static_cast<const uint8_t *>(m_model.verticesPointer());
    const size_t stride = m_model.strideSize(vpvl::PMDModel::kVerticesStride);
    const int nvertices = m_model.vertices().count();
    size_t offset = m_model.strideOffset(vpvl::PMDModel::kVerticesStride);
    for (int i = 0; i < nvertices; i++) {
        const Vector3 &position = *reinterpret_cast<const Vector3 *>(verticesPtr + offset);
        min.setMin(position);
        max.setMax(position);
        offset += stride;
    }
}

void Model::getBoundingSphere(Vector3 &center, Scalar &radius) const
{
    center.setZero();
    radius = 0;
    IBone *bone = findBone(m_encodingRef->stringConstant(IEncoding::kCenter));
    if (bone) {
        const Vector3 &centerPosition = bone->worldTransform().getOrigin();
        const uint8_t *verticesPtr = static_cast<const uint8_t *>(m_model.verticesPointer());
        const size_t stride = m_model.strideSize(vpvl::PMDModel::kVerticesStride);
        const int nvertices = m_model.vertices().count();
        size_t offset = m_model.strideOffset(vpvl::PMDModel::kVerticesStride);
        for (int i = 0; i < nvertices; i++) {
            const Vector3 &position = *reinterpret_cast<const Vector3 *>(verticesPtr + offset);
            btSetMax(radius, centerPosition.distance2(position));
            offset += stride;
        }
        center = centerPosition;
        radius = btSqrt(radius);
    }
    else {
        Vector3 min, max;
        getBoundingBox(min, max);
        center = (min + max) * 0.5;
        radius = (max - min).length() * 0.5f;
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
    uint8_t *bytes = m_encodingRef->toByteArray(value, IString::kShiftJIS);
    m_model.setName(bytes);
    m_encodingRef->disposeByteArray(bytes);
}

void Model::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
    uint8_t *bytes = m_encodingRef->toByteArray(value, IString::kShiftJIS);
    m_model.setEnglishName(bytes);
    m_encodingRef->disposeByteArray(bytes);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_comment);
    uint8_t *bytes = m_encodingRef->toByteArray(value, IString::kShiftJIS);
    m_model.setComment(bytes);
    m_encodingRef->disposeByteArray(bytes);
}

void Model::setEnglishComment(const IString *value)
{
    internal::setString(value, m_englishComment);
    uint8_t *bytes = m_encodingRef->toByteArray(value, IString::kShiftJIS);
    m_model.setEnglishComment(bytes);
    m_encodingRef->disposeByteArray(bytes);
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
    m_model.setEdgeColor(Color(value.x(), value.y(), value.z(), 1.0));
    m_edgeColor = value;
}

void Model::setEdgeWidth(const Scalar &value)
{
    m_model.setEdgeOffset(value);
    m_edgeWidth = value;
}

void Model::setVisible(bool value)
{
    m_model.setVisible(value);
}

void Model::getIndexBuffer(IIndexBuffer *&indexBuffer) const
{
    delete indexBuffer;
    indexBuffer = new IndexBuffer(m_model.indices(), m_vertices.count());
}

void Model::getStaticVertexBuffer(IStaticVertexBuffer *&staticBuffer) const
{
    delete staticBuffer;
    staticBuffer = new StaticVertexBuffer(this);
}

void Model::getDynamicVertexBuffer(IDynamicVertexBuffer *&dynamicBuffer, const IIndexBuffer *indexBuffer) const
{
    delete dynamicBuffer;
    if (indexBuffer && indexBuffer->ident() == &IndexBuffer::kIdent) {
        dynamicBuffer = new DynamicVertexBuffer(this, indexBuffer);
    }
    else {
        dynamicBuffer = 0;
    }
}

void Model::getMatrixBuffer(IMatrixBuffer *&matrixBuffer, IDynamicVertexBuffer *dynamicBuffer, const IIndexBuffer *indexBuffer) const
{
    delete matrixBuffer;
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

void Model::setSkinnningEnable(bool value)
{
    m_enableSkinning = value;
    m_model.setSoftwareSkinningEnable(value);
}

void Model::loadBones(Hash<HashPtr, Bone *> &bone2bone)
{
    /* convert bones (vpvl::Bone => vpvl2::IBone) */
    const vpvl::BoneList &bones = m_model.bones();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vpvl::Bone *b = bones[i];
        Bone *bone = new Bone(this, b, m_encodingRef);
        bone->setParentBone(b);
        bone->setChildBone(b);
        m_bones.add(bone);
        m_name2boneRefs.insert(bone->name()->toHashString(), bone);
        HashPtr key(b);
        bone2bone.insert(key, bone);
    }
}

void Model::loadIKEffectors(const Hash<HashPtr, Bone *> &bone2bone)
{
    /* set IK */
    const vpvl::IKList &IKs = m_model.IKs();
    const int nIKs = IKs.count();
    for (int i = 0; i < nIKs; i++) {
        vpvl::IK *ik = IKs[i];
        Bone **valuePtr = const_cast<Bone **>(bone2bone.find(ik->destinationBone()));
        if (valuePtr) {
            Bone *value = *valuePtr;
            value->setIK(ik, bone2bone);
        }
    }
}

void Model::loadLabels(const Hash<HashPtr, Bone *> &bone2bone)
{
    /* build first bone label (this is special label) */
    Array<IBone *> bones2, firstBone;
    firstBone.add(m_bones[0]);
    Label *label = new Label(this, reinterpret_cast<const uint8_t *>("Root"), firstBone, m_encodingRef, true);
    m_labels.add(label);
    /* other bone labels */
    const vpvl::Array<vpvl::BoneList *> &bonesForUI = m_model.bonesForUI();
    const vpvl::Array<uint8_t *> &categories = m_model.boneCategoryNames();
    const int ncategories = categories.count();
    for (int i = 0; i < ncategories; i++) {
        const vpvl::BoneList *bonesInCategory = bonesForUI[i];
        const int nBonesInCategory = bonesInCategory->count();
        const uint8_t *name = categories[i];
        bones2.clear();
        for (int j = 0; j < nBonesInCategory; j++) {
            vpvl::Bone *bone = bonesInCategory->at(j);
            Bone **valuePtr = const_cast<Bone **>(bone2bone.find(bone));
            if (valuePtr) {
                Bone *value = *valuePtr;
                bones2.add(value);
            }
        }
        label = new Label(this, name, bones2, m_encodingRef, false);
        m_labels.add(label);
    }
}

void Model::loadMaterials()
{
    const vpvl::MaterialList &materials = m_model.materials();
    const vpvl::IndexList &indices = m_model.indices();
    const int nmaterials = materials.count();
    int offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        vpvl::Material *m = materials[i];
        IMaterial *material = new Material(this, m, m_encodingRef, &m_model, i);
        m_materials.add(material);
        int nindices = m->countIndices(), offsetTo = offset + nindices;
        for (int j = offset; j < offsetTo; j++) {
            const int index = indices.at(j);
            IVertex *vertex = m_vertices[index];
            vertex->setMaterial(material);
        }
        offset += nindices;
    }
}

void Model::loadMorphs()
{
    /* convert morphs (vpvl::Face => vpvl2::IMorph) */
    const vpvl::FaceList &morphs = m_model.faces();
    const int nmorphs = morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        vpvl::Face *face = morphs[i];
        if (face->type() != vpvl::Face::kBase) {
            Morph *morph = new Morph(this, face, m_encodingRef);
            morph->setIndex(i);
            m_morphs.add(morph);
            m_name2morphRefs.insert(morph->name()->toHashString(), morph);
        }
    }
}

void Model::loadVertices()
{
    const vpvl::VertexList &vertices = m_model.vertices();
    const int nvertices = vertices.count();
    for (int i = 0; i < nvertices; i++) {
        vpvl::Vertex *vertex = vertices[i];
        m_vertices.add(new Vertex(this, vertex, &m_bones, i));
    }
}

}
}
