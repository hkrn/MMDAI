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
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/Joint.h"
#include "vpvl2/pmd2/Label.h"
#include "vpvl2/pmd2/Material.h"
#include "vpvl2/pmd2/Model.h"
#include "vpvl2/pmd2/Morph.h"
#include "vpvl2/pmd2/RigidBody.h"
#include "vpvl2/pmd2/Vertex.h"
#include "vpvl2/internal/ParallelProcessors.h"

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
    vpvl2::uint8_t signature[3];
    vpvl2::float32_t version;
    vpvl2::uint8_t name[Model::kNameSize];
    vpvl2::uint8_t comment[Model::kCommentSize];
};

struct IKUnit {
    int16_t rootBoneID;
    int16_t targetBoneID;
    uint8_t nlinks;
    uint16_t niterations;
    float32_t angle;
};

#pragma pack(pop)

struct RawIKConstraint {
    IKUnit unit;
    Array<int> effectorBoneIndices;
};

struct IKConstraint {
    Array<Bone *> effectorBoneRefs;
    Bone *rootBoneRef;
    Bone *targetBoneRef;
    int niterations;
    float32_t angle;
};

struct DefaultStaticVertexBuffer : public IModel::StaticVertexBuffer {
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
const DefaultStaticVertexBuffer::Unit DefaultStaticVertexBuffer::kIdent = DefaultStaticVertexBuffer::Unit();

struct DefaultDynamicVertexBuffer : public IModel::DynamicVertexBuffer {
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
            const Scalar &edgeSize = vertex->edgeSize() * materialEdgeSize;
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
        const PointerArray<Vertex> &vertices = modelRef->vertices();
        Unit *bufferPtr = static_cast<Unit *>(address);
        if (enableSkinning) {
            internal::ParallelSkinningVertexProcessor<pmd2::Model, pmd2::Vertex, Unit> processor(modelRef, &vertices, cameraPosition, bufferPtr);
            processor.execute(enableParallelUpdate);
            aabbMin = processor.aabbMin();
            aabbMax = processor.aabbMax();
        }
        else {
            internal::ParallelInitializeVertexProcessor<pmd2::Model, pmd2::Vertex, Unit> processor(&vertices, address);
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

    DefaultIndexBuffer(const Array<int> &indices, const int nvertices)
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

    void setIndexAt(int i, vpvl2::uint16_t value) {
        indicesPtr[i] = value;
    }
    Array<vpvl2::uint16_t> indicesPtr;
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
    const DefaultIndexBuffer *indexBufferRef;
    DefaultDynamicVertexBuffer *dynamicBufferRef;
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

static const vpvl2::Vector3 kAxisX(1.0f, 0.0f, 0.0f);
const vpvl2::float32_t kMinDistance    = 0.0001f;
const vpvl2::float32_t kMinAngle       = 0.00000001f;
const vpvl2::float32_t kMinAxis        = 0.0000001f;
const vpvl2::float32_t kMinRotationSum = 0.002f;
const vpvl2::float32_t kMinRotation    = 0.00001f;

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

struct Model::PrivateContext {
    PrivateContext(IEncoding *encodingRef, Model *self)
        : selfRef(self),
          sceneRef(0),
          encodingRef(encodingRef),
          parentModelRef(0),
          parentBoneRef(0),
          namePtr(0),
          englishNamePtr(0),
          commentPtr(0),
          englishCommentPtr(0),
          position(kZeroV3),
          rotation(Quaternion::getIdentity()),
          opacity(1),
          scaleFactor(1),
          edgeColor(kZeroV3),
          aabbMax(kZeroV3),
          aabbMin(kZeroV3),
          edgeWidth(0),
          hasEnglish(false),
          visible(false),
          physicsEnabled(false)
    {
        edgeColor.setW(1);
    }
    ~PrivateContext() {
        release();
        encodingRef = 0;
        selfRef = 0;
    }

    static bool preparseIKConstraints(uint8_t *&ptr, size_t &rest, DataInfo &info) {
        uint16_t size;
        if (!internal::getTyped<uint16_t>(ptr, rest, size)) {
            return false;
        }
        info.IKConstraintsCount = size;
        info.IKConstraintsPtr = ptr;
        IKUnit unit;
        size_t unitSize = 0;
        for (size_t i = 0; i < size; i++) {
            if (sizeof(unit) > rest) {
                return false;
            }
            internal::getData(ptr, unit);
            unitSize = sizeof(unit) + unit.nlinks * sizeof(uint16_t);
            if (unitSize > rest) {
                return false;
            }
            internal::drainBytes(unitSize, ptr, rest);
        }
        return true;
    }

    void release() {
        internal::zerofill(&dataInfo, sizeof(dataInfo));
        vertices.releaseAll();
        materials.releaseAll();
        bones.releaseAll();
        rawConstraints.releaseAll();
        morphs.releaseAll();
        labels.releaseAll();
        rigidBodies.releaseAll();
        joints.releaseAll();
        customToonTextures.releaseAll();
        constraints.releaseAll();
        delete namePtr;
        namePtr = 0;
        delete englishNamePtr;
        englishNamePtr = 0;
        delete commentPtr;
        commentPtr = 0;
        delete englishCommentPtr;
        englishCommentPtr = 0;
        position.setZero();
        rotation.setValue(0, 0, 0, 1);
        opacity = 1;
        scaleFactor = 1;
        edgeColor.setZero();
        aabbMax.setZero();
        aabbMin.setZero();
        edgeWidth = 0;
        hasEnglish = false;
        visible = false;
        physicsEnabled = false;
    }
    void parseNamesAndComments(const Model::DataInfo &info) {
        internal::setStringDirect(encodingRef->toString(info.namePtr, IString::kShiftJIS, kNameSize), namePtr);
        internal::setStringDirect(encodingRef->toString(info.englishNamePtr, IString::kShiftJIS, kNameSize), englishNamePtr);
        internal::setStringDirect(encodingRef->toString(info.commentPtr, IString::kShiftJIS, kCommentSize), commentPtr);
        internal::setStringDirect(encodingRef->toString(info.englishCommentPtr, IString::kShiftJIS, kCommentSize), englishCommentPtr);
    }
    void parseVertices(const Model::DataInfo &info) {
        const int nvertices = info.verticesCount;
        uint8_t *ptr = info.verticesPtr;
        size_t size;
        for (int i = 0; i < nvertices; i++) {
            Vertex *vertex = vertices.append(new Vertex(selfRef));
            vertex->read(ptr, info, size);
            ptr += size;
        }
    }
    void parseIndices(const Model::DataInfo &info) {
        const int nindices = info.indicesCount;
        uint8_t *ptr = info.indicesPtr;
        for (int i = 0; i < nindices; i++) {
            uint16_t index = internal::readUnsignedIndex(ptr, sizeof(uint16_t));
            indices.append(index);
        }
    }
    void parseMaterials(const Model::DataInfo &info) {
        const int nmaterials = info.materialsCount, nindices = indices.count();
        uint8_t *ptr = info.materialsPtr;
        size_t size;
        int indexOffset = 0;
        for (int i = 0; i < nmaterials; i++) {
            Material *material = materials.append(new Material(selfRef, encodingRef));
            material->read(ptr, info, size);
            IMaterial::IndexRange range = material->indexRange();
            int indexOffsetTo = indexOffset + range.count;
            range.start = nindices;
            range.end = 0;
            for (int j = indexOffset; j < indexOffsetTo; j++) {
                const int index = indices[j];
                IVertex *vertex = vertices[index];
                vertex->setMaterial(material);
                btSetMin(range.start, index);
                btSetMax(range.end, index);
            }
            material->setIndexRange(range);
            indexOffset = indexOffsetTo;
            ptr += size;
        }
    }
    void parseBones(const Model::DataInfo &info) {
        const int nbones = info.bonesCount;
        const uint8_t *englishPtr = info.englishBoneNamesPtr;
        uint8_t *ptr = info.bonesPtr;
        size_t size;
        for (int i = 0; i < nbones; i++) {
            Bone *bone = bones.append(new Bone(selfRef, encodingRef));
            bone->readBone(ptr, info, size);
            sortedBoneRefs.append(bone);
            name2boneRefs.insert(bone->name()->toHashString(), bone);
            if (hasEnglish) {
                bone->readEnglishName(englishPtr, i);
                name2boneRefs.insert(bone->englishName()->toHashString(), bone);
            }
            ptr += size;
        }
        Bone::loadBones(bones);
        sortedBoneRefs.sort(BonePredication());
        selfRef->performUpdate();
    }
    void parseIKConstraints(const Model::DataInfo &info) {
        const int nconstraints = info.IKConstraintsCount;
        uint8_t *ptr = info.IKConstraintsPtr;
        size_t size;
        for (int i = 0; i < nconstraints; i++) {
            RawIKConstraint *rawConstraint = rawConstraints.append(new RawIKConstraint());
            IKUnit &unit = rawConstraint->unit;
            internal::getData(ptr, unit);
            uint8_t *ptr2 = const_cast<uint8_t *>(ptr + sizeof(unit));
            const int neffectors = unit.nlinks;
            for (int j = 0; j < neffectors; j++) {
                int boneIndex = internal::readUnsignedIndex(ptr2, sizeof(uint16_t));
                rawConstraint->effectorBoneIndices.append(boneIndex);
            }
            int nlinks = unit.nlinks;
            size = sizeof(unit) + sizeof(uint16_t) * nlinks;
            ptr += size;
        }
    }
    void parseMorphs(const Model::DataInfo &info) {
        const int nmorphs = info.morphsCount;
        const uint8_t *englishPtr = info.englishFaceNamesPtr;
        uint8_t *ptr = info.morphsPtr;
        size_t size;
        for (int i = 0; i < nmorphs; i++) {
            Morph *morph = morphs.append(new Morph(selfRef, encodingRef));
            morph->read(ptr, size);
            name2morphRefs.insert(morph->name()->toHashString(), morph);
            if (hasEnglish) {
                morph->readEnglishName(englishPtr, i);
                name2morphRefs.insert(morph->englishName()->toHashString(), morph);
            }
            ptr += size;
        }
    }
    void parseLabels(const Model::DataInfo &info) {
        int ncategories = info.boneCategoryNamesCount;
        uint8_t *boneCategoryNamesPtr = info.boneCategoryNamesPtr;
        size_t size = 0;
        for (int i = 0; i < ncategories; i++) {
            Label::Type type = i == 0 ? Label::kSpecialBoneCategoryLabel : Label::kBoneCategoryLabel;
            Label *label = labels.append(new Label(selfRef, encodingRef, boneCategoryNamesPtr, type));
            label->readEnglishName(info.englishBoneFramesPtr, i);
            boneCategoryNamesPtr += Bone::kCategoryNameSize;
        }
        int nbones = info.boneLabelsCount;
        uint8_t *boneLabelsPtr = info.boneLabelsPtr;
        for (int i = 0; i < nbones; i++) {
            if (Label *label = Label::selectCategory(labels, boneLabelsPtr)) {
                label->read(boneLabelsPtr, info, size);
                boneLabelsPtr += size;
            }
        }
        int nmorphs = info.morphLabelsCount;
        uint8_t *morphLabelsPtr = info.morphLabelsPtr;
        Label *morphCategory = labels.append(new Label(selfRef, encodingRef, 0, Label::kMorphCategoryLabel));
        for (int i = 0; i < nmorphs; i++) {
            morphCategory->read(morphLabelsPtr, info, size);
            morphLabelsPtr += size;
        }
    }
    void parseCustomToonTextures(const Model::DataInfo &info) {
        static const uint8_t kFallbackToonTextureName[] = "toon0.bmp";
        uint8_t *ptr = info.customToonTextureNamesPtr;
        customToonTextures.append(encodingRef->toString(kFallbackToonTextureName,
                                                        sizeof(kFallbackToonTextureName) - 1,
                                                        IString::kUTF8));
        for (int i = 0; i < kMaxCustomToonTextures; i++) {
            IString *customToonTexture = encodingRef->toString(ptr, IString::kShiftJIS, kCustomToonTextureNameSize);
            customToonTextures.append(customToonTexture);
            ptr += kCustomToonTextureNameSize;
        }
    }
    void parseRigidBodies(const Model::DataInfo &info) {
        const int nRigidBodies = info.rigidBodiesCount;
        uint8_t *ptr = info.rigidBodiesPtr;
        size_t size;
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = rigidBodies.append(new RigidBody(selfRef, encodingRef));
            rigidBody->read(ptr, info, size);
            ptr += size;
        }
    }
    void parseJoints(const Model::DataInfo &info) {
        const int njoints = info.jointsCount;
        uint8_t *ptr = info.jointsPtr;
        size_t size;
        for (int i = 0; i < njoints; i++) {
            Joint *joint = joints.append(new Joint(selfRef, encodingRef));
            joint->read(ptr, info, size);
            ptr += size;
        }
    }

    void loadIKConstraint() {
        const int nbones = bones.count();
        const int nconstraints = rawConstraints.count();
        for (int i = 0; i < nconstraints; i++) {
            RawIKConstraint *rawConstraint = rawConstraints[i];
            const IKUnit &unit = rawConstraint->unit;
            int targetIndex = unit.targetBoneID;
            int rootIndex = unit.rootBoneID;
            if (internal::checkBound(targetIndex, 0, nbones) && internal::checkBound(rootIndex, 0, nbones)) {
                Bone *rootBoneRef = bones[rootIndex], *targetBoneRef = bones[targetIndex];
                IKConstraint *constraint = constraints.append(new IKConstraint());
                const Array<int> &effectors = rawConstraint->effectorBoneIndices;
                const int neffectors = effectors.count();
                for (int j = 0; j < neffectors; j++) {
                    int boneIndex = effectors[j];
                    if (internal::checkBound(boneIndex, 0, nbones)) {
                        Bone *effectorBone = bones[boneIndex];
                        constraint->effectorBoneRefs.append(effectorBone);
                    }
                }
                constraint->rootBoneRef = rootBoneRef;
                constraint->targetBoneRef = targetBoneRef;
                constraint->niterations = unit.niterations;
                constraint->angle = unit.angle * SIMD_PI;
                rootBoneRef->setTargetBoneRef(targetBoneRef);
            }
        }
    }

    Model *selfRef;
    Scene *sceneRef;
    IEncoding *encodingRef;
    IModel *parentModelRef;
    IBone *parentBoneRef;
    IString *namePtr;
    IString *englishNamePtr;
    IString *commentPtr;
    IString *englishCommentPtr;
    PointerArray<Vertex> vertices;
    Array<int> indices;
    PointerArray<Material> materials;
    PointerArray<Bone> bones;
    PointerArray<RawIKConstraint> rawConstraints;
    PointerArray<Morph> morphs;
    PointerArray<Label> labels;
    PointerArray<RigidBody> rigidBodies;
    PointerArray<Joint> joints;
    PointerArray<IString> customToonTextures;
    PointerArray<IKConstraint> constraints;
    Array<Bone *> sortedBoneRefs;
    Hash<HashString, IBone *> name2boneRefs;
    Hash<HashString, IMorph *> name2morphRefs;
    DataInfo dataInfo;
    Vector3 position;
    Quaternion rotation;
    Scalar opacity;
    Scalar scaleFactor;
    Vector3 edgeColor;
    Vector3 aabbMax;
    Vector3 aabbMin;
    Scalar edgeWidth;
    bool hasEnglish;
    bool visible;
    bool physicsEnabled;
};

Model::Model(IEncoding *encodingRef)
    : m_context(0)
{
    m_context = new PrivateContext(encodingRef, this);
}

Model::~Model()
{
    m_context->release();
    delete m_context;
    m_context = 0;
}

bool Model::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    Header header;
    if (!data || sizeof(header) > rest) {
        m_context->dataInfo.error = kInvalidHeaderError;
        return false;
    }
    uint8_t *ptr = const_cast<uint8_t *>(data);
    internal::getData(ptr, header);
    info.encoding = m_context->encodingRef;
    info.basePtr = ptr;
    // Check the signature and version is correct
    if (memcmp(header.signature, "Pmd", sizeof(header.signature)) != 0) {
        m_context->dataInfo.error = kInvalidSignatureError;
        return false;
    }
    if (header.version != 1.0) {
        m_context->dataInfo.error = kInvalidVersionError;
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
        m_context->dataInfo.error = kInvalidIndicesError;
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
    if (!PrivateContext::preparseIKConstraints(ptr, rest, info)) {
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
    m_context->hasEnglish = hasEnglish != 0;
    if (m_context->hasEnglish) {
        const size_t boneNameSize = Bone::kNameSize * info.bonesCount;
        const size_t morphNameSize =  Morph::kNameSize * info.morphLabelsCount;
        const size_t boneCategoryNameSize = Bone::kCategoryNameSize * info.boneCategoryNamesCount;
        const size_t required = kNameSize + kCommentSize + boneNameSize + morphNameSize + boneCategoryNameSize;
        if (required > rest) {
            m_context->dataInfo.error = kInvalidEnglishNameSizeError;
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
        m_context->dataInfo.error = kInvalidTextureSizeError;
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
        m_context->release();
        m_context->parseNamesAndComments(info);
        m_context->parseVertices(info);
        m_context->parseIndices(info);
        m_context->parseMaterials(info);
        m_context->parseBones(info);
        m_context->parseIKConstraints(info);
        m_context->parseMorphs(info);
        m_context->parseLabels(info);
        m_context->parseCustomToonTextures(info);
        m_context->parseRigidBodies(info);
        m_context->parseJoints(info);
        m_context->loadIKConstraint();
        if (!Material::loadMaterials(m_context->materials, m_context->customToonTextures, m_context->indices.count())
                || !Vertex::loadVertices(m_context->vertices, m_context->bones)
                || !Morph::loadMorphs(m_context->morphs, m_context->vertices)
                || !Label::loadLabels(m_context->labels, m_context->bones, m_context->morphs)
                || !RigidBody::loadRigidBodies(m_context->rigidBodies, m_context->bones)
                || !Joint::loadJoints(m_context->joints, m_context->rigidBodies)) {
            m_context->dataInfo.error = info.error;
            return false;
        }
        m_context->dataInfo = info;
        return true;
    }
    else {
        m_context->dataInfo.error = info.error;
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
    internal::writeStringAsByteArray(m_context->namePtr, IString::kShiftJIS, m_context->encodingRef, sizeof(header.name), namePtr);
    internal::writeStringAsByteArray(m_context->commentPtr, IString::kShiftJIS, m_context->encodingRef, sizeof(header.comment), commentPtr);
    internal::writeBytes(&header, sizeof(header), data);
    Vertex::writeVertices(m_context->vertices, m_context->dataInfo, data);
    const int32_t nindices = m_context->indices.count();
    internal::writeBytes(&nindices, sizeof(nindices), data);
    for (int32_t i = 0; i < nindices; i++) {
        int index = m_context->indices[i];
        internal::writeUnsignedIndex(index, sizeof(uint16_t), data);
    }
    Material::writeMaterials(m_context->materials, m_context->dataInfo, data);
    Bone::writeBones(m_context->bones, m_context->dataInfo, data);
    const int nconstraints = m_context->rawConstraints.count();
    internal::writeUnsignedIndex(nconstraints, sizeof(uint16_t), data);
    for (int i = 0; i < nconstraints; i++) {
        const RawIKConstraint *constraint = m_context->rawConstraints[i];
        internal::writeBytes(&constraint->unit, sizeof(constraint->unit), data);
        const Array<int> &effectorBoneIndices = constraint->effectorBoneIndices;
        const int nbonesInIK = effectorBoneIndices.count();
        for (int j = 0; j < nbonesInIK; j++) {
            int boneIndex = effectorBoneIndices[j];
            internal::writeSignedIndex(boneIndex, sizeof(uint16_t), data);
        }
    }
    Morph::writeMorphs(m_context->morphs, m_context->dataInfo, data);
    Label::writeLabels(m_context->labels, m_context->dataInfo, data);
    internal::writeSignedIndex(m_context->hasEnglish ? 1 : 0, sizeof(uint8_t), data);
    if (m_context->hasEnglish) {
        internal::writeStringAsByteArray(m_context->englishNamePtr, IString::kShiftJIS, m_context->encodingRef, kNameSize, data);
        internal::writeStringAsByteArray(m_context->englishCommentPtr, IString::kShiftJIS, m_context->encodingRef, kCommentSize, data);
        Bone::writeEnglishNames(m_context->bones, m_context->dataInfo, data);
        Morph::writeEnglishNames(m_context->morphs, m_context->dataInfo, data);
        Label::writeEnglishNames(m_context->labels, m_context->dataInfo, data);
    }
    uint8_t customTextureName[kCustomToonTextureNameSize], *customTextureNamePtr = customTextureName;
    for (int i = 0; i < kMaxCustomToonTextures; i++) {
        const IString *customToonTextureRef = m_context->customToonTextures[i];
        internal::writeStringAsByteArray(customToonTextureRef, IString::kShiftJIS, m_context->encodingRef, sizeof(customTextureName), customTextureNamePtr);
        internal::writeBytes(customTextureName, sizeof(customTextureName), data);
        customTextureNamePtr = customTextureName;
    }
    RigidBody::writeRigidBodies(m_context->rigidBodies, m_context->dataInfo, data);
    Joint::writeJoints(m_context->joints, m_context->dataInfo, data);
    written = data - base;
    VPVL2_VLOG(1, "PMDEOF: base=" << reinterpret_cast<const void *>(base) << " data=" << reinterpret_cast<const void *>(data) << " written=" << written);
}

IModel::ErrorType Model::error() const
{
    return m_context->dataInfo.error;
}

size_t Model::estimateSize() const
{
    size_t size = 0;
    size += sizeof(Header);
    size += Vertex::estimateTotalSize(m_context->vertices, m_context->dataInfo);
    size += sizeof(int32_t) + m_context->indices.count() * sizeof(uint16_t);
    size += Material::estimateTotalSize(m_context->materials, m_context->dataInfo);
    size += Bone::estimateTotalSize(m_context->bones, m_context->dataInfo);
    const uint16_t nconstraints = m_context->rawConstraints.count();
    size += sizeof(nconstraints);
    for (int i = 0; i < nconstraints; i++) {
        const RawIKConstraint *constraint = m_context->rawConstraints[i];
        size += sizeof(constraint->unit);
        size += sizeof(uint16_t) * constraint->effectorBoneIndices.count();
    }
    size += Morph::estimateTotalSize(m_context->morphs, m_context->dataInfo);
    size += Label::estimateTotalSize(m_context->labels, m_context->dataInfo);
    size += sizeof(uint8_t);
    if (m_context->hasEnglish) {
        size += kNameSize;
        size += kCommentSize;
        size += Bone::kNameSize * m_context->dataInfo.bonesCount;
        size += Morph::kNameSize * m_context->dataInfo.morphsCount;
        size += Bone::kCategoryNameSize * m_context->dataInfo.boneCategoryNamesCount;
    }
    size += kCustomToonTextureNameSize * kMaxCustomToonTextures;
    size += RigidBody::estimateTotalSize(m_context->rigidBodies, m_context->dataInfo);
    size += Joint::estimateTotalSize(m_context->joints, m_context->dataInfo);
    return size;
}

void Model::resetMotionState(btDiscreteDynamicsWorld *worldRef)
{
    btOverlappingPairCache *cache = worldRef->getPairCache();
    btDispatcher *dispatcher = worldRef->getDispatcher();
    const int nRigidBodies = m_context->rigidBodies.count();
    Vector3 basePosition(kZeroV3);
    /* get offset position of the model by the bone of root or center for RigidBody#setKinematic */
    if (!VPVL2PMDGetBonePosition(this, m_context->encodingRef, IEncoding::kRootBone, basePosition)) {
        VPVL2PMDGetBonePosition(this, m_context->encodingRef, IEncoding::kCenter, basePosition);
    }
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_context->rigidBodies[i];
        if (cache) {
            btRigidBody *body = rigidBody->body();
            cache->cleanProxyFromPairs(body->getBroadphaseHandle(), dispatcher);
        }
        rigidBody->setKinematic(false, basePosition);
    }
    const int njoints = m_context->joints.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = m_context->joints[i];
        joint->updateTransform();
    }
}

void Model::solveInverseKinematics()
{
    const Array<IKConstraint *> &constraints = m_context->constraints;
    const int nconstraints = constraints.count();
    Quaternion q(Quaternion::getIdentity());
    Matrix3x3 matrix(Matrix3x3::getIdentity());
    Vector3 localRootBonePosition(kZeroV3), localTargetBonePosition(kZeroV3);
    for (int i = 0; i < nconstraints; i++) {
        IKConstraint *constraint = constraints[i];
        const Array<Bone *> &effectorBones = constraint->effectorBoneRefs;
        const int neffectors = effectorBones.count();
        Bone *targetBoneRef = constraint->targetBoneRef;
        const Quaternion &originTargetBoneRotation = targetBoneRef->localRotation();
        const Vector3 &rootBonePosition = constraint->rootBoneRef->worldTransform().getOrigin();
        const Scalar &angleLimit = constraint->angle;
        int niterations = constraint->niterations;
        targetBoneRef->performTransform();
        for (int j = 0; j < niterations; j++) {
            for (int k = 0; k < neffectors; k++) {
                Bone *effectorBone = effectorBones[k];
                const Vector3 &currentTargetBonePosition = targetBoneRef->worldTransform().getOrigin();
                const Transform &effectorTransform = effectorBone->worldTransform().inverse();
                localRootBonePosition = effectorTransform * rootBonePosition;
                localTargetBonePosition = effectorTransform * currentTargetBonePosition;
                if (localRootBonePosition.distance2(localTargetBonePosition) < kMinDistance) {
                    j = niterations;
                    break;
                }
                localRootBonePosition.safeNormalize();
                localTargetBonePosition.safeNormalize();
                const Scalar &dot = localRootBonePosition.dot(localTargetBonePosition);
                if (dot > 1.0f)
                    continue;
                Scalar angle = btAcos(dot);
                if (btFabs(angle) < kMinAngle)
                    continue;
                btClamp(angle, -angleLimit, angleLimit);
                Vector3 axis = localTargetBonePosition.cross(localRootBonePosition);
                if (axis.length2() < kMinAxis && j > 0)
                    continue;
                axis.normalize();
                q.setRotation(axis, angle);
                if (effectorBone->isAxisXAligned()) {
                    if (j == 0) {
                        q.setRotation(kAxisX, btFabs(angle));
                    }
                    else {
                        Scalar x, y, z, cx, cy, cz;
                        matrix.setIdentity();
                        matrix.setRotation(q);
                        matrix.getEulerZYX(z, y, x);
                        matrix.setRotation(effectorBone->localRotation());
                        matrix.getEulerZYX(cz, cy, cx);
                        if (x + cx > SIMD_PI)
                            x = SIMD_PI - cx;
                        if (kMinRotationSum > x + cx)
                            x = kMinRotationSum - cx;
                        btClamp(x, -angleLimit, angleLimit);
                        if (btFabs(x) < kMinRotation)
                            continue;
                        q.setEulerZYX(0.0f, 0.0f, x);
                    }
                    const Quaternion &q2 = q * effectorBone->localRotation();
                    effectorBone->setLocalRotation(q2);
                }
                else {
                    const Quaternion &q2 = effectorBone->localRotation() * q;
                    effectorBone->setLocalRotation(q2);
                }
                for (int l = k; l >= 0; l--) {
                    Bone *bone = effectorBones[l];
                    bone->performTransform();
                }
                targetBoneRef->performTransform();
            }
        }
        targetBoneRef->setLocalRotation(originTargetBoneRotation);
        targetBoneRef->performTransform();
    }
}

void Model::performUpdate()
{
    {
        internal::ParallelResetVertexProcessor<pmd2::Vertex> processor(&m_context->vertices);
        processor.execute();
    }
    const int nmorphs = m_context->morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = m_context->morphs[i];
        morph->update();
    }
    const int nbones = m_context->sortedBoneRefs.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_context->sortedBoneRefs[i];
        bone->performTransform();
    }
    solveInverseKinematics();
    if (m_context->physicsEnabled) {
        internal::ParallelUpdateRigidBodyProcessor<pmd2::RigidBody> processor(&m_context->rigidBodies);
        processor.execute();
    }
}

void Model::joinWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef && m_context->physicsEnabled) {
        const int nRigidBodies = m_context->rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_context->rigidBodies[i];
            worldRef->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->collisionGroupMask());
        }
        const int njoints = m_context->joints.count();
        for (int i = 0; i < njoints; i++) {
            Joint *joint = m_context->joints[i];
            worldRef->addConstraint(joint->constraint());
        }
    }
}

