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
#include "vpvl2/internal/ModelHelper.h"

#include "vpvl2/pmd/Bone.h"
#include "vpvl2/pmd/Label.h"
#include "vpvl2/pmd/Material.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Morph.h"
#include "vpvl2/pmd/Vertex.h"
#include "vpvl2/internal/ParallelProcessors.h"

#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include "vpvl/RigidBody.h"

namespace vpvl2
{
namespace pmd
{

struct DefaultStaticVertexBuffer : public IModel::StaticVertexBuffer {
    struct Unit {
        Unit() {}
        void update(const IVertex *vertex) {
            IBone *bone1 = vertex->boneRef(0), *bone2 = vertex->boneRef(1);
            texcoord = vertex->textureCoord();
            boneIndices.setValue(Scalar(bone1->index()), Scalar(bone2->index()), 0, 0);
            boneWeights.setValue(Scalar(vertex->weight(0)), 0, 0, 0);
        }
        Vector3 texcoord;
        Vector4 boneIndices;
        Vector4 boneWeights;
    };
    static const Unit kIdent;

    DefaultStaticVertexBuffer(const Model *model)
        : modelRef(model)
    {
    }
    ~DefaultStaticVertexBuffer() {
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
const DefaultStaticVertexBuffer::Unit DefaultStaticVertexBuffer::kIdent = DefaultStaticVertexBuffer::Unit();

struct DefaultDynamicVertexBuffer : public IModel::DynamicVertexBuffer {
    struct Unit {
        Unit() {}
        void update(const IVertex *vertex, int index) {
            position = vertex->origin();
            normal = vertex->normal();
            normal[3] = Scalar(vertex->edgeSize());
            edge[3] = Scalar(index);
            uva0.setValue(0, 0, 0, 1);
        }
        void update(const IVertex *vertex, const IVertex::EdgeSizePrecision &materialEdgeSize, int index, Vector3 &p) {
            Vector3 n;
            const IVertex::EdgeSizePrecision &edgeSize = vertex->edgeSize() * materialEdgeSize;
            vertex->performSkinning(p, n);
            position = p;
            normal = n;
            normal[3] = Scalar(vertex->edgeSize());
            edge = position + normal * Scalar(edgeSize);
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

    DefaultDynamicVertexBuffer(const Model *model, const IModel::IndexBuffer *indexBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          enableSkinning(true),
          enableParallelUpdate(false)
    {
    }
    ~DefaultDynamicVertexBuffer() {
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
        const Array<IVertex *> &vertices = modelRef->vertices();
        Unit *bufferPtr = static_cast<Unit *>(address);
        if (enableSkinning) {
            internal::ParallelSkinningVertexProcessor<pmd::Model, IVertex, Unit> processor(modelRef, &vertices, cameraPosition, bufferPtr);
            processor.execute(enableParallelUpdate);
            aabbMin = processor.aabbMin();
            aabbMax = processor.aabbMax();
        }
        else {
            internal::ParallelInitializeVertexProcessor<pmd::Model, IVertex, Unit> processor(&vertices, address);
            processor.execute(enableParallelUpdate);
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
    const IModel::IndexBuffer *indexBufferRef;
    bool enableSkinning;
    bool enableParallelUpdate;
};
const DefaultDynamicVertexBuffer::Unit DefaultDynamicVertexBuffer::kIdent = DefaultDynamicVertexBuffer::Unit();

struct DefaultIndexBuffer : public IModel::IndexBuffer {
    static const uint16_t kIdent = 0;

    DefaultIndexBuffer(const vpvl::Array<uint16_t> &indices, const int nvertices)
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
        internal::ModelHelper::swapIndices(&indicesPtr[0], indicesPtr.count());
#endif
    }
    ~DefaultIndexBuffer() {
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
const uint16_t DefaultIndexBuffer::kIdent;

struct DefaultMatrixBuffer : public IModel::MatrixBuffer {
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

    DefaultMatrixBuffer(const IModel *model, const DefaultIndexBuffer *indexBuffer, DefaultDynamicVertexBuffer *dynamicBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          dynamicBufferRef(dynamicBuffer)
    {
        model->getBoneRefs(bones);
        model->getMaterialRefs(materials);
        model->getVertexRefs(vertices);
        initialize();
    }
    ~DefaultMatrixBuffer() {
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
        DefaultDynamicVertexBuffer::Unit *units = static_cast<DefaultDynamicVertexBuffer::Unit *>(address);
        for (int i = 0; i < nvertices; i++) {
            const IVertex *vertex = vertices[i];
            DefaultDynamicVertexBuffer::Unit &buffer = units[i];
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
            const IMaterial::IndexRange &range = material->indexRange();
            const int nindices = range.count;
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
    const DefaultIndexBuffer *indexBufferRef;
    DefaultDynamicVertexBuffer *dynamicBufferRef;
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
      m_parentSceneRef(0),
      m_parentModelRef(0),
      m_parentBoneRef(0),
      m_aabbMax(kZeroV3),
      m_aabbMin(kZeroV3),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(1),
      m_edgeColor(kZeroV3),
      m_edgeWidth(0),
      m_enableSkinning(true),
      m_enablePhysics(false)
{
    m_model.setSoftwareSkinningEnable(false);
    m_edgeColor.setW(1);
}

Model::~Model()
{
    m_bones.releaseAll();
    m_labels.releaseAll();
    m_materials.releaseAll();
    m_morphs.releaseAll();
    m_vertices.releaseAll();
    m_createdBones.releaseAll();
    m_createdMaterials.releaseAll();
    m_createdMorphs.releaseAll();
    m_createdVertices.releaseAll();
    m_encodingRef = 0;
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

void Model::save(uint8_t *data, size_t &written) const
{
    m_model.save(data);
    written = estimateSize();
}

size_t Model::estimateSize() const
{
    return m_model.estimateSize();
}

void Model::joinWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (m_enablePhysics) {
        m_model.joinWorld(worldRef);
    }
}

void Model::leaveWorld(btDiscreteDynamicsWorld *worldRef)
{
    m_model.leaveWorld(worldRef);
}

void Model::resetMotionState(btDiscreteDynamicsWorld *worldRef)
{
    m_model.leaveWorld(worldRef);
    if (m_enablePhysics) {
        m_model.joinWorld(worldRef);
        btOverlappingPairCache *cache = worldRef->getPairCache();
        btDispatcher *dispatcher = worldRef->getDispatcher();
        const vpvl::RigidBodyList &rigidBodies = m_model.rigidBodies();
        const int nRigidBodies = rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            vpvl::RigidBody *rigidBody = rigidBodies[i];
            if (cache) {
                btRigidBody *body = rigidBody->body();
                cache->cleanProxyFromPairs(body->getBroadphaseHandle(), dispatcher);
            }
        }
    }
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

IBone *Model::findBoneRef(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IBone *const *bone = m_name2boneRefs.find(key);
        return bone ? *bone : 0;
    }
    return 0;
}

IMorph *Model::findMorphRef(const IString *value) const
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
    case kTextures:
    default:
        return 0;
    }
}

void Model::getIndices(Array<int> &value) const
{
    const vpvl::IndexList &indices = m_model.indices();
    const int nindices = indices.count();
    value.clear();
    for (int i = 0; i < nindices; i++) {
        int index = indices[i];
        value.append(index);
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
    IBone *bone = findBoneRef(m_encodingRef->stringConstant(IEncoding::kCenter));
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

IVertex::EdgeSizePrecision Model::edgeScaleFactor(const Vector3 &cameraPosition) const
{
    IVertex::EdgeSizePrecision length = 0;
    if (m_bones.count() > 1) {
        IBone *bone = m_bones.at(1);
        length = (cameraPosition - bone->worldTransform().getOrigin()).length();
    }
    return length / IVertex::EdgeSizePrecision(1000.0);
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

void Model::setEdgeWidth(const IVertex::EdgeSizePrecision &value)
{
    m_model.setEdgeOffset(Scalar(value));
    m_edgeWidth = value;
}

void Model::setParentSceneRef(Scene *value)
{
    m_parentSceneRef = value;
}

void Model::setParentModelRef(IModel *value)
{
    if (!internal::ModelHelper::hasModelLoopChain(value, this)) {
        m_parentModelRef = value;
    }
}

void Model::setParentBoneRef(IBone *value)
{
    if (!internal::ModelHelper::hasBoneLoopChain(value, this)) {
        m_parentBoneRef = value;
    }
}

void Model::setVisible(bool value)
{
    m_model.setVisible(value);
}

void Model::setPhysicsEnable(bool value)
{
    m_enablePhysics = value;
}

void Model::getIndexBuffer(IndexBuffer *&indexBuffer) const
{
    delete indexBuffer;
    indexBuffer = new DefaultIndexBuffer(m_model.indices(), m_vertices.count());
}

void Model::getStaticVertexBuffer(StaticVertexBuffer *&staticBuffer) const
{
    delete staticBuffer;
    staticBuffer = new DefaultStaticVertexBuffer(this);
}

void Model::getDynamicVertexBuffer(DynamicVertexBuffer *&dynamicBuffer, const IndexBuffer *indexBuffer) const
{
    delete dynamicBuffer;
    if (indexBuffer && indexBuffer->ident() == &DefaultIndexBuffer::kIdent) {
        dynamicBuffer = new DefaultDynamicVertexBuffer(this, indexBuffer);
    }
    else {
        dynamicBuffer = 0;
    }
}

void Model::getMatrixBuffer(MatrixBuffer *&matrixBuffer, DynamicVertexBuffer *dynamicBuffer, const IndexBuffer *indexBuffer) const
{
    delete matrixBuffer;
    delete matrixBuffer;
    if (indexBuffer && indexBuffer->ident() == &DefaultIndexBuffer::kIdent &&
            dynamicBuffer && dynamicBuffer->ident() == &DefaultDynamicVertexBuffer::kIdent) {
        matrixBuffer = new DefaultMatrixBuffer(this,
                                               static_cast<const DefaultIndexBuffer *>(indexBuffer),
                                               static_cast<DefaultDynamicVertexBuffer *>(dynamicBuffer));
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

float32_t Model::version() const
{
    return 1.0f;
}

void Model::setVersion(float32_t /* value */)
{
    /* do nothing */
}

IBone *Model::createBone()
{
    return new Bone(this, m_createdBones.append(new vpvl::Bone()), m_encodingRef);
}

IJoint *Model::createJoint()
{
    return 0;
}

ILabel *Model::createLabel()
{
    return new Label(this, 0, m_bones, m_encodingRef, -1, false);
}

IMaterial *Model::createMaterial()
{
    return new Material(this, m_createdMaterials.append(new vpvl::Material()), m_encodingRef, &m_model, -1);
}

IMorph *Model::createMorph()
{
    return new Morph(this, m_createdMorphs.append(new vpvl::Face()), m_encodingRef);
}

IRigidBody *Model::createRigidBody()
{
    return 0;
}

IVertex *Model::createVertex()
{
    return new Vertex(this, m_createdVertices.append(new vpvl::Vertex()), &m_bones, -1);
}

IBone *Model::findBoneRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<IBone, IBone>(m_bones, value);
}

IJoint *Model::findJointRefAt(int /* value */) const
{
    return 0;
}

ILabel *Model::findLabelRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<ILabel, ILabel>(m_labels, value);
}

IMaterial *Model::findMaterialRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<IMaterial, IMaterial>(m_materials, value);
}

IMorph *Model::findMorphRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<IMorph, IMorph>(m_morphs, value);
}

IRigidBody *Model::findRigidBodyRefAt(int /* value */) const
{
    return 0;
}

IVertex *Model::findVertexRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<IVertex, IVertex>(m_vertices, value);
}

void Model::setIndices(const Array<int> &value)
{
    const int nindices = value.count();
    const int nvertices = m_vertices.count();
    vpvl::IndexList newIndices;
    for (int i = 0; i < nindices; i++) {
        int index = value[i];
        if (internal::checkBound(index, 0, nvertices)) {
            newIndices.add(index);
        }
        else {
            newIndices.add(0);
        }
    }
    m_model.setIndices(newIndices);
}

void Model::addBone(IBone *value)
{
    internal::ModelHelper::addObject2<Bone>(this, value, m_bones);
}

void Model::addJoint(IJoint * /* value */)
{
}

void Model::addLabel(ILabel *value)
{
    internal::ModelHelper::addObject2<Label>(this, value, m_labels);
}

void Model::addMaterial(IMaterial *value)
{
    internal::ModelHelper::addObject2<Material>(this, value, m_materials);
}

void Model::addMorph(IMorph *value)
{
    internal::ModelHelper::addObject2<Morph>(this, value, m_morphs);
}

void Model::addRigidBody(IRigidBody * /* value */)
{
}

void Model::addVertex(IVertex *value)
{
    internal::ModelHelper::addObject2<Vertex>(this, value, m_vertices);
}

void Model::removeBone(IBone *value)
{
    internal::ModelHelper::removeObject2<Bone>(this, value, m_bones);
}

void Model::removeJoint(IJoint * /* value */)
{
}

void Model::removeLabel(ILabel *value)
{
    internal::ModelHelper::removeObject2<Label>(this, value, m_labels);
}

void Model::removeMaterial(IMaterial *value)
{
    internal::ModelHelper::removeObject2<Material>(this, value, m_materials);
}

void Model::removeMorph(IMorph *value)
{
    internal::ModelHelper::removeObject2<Morph>(this, value, m_morphs);
}

void Model::removeRigidBody(IRigidBody * /* value */)
{
}

void Model::removeVertex(IVertex *value)
{
    internal::ModelHelper::removeObject2<Vertex>(this, value, m_vertices);
}

void Model::loadBones(Hash<HashPtr, Bone *> &bone2bone)
{
    /* convert bones (vpvl::Bone => vpvl2::IBone) */
    const vpvl::BoneList &bones = m_model.bones();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vpvl::Bone *b = bones[i];
        Bone *bone = m_bones.append(new Bone(this, b, m_encodingRef));
        bone->setParentBone(b);
        bone->setChildBone(b);
        bone->setIndex(b->id());
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
        if (Bone *const *valuePtr = bone2bone.find(ik->destinationBone())) {
            Bone *value = *valuePtr;
            value->setIK(ik, bone2bone);
        }
    }
}

void Model::loadLabels(const Hash<HashPtr, Bone *> &bone2bone)
{
    /* build first bone label (this is special label) */
    Array<IBone *> bones2, firstBone;
    firstBone.append(m_bones[0]);
    Label *label = m_labels.append(new Label(this, reinterpret_cast<const uint8_t *>("Root"), firstBone, m_encodingRef, 0, true));
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
            if (Bone *const *valuePtr = bone2bone.find(bone)) {
                Bone *value = *valuePtr;
                bones2.append(value);
            }
        }
        label = m_labels.append(new Label(this, name, bones2, m_encodingRef, i + 1, false));
    }
}

void Model::loadMaterials()
{
    const vpvl::MaterialList &materials = m_model.materials();
    const vpvl::IndexList &indices = m_model.indices();
    const int nmaterials = materials.count(), nindices = m_model.indices().count();
    int offset = 0;
    for (int i = 0; i < nmaterials; i++) {
        vpvl::Material *m = materials[i];
        IMaterial *material = m_materials.append(new Material(this, m, m_encodingRef, &m_model, i));
        IMaterial::IndexRange range = material->indexRange();
        int offsetTo = offset + range.count;
        range.start = nindices;
        range.end = 0;
        for (int j = offset; j < offsetTo; j++) {
            const int index = indices.at(j);
            IVertex *vertex = m_vertices[index];
            vertex->setMaterialRef(material);
            btSetMin(range.start, index);
            btSetMax(range.end, index);
        }
        material->setIndexRange(range);
        offset = offsetTo;
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
            Morph *morph = m_morphs.append(new Morph(this, face, m_encodingRef));
            morph->setIndex(i);
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
        m_vertices.append(new Vertex(this, vertex, &m_bones, i));
    }
}

}
}