void Model::leaveWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef) {
        const int nRigidBodies = m_context->rigidBodies.count();
        for (int i = nRigidBodies - 1; i >= 0; i--) {
            RigidBody *rigidBody = m_context->rigidBodies[i];
            worldRef->removeCollisionObject(rigidBody->body());
        }
        const int njoints = m_context->joints.count();
        for (int i = njoints - 1; i >= 0; i--) {
            Joint *joint = m_context->joints[i];
            worldRef->removeConstraint(joint->constraint());
        }
    }
}

IBone *Model::findBone(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IBone **bone = const_cast<IBone **>(m_context->name2boneRefs.find(key));
        return bone ? *bone : 0;
    }
    return 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IMorph **morph = const_cast<IMorph **>(m_context->name2morphRefs.find(key));
        return morph ? *morph : 0;
    }
    return 0;
}

int Model::count(ObjectType value) const
{
    switch (value) {
    case kBone:
        return m_context->bones.count();
    case kIK: {
        int nbones = 0, nIKJoints = 0;
        for (int i = 0; i < nbones; i++) {
            if (m_context->bones[i]->hasInverseKinematics())
                nIKJoints++;
        }
        return nIKJoints;
    }
    case kIndex:
        return m_context->indices.count();
    case kJoint:
        return m_context->joints.count();
    case kMaterial:
        return m_context->materials.count();
    case kMorph:
        return m_context->morphs.count();
    case kRigidBody:
        return m_context->rigidBodies.count();
    case kVertex:
        return m_context->vertices.count();
    case kMaxObjectType:
    default:
        return 0;
    }
    return 0;
}

IModel::Type Model::type() const
{
    return kPMDModel;
}

const IString *Model::name() const
{
    return m_context->namePtr;
}

const IString *Model::englishName() const
{
    return m_context->englishNamePtr;
}

const IString *Model::comment() const
{
    return m_context->commentPtr;
}

const IString *Model::englishComment() const
{
    return m_context->englishCommentPtr;
}

bool Model::isVisible() const
{
    return m_context->visible && !btFuzzyZero(m_context->opacity);
}

Vector3 Model::worldPosition() const
{
    return m_context->position;
}

Quaternion Model::worldRotation() const
{
    return m_context->rotation;
}

Scalar Model::opacity() const
{
    return m_context->opacity;
}

Scalar Model::scaleFactor() const
{
    return m_context->scaleFactor;
}

Vector3 Model::edgeColor() const
{
    return m_context->edgeColor;
}

Scalar Model::edgeWidth() const
{
    return m_context->edgeWidth;
}

Scene *Model::parentSceneRef() const
{
    return m_context->sceneRef;
}

IModel *Model::parentModelRef() const
{
    return m_context->parentModelRef;
}

IBone *Model::parentBoneRef() const
{
    return m_context->parentBoneRef;
}

bool Model::isPhysicsEnabled() const
{
    return m_context->physicsEnabled;
}

const PointerArray<Vertex> &Model::vertices() const
{
    return m_context->vertices;
}

const Array<int> &Model::indices() const
{
    return m_context->indices;
}

const PointerArray<Material> &Model::materials() const
{
    return m_context->materials;
}

const PointerArray<Bone> &Model::bones() const
{
    return m_context->bones;
}

const PointerArray<Morph> &Model::morphs() const
{
    return m_context->morphs;
}

const PointerArray<Label> &Model::labels() const
{
    return m_context->labels;
}

const PointerArray<RigidBody> &Model::rigidBodies() const
{
    return m_context->rigidBodies;
}

const PointerArray<Joint> &Model::joints() const
{
    return m_context->joints;
}

void Model::getBoneRefs(Array<IBone *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->bones, value);
}

void Model::getJointRefs(Array<IJoint *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->joints, value);
}

void Model::getLabelRefs(Array<ILabel *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->labels, value);
}

void Model::getMaterialRefs(Array<IMaterial *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->materials, value);
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->morphs, value);
}

void Model::getRigidBodyRefs(Array<IRigidBody *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->rigidBodies, value);
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
    internal::ModelHelper::getObjectRefs(m_context->vertices, value);
}

void Model::getIndices(Array<int> &value) const
{
    value.copy(m_context->indices);
}

IVertex::EdgeSizePrecision Model::edgeScaleFactor(const Vector3 &cameraPosition) const
{
    IVertex::EdgeSizePrecision length = 0;
    if (m_context->bones.count() > 1) {
        IBone *bone = m_context->bones.at(1);
        length = (cameraPosition - bone->worldTransform().getOrigin()).length();
    }
    return length / IVertex::EdgeSizePrecision(1000.0);
}

void Model::setName(const IString *value)
{
    internal::setString(value, m_context->namePtr);
}

void Model::setEnglishName(const IString *value)
{
    internal::setString(value, m_context->englishNamePtr);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_context->commentPtr);
}

void Model::setEnglishComment(const IString *value)
{
    internal::setString(value, m_context->englishCommentPtr);
}

void Model::setWorldPosition(const Vector3 &value)
{
    m_context->position = value;
}

void Model::setWorldRotation(const Quaternion &value)
{
    m_context->rotation = value;
}

void Model::setOpacity(const Scalar &value)
{
    m_context->opacity = value;
}

void Model::setScaleFactor(const Scalar &value)
{
    m_context->scaleFactor = value;
}

void Model::setEdgeColor(const Vector3 &value)
{
    m_context->edgeColor = value;
}

void Model::setEdgeWidth(const Scalar &value)
{
    m_context->edgeWidth = value;
}

void Model::setParentSceneRef(Scene *value)
{
    m_context->sceneRef = value;
}

void Model::setParentModelRef(IModel *value)
{
    if (!internal::ModelHelper::hasModelLoopChain(value, this)) {
        m_context->parentModelRef = value;
    }
}

void Model::setParentBoneRef(IBone *value)
{
    if (!internal::ModelHelper::hasBoneLoopChain(value, m_context->parentModelRef)) {
        m_context->parentBoneRef = value;
    }
}

void Model::setPhysicsEnable(bool value)
{
    m_context->physicsEnabled = value;
}

void Model::setVisible(bool value)
{
    m_context->visible = value;
}

void Model::getAabb(Vector3 &min, Vector3 &max) const
{
    min = m_context->aabbMin;
    max = m_context->aabbMax;
}

void Model::setAabb(const Vector3 &min, const Vector3 &max)
{
    m_context->aabbMin = min;
    m_context->aabbMax = max;
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
    return new Bone(this, m_context->encodingRef);
}

IJoint *Model::createJoint()
{
    return new Joint(this, m_context->encodingRef);
}

ILabel *Model::createLabel()
{
    return new Label(this, m_context->encodingRef, reinterpret_cast<const uint8_t *>(""), Label::kBoneCategoryLabel);
}

IMaterial *Model::createMaterial()
{
    return new Material(this, m_context->encodingRef);
}

IMorph *Model::createMorph()
{
    return new Morph(this, m_context->encodingRef);
}

IRigidBody *Model::createRigidBody()
{
    return new RigidBody(this, m_context->encodingRef);
}

IVertex *Model::createVertex()
{
    return new Vertex(this);
}

IBone *Model::findBoneAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Bone, IBone>(m_context->bones, value);
}

IJoint *Model::findJointAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Joint, IJoint>(m_context->joints, value);
}

ILabel *Model::findLabelAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Label, ILabel>(m_context->labels, value);
}

IMaterial *Model::findMaterialAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Material, IMaterial>(m_context->materials, value);
}

IMorph *Model::findMorphAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Morph, IMorph>(m_context->morphs, value);
}

IRigidBody *Model::findRigidBodyAt(int value) const
{
    return internal::ModelHelper::findObjectAt<RigidBody, IRigidBody>(m_context->rigidBodies, value);
}

IVertex *Model::findVertexAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Vertex, IVertex>(m_context->vertices, value);
}

void Model::setIndices(const Array<int> &value)
{
    const int nindices = value.count();
    const int nvertices = m_context->vertices.count();
    m_context->indices.clear();
    for (int i = 0; i < nindices; i++) {
        int index = value[i];
        if (internal::checkBound(index, 0, nvertices)) {
            m_context->indices.append(index);
        }
        else {
            m_context->indices.append(0);
        }
    }
}

void Model::addBone(IBone *value)
{
    internal::ModelHelper::addObject(this, value, m_context->bones);
}

void Model::addJoint(IJoint *value)
{
    internal::ModelHelper::addObject(this, value, m_context->joints);
}

void Model::addLabel(ILabel *value)
{
    internal::ModelHelper::addObject(this, value, m_context->labels);
}

void Model::addMaterial(IMaterial *value)
{
    internal::ModelHelper::addObject(this, value, m_context->materials);
}

void Model::addMorph(IMorph *value)
{
    internal::ModelHelper::addObject(this, value, m_context->morphs);
}

void Model::addRigidBody(IRigidBody *value)
{
    internal::ModelHelper::addObject(this, value, m_context->rigidBodies);
}

void Model::addVertex(IVertex *value)
{
    internal::ModelHelper::addObject(this, value, m_context->vertices);
}

void Model::removeBone(IBone *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->bones);
}

void Model::removeJoint(IJoint *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->joints);
}

void Model::removeLabel(ILabel *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->labels);
}

void Model::removeMaterial(IMaterial *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->materials);
}

void Model::removeMorph(IMorph *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->morphs);
}

void Model::removeRigidBody(IRigidBody *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->rigidBodies);
}

void Model::removeVertex(IVertex *value)
{
    internal::ModelHelper::removeObject(this, value, m_context->vertices);
}

void Model::getIndexBuffer(IndexBuffer *&indexBuffer) const
{
    delete indexBuffer;
    indexBuffer = new DefaultIndexBuffer(m_context->indices, m_context->vertices.count());
}

void Model::getStaticVertexBuffer(StaticVertexBuffer *&staticBuffer) const
{
    delete staticBuffer;
    staticBuffer = new DefaultStaticVertexBuffer(this);
}

void Model::getDynamicVertexBuffer(DynamicVertexBuffer *&dynamicBuffer,
                                   const IndexBuffer *indexBuffer) const
{
    delete dynamicBuffer;
    if (indexBuffer && indexBuffer->ident() == &DefaultIndexBuffer::kIdent) {
        dynamicBuffer = new DefaultDynamicVertexBuffer(this, indexBuffer);
    }
    else {
        dynamicBuffer = 0;
    }
}

void Model::getMatrixBuffer(MatrixBuffer *&matrixBuffer,
                            DynamicVertexBuffer *dynamicBuffer,
                            const IndexBuffer *indexBuffer) const
{
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

}
}
