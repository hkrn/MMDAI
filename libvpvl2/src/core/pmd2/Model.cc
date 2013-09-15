/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

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
    uint8 signature[3];
    float32 version;
    uint8 name[internal::kPMDModelNameSize];
    uint8 comment[internal::kPMDModelCommentSize];
};

struct IKUnit {
    int16 rootBoneID;
    int16 targetBoneID;
    uint8 njoints;
    uint16 niterations;
    float32 angle;
};

#pragma pack(pop)

struct RawIKConstraint {
    IKUnit unit;
    Array<int> jointBoneIndices;
};

struct IKConstraint {
    Array<Bone *> jointBoneRefs;
    Bone *rootBoneRef;
    Bone *effectorBoneRef;
    int niterations;
    float32 angleLimit;
};

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

    vsize size() const {
        return strideSize() * modelRef->vertices().count();
    }
    vsize strideOffset(StrideType type) const {
        const uint8 *base = reinterpret_cast<const uint8 *>(&kIdent.texcoord);
        switch (type) {
        case kBoneIndexStride:
            return reinterpret_cast<const uint8 *>(&kIdent.boneIndices) - base;
        case kBoneWeightStride:
            return reinterpret_cast<const uint8 *>(&kIdent.boneWeights) - base;
        case kTextureCoordStride:
            return reinterpret_cast<const uint8 *>(&kIdent.texcoord) - base;
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
    vsize strideSize() const {
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

    vsize size() const {
        return strideSize() * modelRef->vertices().count();
    }
    vsize strideOffset(StrideType type) const {
        const uint8 *base = reinterpret_cast<const uint8 *>(&kIdent.position);
        switch (type) {
        case kVertexStride:
            return reinterpret_cast<const uint8 *>(&kIdent.position) - base;
        case kNormalStride:
            return reinterpret_cast<const uint8 *>(&kIdent.normal) - base;
        case kMorphDeltaStride:
            return reinterpret_cast<const uint8 *>(&kIdent.delta) - base;
        case kEdgeVertexStride:
            return reinterpret_cast<const uint8 *>(&kIdent.edge) - base;
        case kEdgeSizeStride:
            return reinterpret_cast<const uint8 *>(&kIdent.normal[3]) - base;
        case kVertexIndexStride:
            return reinterpret_cast<const uint8 *>(&kIdent.edge[3]) - base;
        case kUVA0Stride:
            return reinterpret_cast<const uint8 *>(&kIdent.uva0) - base;
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
    vsize strideSize() const {
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
    static const uint16 kIdent = 0;

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
    vsize size() const {
        return strideSize() * nindices;
    }
    vsize strideOffset(StrideType /* type */) const {
        return 0;
    }
    vsize strideSize() const {
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

    void setIndexAt(int i, uint16 value) {
        indicesPtr[i] = value;
    }
    Array<uint16> indicesPtr;
    int nindices;
};
const uint16 DefaultIndexBuffer::kIdent;

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
    vsize size(int materialIndex) const {
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
    if (const IBone *bone = modelRef->findBoneRef(encodingRef->stringConstant(value))) {
        position = bone->localTransform().getOrigin();
        return !position.fuzzyZero();
    }
    return false;
}

static const Vector3 kAxisX(1.0f, 0.0f, 0.0f);
const float32 kMinDistance    = 0.0001f;
const float32 kMinAngle       = 0.00000001f;
const float32 kMinAxis        = 0.0000001f;
const float32 kMinRotationSum = 0.002f;
const float32 kMinRotation    = 0.00001f;

}

namespace vpvl2
{
namespace pmd2
{

const uint8 *const Model::kFallbackToonTextureName = reinterpret_cast<const uint8 *>("toon0.bmp");

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

    static bool preparseIKConstraints(uint8 *&ptr, vsize &rest, DataInfo &info) {
        uint16 size;
        if (!internal::getTyped<uint16>(ptr, rest, size)) {
            return false;
        }
        info.IKConstraintsCount = size;
        info.IKConstraintsPtr = ptr;
        IKUnit unit;
        vsize unitSize = 0;
        for (vsize i = 0; i < size; i++) {
            if (sizeof(unit) > rest) {
                return false;
            }
            internal::getData(ptr, unit);
            unitSize = sizeof(unit) + unit.njoints * sizeof(uint16);
            if (unitSize > rest) {
                return false;
            }
            internal::drainBytes(unitSize, ptr, rest);
        }
        return true;
    }

    void release() {
        internal::zerofill(&dataInfo, sizeof(dataInfo));
        textures.releaseAll();
        vertices.releaseAll();
        materials.releaseAll();
        bones.releaseAll();
        rawConstraints.releaseAll();
        morphs.releaseAll();
        labels.releaseAll();
        rigidBodies.releaseAll();
        joints.releaseAll();
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
        uint8 *ptr = info.verticesPtr;
        vsize size;
        for (int i = 0; i < nvertices; i++) {
            Vertex *vertex = vertices.append(new Vertex(selfRef));
            vertex->read(ptr, info, size);
            ptr += size;
        }
    }
    void parseIndices(const Model::DataInfo &info) {
        const int nindices = info.indicesCount;
        uint8 *ptr = info.indicesPtr;
        for (int i = 0; i < nindices; i++) {
            uint16 index = internal::readUnsignedIndex(ptr, sizeof(uint16));
            indices.append(index);
        }
    }
    void parseMaterials(const Model::DataInfo &info) {
        const int nmaterials = info.materialsCount, nindices = indices.count();
        uint8 *ptr = info.materialsPtr;
        vsize size;
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
                vertex->setMaterialRef(material);
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
        const uint8 *englishPtr = info.englishBoneNamesPtr;
        uint8 *ptr = info.bonesPtr;
        vsize size;
        for (int i = 0; i < nbones; i++) {
            Bone *bone = bones.append(new Bone(selfRef, encodingRef));
            bone->readBone(ptr, info, size);
            sortedBoneRefs.append(bone);
            name2boneRefs.insert(bone->name(IEncoding::kJapanese)->toHashString(), bone);
            if (hasEnglish) {
                bone->readEnglishName(englishPtr, i);
                name2boneRefs.insert(bone->name(IEncoding::kEnglish)->toHashString(), bone);
            }
            ptr += size;
        }
        Bone::loadBones(bones);
        sortedBoneRefs.sort(BonePredication());
        selfRef->performUpdate();
    }
    void parseIKConstraints(const Model::DataInfo &info) {
        const int nconstraints = info.IKConstraintsCount;
        uint8 *ptr = info.IKConstraintsPtr;
        vsize size;
        for (int i = 0; i < nconstraints; i++) {
            RawIKConstraint *rawConstraint = rawConstraints.append(new RawIKConstraint());
            IKUnit &unit = rawConstraint->unit;
            internal::getData(ptr, unit);
            uint8 *ptr2 = const_cast<uint8 *>(ptr + sizeof(unit));
            const int njoints = unit.njoints;
            for (int j = 0; j < njoints; j++) {
                int boneIndex = internal::readUnsignedIndex(ptr2, sizeof(uint16));
                rawConstraint->jointBoneIndices.append(boneIndex);
            }
            size = sizeof(unit) + sizeof(uint16) * njoints;
            ptr += size;
        }
    }
    void parseMorphs(const Model::DataInfo &info) {
        const int nmorphs = info.morphsCount;
        const uint8 *englishPtr = info.englishFaceNamesPtr;
        uint8 *ptr = info.morphsPtr;
        vsize size;
        for (int i = 0; i < nmorphs; i++) {
            Morph *morph = morphs.append(new Morph(selfRef, encodingRef));
            morph->read(ptr, size);
            name2morphRefs.insert(morph->name(IEncoding::kJapanese)->toHashString(), morph);
            if (hasEnglish) {
                morph->readEnglishName(englishPtr, i);
                name2morphRefs.insert(morph->name(IEncoding::kEnglish)->toHashString(), morph);
            }
            ptr += size;
        }
    }
    void parseLabels(const Model::DataInfo &info) {
        int ncategories = info.boneCategoryNamesCount;
        uint8 *boneCategoryNamesPtr = info.boneCategoryNamesPtr;
        vsize size = 0;
        const uint8 *rootLabelName = reinterpret_cast<const uint8 *>("Root");
        labels.append(new Label(selfRef, encodingRef, rootLabelName, Label::kSpecialBoneCategoryLabel));
        for (int i = 0; i < ncategories; i++) {
            Label *label = labels.append(new Label(selfRef, encodingRef, boneCategoryNamesPtr, Label::kBoneCategoryLabel));
            label->readEnglishName(info.englishBoneFramesPtr, i);
            boneCategoryNamesPtr += Bone::kCategoryNameSize;
        }
        int nbones = info.boneLabelsCount;
        uint8 *boneLabelsPtr = info.boneLabelsPtr;
        for (int i = 0; i < nbones; i++) {
            if (Label *label = Label::selectCategory(labels, boneLabelsPtr)) {
                label->read(boneLabelsPtr, info, size);
                boneLabelsPtr += size;
            }
        }
        int nmorphs = info.morphLabelsCount;
        uint8 *morphLabelsPtr = info.morphLabelsPtr;
        Label *morphCategory = labels.append(new Label(selfRef, encodingRef, 0, Label::kMorphCategoryLabel));
        for (int i = 0; i < nmorphs; i++) {
            morphCategory->read(morphLabelsPtr, info, size);
            morphLabelsPtr += size;
        }
    }
    void parseCustomToonTextures(const Model::DataInfo &info) {
        static const uint8 kFallbackToonTextureName[] = "toon0.bmp";
        uint8 *ptr = info.customToonTextureNamesPtr;
        IString *path = encodingRef->toString(kFallbackToonTextureName,
                                              sizeof(kFallbackToonTextureName) - 1,
                                              IString::kUTF8);
        textures.insert(path->toHashString(), path);
        customToonTextures.append(path);
        for (int i = 0; i < kMaxCustomToonTextures; i++) {
            path = encodingRef->toString(ptr, IString::kShiftJIS, kCustomToonTextureNameSize);
            textures.insert(path->toHashString(), path);
            customToonTextures.append(path);
            ptr += kCustomToonTextureNameSize;
        }
    }
    void parseRigidBodies(const Model::DataInfo &info) {
        const int nRigidBodies = info.rigidBodiesCount;
        uint8 *ptr = info.rigidBodiesPtr;
        vsize size;
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = rigidBodies.append(new RigidBody(selfRef, encodingRef));
            rigidBody->read(ptr, info, size);
            ptr += size;
        }
    }
    void parseJoints(const Model::DataInfo &info) {
        const int njoints = info.jointsCount;
        uint8 *ptr = info.jointsPtr;
        vsize size;
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
                Bone *rootBoneRef = bones[rootIndex], *effectorBoneRef = bones[targetIndex];
                IKConstraint *constraint = constraints.append(new IKConstraint());
                const Array<int> &jointBoneIndices = rawConstraint->jointBoneIndices;
                const int njoints = jointBoneIndices.count();
                for (int j = 0; j < njoints; j++) {
                    int boneIndex = jointBoneIndices[j];
                    if (internal::checkBound(boneIndex, 0, nbones)) {
                        Bone *jointBoneRef = bones[boneIndex];
                        constraint->jointBoneRefs.append(jointBoneRef);
                    }
                }
                constraint->rootBoneRef = rootBoneRef;
                constraint->effectorBoneRef = effectorBoneRef;
                constraint->niterations = unit.niterations;
                constraint->angleLimit = unit.angle * SIMD_PI;
                rootBoneRef->setTargetBoneRef(effectorBoneRef);
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
    PointerHash<HashString, IString> textures;
    PointerArray<Material> materials;
    PointerArray<Bone> bones;
    PointerArray<RawIKConstraint> rawConstraints;
    PointerArray<Morph> morphs;
    PointerArray<Label> labels;
    PointerArray<RigidBody> rigidBodies;
    PointerArray<Joint> joints;
    PointerArray<IKConstraint> constraints;
    Array<IString *> customToonTextures;
    Array<Bone *> sortedBoneRefs;
    Hash<HashString, IBone *> name2boneRefs;
    Hash<HashString, IMorph *> name2morphRefs;
    Array<PropertyEventListener *> eventRefs;
    DataInfo dataInfo;
    Vector3 position;
    Quaternion rotation;
    Scalar opacity;
    Scalar scaleFactor;
    Vector3 edgeColor;
    Vector3 aabbMax;
    Vector3 aabbMin;
    IVertex::EdgeSizePrecision edgeWidth;
    bool hasEnglish;
    bool visible;
    bool physicsEnabled;
};

const int Model::kNameSize = internal::kPMDModelNameSize;
const int Model::kCommentSize = internal::kPMDModelCommentSize;
const int Model::kCustomToonTextureNameSize = internal::kPMDModelCustomToonTextureSize;
const int Model::kMaxCustomToonTextures = 10;

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

bool Model::preparse(const uint8 *data, vsize size, DataInfo &info)
{
    vsize rest = size;
    if (!data || sizeof(Header) > rest) {
        m_context->dataInfo.error = kInvalidHeaderError;
        return false;
    }
    uint8 *ptr = const_cast<uint8 *>(data);
    Header *header = reinterpret_cast<Header *>(ptr);
    info.encoding = m_context->encodingRef;
    info.basePtr = ptr;
    // Check the signature and version is correct
    if (std::memcmp(header->signature, "Pmd", sizeof(header->signature)) != 0) {
        m_context->dataInfo.error = kInvalidSignatureError;
        return false;
    }
    if (header->version != 1.0f) {
        m_context->dataInfo.error = kInvalidVersionError;
        return false;
    }
    // Name and Comment (in Shift-JIS)
    info.namePtr = header->name;
    info.commentPtr = header->comment;
    internal::drainBytes(sizeof(*header), ptr, rest);
    // Vertex
    if (!Vertex::preparse(ptr, rest, info)) {
        info.error = kInvalidVerticesError;
        return false;
    }
    // Index
    int nindices;
    vsize indexSize = sizeof(uint16);
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
    uint8 hasEnglish;
    if (!internal::getTyped<uint8>(ptr, rest, hasEnglish)) {
        info.error = kInvalidEnglishNameSizeError;
        return false;
    }
    m_context->hasEnglish = hasEnglish != 0;
    if (m_context->hasEnglish) {
        const vsize boneNameSize = Bone::kNameSize * info.bonesCount;
        const vsize morphNameSize =  Morph::kNameSize * info.morphLabelsCount;
        const vsize boneCategoryNameSize = Bone::kCategoryNameSize * info.boneCategoryNamesCount;
        const vsize required = kNameSize + kCommentSize + boneNameSize + morphNameSize + boneCategoryNameSize;
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
    vsize customToonTextureNameSize = kMaxCustomToonTextures * kCustomToonTextureNameSize;
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

bool Model::load(const uint8 *data, vsize size)
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

void Model::save(uint8 *data, vsize &written) const
{
    Header header;
    header.version = 1.0;
    uint8 *base = data;
    internal::copyBytes(header.signature, "Pmd", sizeof(header.signature));
    uint8 *namePtr = header.name, *commentPtr = header.comment;
    internal::writeStringAsByteArray(m_context->namePtr, IString::kShiftJIS, m_context->encodingRef, sizeof(header.name), namePtr);
    internal::writeStringAsByteArray(m_context->commentPtr, IString::kShiftJIS, m_context->encodingRef, sizeof(header.comment), commentPtr);
    internal::writeBytes(&header, sizeof(header), data);
    Vertex::writeVertices(m_context->vertices, m_context->dataInfo, data);
    const int32 nindices = m_context->indices.count();
    internal::writeBytes(&nindices, sizeof(nindices), data);
    for (int32 i = 0; i < nindices; i++) {
        int index = m_context->indices[i];
        internal::writeUnsignedIndex(index, sizeof(uint16), data);
    }
    Material::writeMaterials(m_context->materials, m_context->dataInfo, data);
    Bone::writeBones(m_context->bones, m_context->dataInfo, data);
    const int nconstraints = m_context->rawConstraints.count();
    internal::writeUnsignedIndex(nconstraints, sizeof(uint16), data);
    for (int i = 0; i < nconstraints; i++) {
        const RawIKConstraint *constraint = m_context->rawConstraints[i];
        internal::writeBytes(&constraint->unit, sizeof(constraint->unit), data);
        const Array<int> &jointBoneIndices = constraint->jointBoneIndices;
        const int nbonesInIK = jointBoneIndices.count();
        for (int j = 0; j < nbonesInIK; j++) {
            int boneIndex = jointBoneIndices[j];
            internal::writeSignedIndex(boneIndex, sizeof(uint16), data);
        }
    }
    Morph::writeMorphs(m_context->morphs, m_context->dataInfo, data);
    Label::writeLabels(m_context->labels, m_context->dataInfo, data);
    internal::writeSignedIndex(m_context->hasEnglish ? 1 : 0, sizeof(uint8), data);
    if (m_context->hasEnglish) {
        internal::writeStringAsByteArray(m_context->englishNamePtr, IString::kShiftJIS, m_context->encodingRef, kNameSize, data);
        internal::writeStringAsByteArray(m_context->englishCommentPtr, IString::kShiftJIS, m_context->encodingRef, kCommentSize, data);
        Bone::writeEnglishNames(m_context->bones, m_context->dataInfo, data);
        Morph::writeEnglishNames(m_context->morphs, m_context->dataInfo, data);
        Label::writeEnglishNames(m_context->labels, m_context->dataInfo, data);
    }
    uint8 customTextureName[internal::kPMDModelCustomToonTextureSize], *customTextureNamePtr = customTextureName;
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

vsize Model::estimateSize() const
{
    vsize size = 0;
    size += sizeof(Header);
    size += Vertex::estimateTotalSize(m_context->vertices, m_context->dataInfo);
    size += sizeof(int32) + m_context->indices.count() * sizeof(uint16);
    size += Material::estimateTotalSize(m_context->materials, m_context->dataInfo);
    size += Bone::estimateTotalSize(m_context->bones, m_context->dataInfo);
    const uint16 nconstraints = m_context->rawConstraints.count();
    size += sizeof(nconstraints);
    for (int i = 0; i < nconstraints; i++) {
        const RawIKConstraint *constraint = m_context->rawConstraints[i];
        size += sizeof(constraint->unit);
        size += sizeof(uint16) * constraint->jointBoneIndices.count();
    }
    size += Morph::estimateTotalSize(m_context->morphs, m_context->dataInfo);
    size += Label::estimateTotalSize(m_context->labels, m_context->dataInfo);
    size += sizeof(uint8);
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
    if (m_context->physicsEnabled) {
        const int nbones = m_context->sortedBoneRefs.count();
        for (int i = 0; i < nbones; i++) {
            Bone *bone = m_context->sortedBoneRefs[i];
            bone->performTransform();
        }
        btOverlappingPairCache *cache = worldRef->getPairCache();
        btDispatcher *dispatcher = worldRef->getDispatcher();
        const int nRigidBodies = m_context->rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_context->rigidBodies[i];
            if (cache) {
                btRigidBody *body = rigidBody->body();
                btBroadphaseProxy *proxy = body->getBroadphaseHandle();
                cache->cleanProxyFromPairs(proxy, dispatcher);
            }
            rigidBody->setActivation(true);
        }
        const int njoints = m_context->joints.count();
        for (int i = 0; i < njoints; i++) {
            Joint *joint = m_context->joints[i];
            joint->updateTransform();
        }
    }
}

void Model::solveInverseKinematics()
{
    const Array<IKConstraint *> &constraints = m_context->constraints;
    const int nconstraints = constraints.count();
    Quaternion newRotation(Quaternion::getIdentity());
    Matrix3x3 matrix(Matrix3x3::getIdentity());
    Vector3 localRootBonePosition(kZeroV3), localEffectorPosition(kZeroV3),
            rotationEuler(kZeroV3), jointEuler(kZeroV3), localAxis(kZeroV3);
    for (int i = 0; i < nconstraints; i++) {
        IKConstraint *constraint = constraints[i];
        const Array<Bone *> &jointBoneRefs = constraint->jointBoneRefs;
        const int njoints = jointBoneRefs.count();
        Bone *effectorBoneRef = constraint->effectorBoneRef;
        const Quaternion &originTargetBoneRotation = effectorBoneRef->localOrientation();
        const Vector3 &rootBonePosition = constraint->rootBoneRef->worldTransform().getOrigin();
        const Scalar &angleLimit = constraint->angleLimit;
        const int niterations = constraint->niterations;
        effectorBoneRef->performTransform();
        for (int j = 0; j < niterations; j++) {
            for (int k = 0; k < njoints; k++) {
                Bone *jointBoneRef = jointBoneRefs[k];
                const Vector3 &currentEffectorPosition = effectorBoneRef->worldTransform().getOrigin();
                const Transform &jointBoneTransform = jointBoneRef->worldTransform().inverse();
                localRootBonePosition = jointBoneTransform * rootBonePosition;
                localEffectorPosition = jointBoneTransform * currentEffectorPosition;
                localRootBonePosition.safeNormalize();
                localEffectorPosition.safeNormalize();
                const Scalar &dot = localRootBonePosition.dot(localEffectorPosition);
                if (btFuzzyZero(dot)) {
                    break;
                }
                const Scalar &newAngleLimit = angleLimit * (k + 1) * 2;
                Scalar angle = btAcos(dot);
                btClamp(angle, -newAngleLimit, newAngleLimit);
                localAxis = localEffectorPosition.cross(localRootBonePosition);
                localAxis.safeNormalize();
                if (jointBoneRef->isAxisXAligned()) {
                    if (j == 0) {
                        newRotation.setRotation(kAxisX, btFabs(angle));
                    }
                    else {
                        newRotation.setRotation(localAxis, angle);
                        matrix.setRotation(newRotation);
                        matrix.getEulerZYX(rotationEuler[2], rotationEuler[1], rotationEuler[0]);
                        matrix.setRotation(jointBoneRef->localOrientation());
                        matrix.getEulerZYX(jointEuler[2], jointEuler[1], jointEuler[0]);
                        Scalar x = rotationEuler.x(), ex = jointEuler.x();
                        if (x + ex > SIMD_PI) {
                            x = SIMD_PI - ex;
                        }
                        if (kMinRotationSum > x + ex) {
                            x = kMinRotationSum - ex;
                        }
                        btClamp(x, -angleLimit, angleLimit);
                        if (btFabs(x) < kMinRotation) {
                            continue;
                        }
                        newRotation.setEulerZYX(0.0f, 0.0f, x);
                    }
                    const Quaternion &localRotation = newRotation * jointBoneRef->localOrientation();
                    jointBoneRef->setLocalOrientation(localRotation);
                }
                else {
                    newRotation.setRotation(localAxis, angle);
                    const Quaternion &localRotation = jointBoneRef->localOrientation() * newRotation;
                    jointBoneRef->setLocalOrientation(localRotation);
                }
                for (int l = k; l >= 0; l--) {
                    Bone *jointBoneRef = jointBoneRefs[l];
                    jointBoneRef->performTransform();
                }
                effectorBoneRef->performTransform();
            }
        }
        effectorBoneRef->setLocalOrientation(originTargetBoneRotation);
        effectorBoneRef->performTransform();
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
            rigidBody->joinWorld(worldRef);
        }
        const int njoints = m_context->joints.count();
        for (int i = 0; i < njoints; i++) {
            Joint *joint = m_context->joints[i];
            joint->joinWorld(worldRef);
        }
    }
}

void Model::leaveWorld(btDiscreteDynamicsWorld *worldRef)
{
    if (worldRef) {
        const int nRigidBodies = m_context->rigidBodies.count();
        for (int i = nRigidBodies - 1; i >= 0; i--) {
            RigidBody *rigidBody = m_context->rigidBodies[i];
            rigidBody->leaveWorld(worldRef);
        }
        const int njoints = m_context->joints.count();
        for (int i = njoints - 1; i >= 0; i--) {
            Joint *joint = m_context->joints[i];
            joint->leaveWorld(worldRef);
        }
    }
}

IBone *Model::findBoneRef(const IString *value) const
{
    if (value) {
        const HashString &key = value->toHashString();
        IBone **bone = const_cast<IBone **>(m_context->name2boneRefs.find(key));
        return bone ? *bone : 0;
    }
    return 0;
}

IMorph *Model::findMorphRef(const IString *value) const
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
    case kSoftBody:
        return 0;
    case kTexture:
        return m_context->textures.count();
    case kVertex:
        return m_context->vertices.count();
    case kMaxObjectType:
    default:
        return 0;
    }
    return 0;
}

void Model::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
        m_context->eventRefs.append(value);
    }
}

void Model::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
    }
}

void Model::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_context->eventRefs);
}

IModel::Type Model::type() const
{
    return kPMDModel;
}

const IString *Model::name(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->namePtr;
    case IEncoding::kEnglish:
        return m_context->englishNamePtr;
    default:
        return 0;
    }
}

const IString *Model::comment(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->commentPtr;
    case IEncoding::kEnglish:
        return m_context->englishCommentPtr;
    default:
        return 0;
    }
}

bool Model::isVisible() const
{
    return m_context->visible && !btFuzzyZero(m_context->opacity);
}

Vector3 Model::worldTranslation() const
{
    return m_context->position;
}

Quaternion Model::worldOrientation() const
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

IVertex::EdgeSizePrecision Model::edgeWidth() const
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

void Model::getTextureRefs(Array<const IString *> &value) const
{
    internal::ModelHelper::getTextureRefs(m_context->textures, value);
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

void Model::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->namePtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->namePtr);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishNamePtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->englishNamePtr);
        }
        break;
    default:
        break;
    }
}

void Model::setComment(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->commentPtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, commentWillChange(value, type, this));
            internal::setString(value, m_context->commentPtr);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishCommentPtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, commentWillChange(value, type, this));
            internal::setString(value, m_context->englishCommentPtr);
        }
        break;
    default:
        break;
    }
}

void Model::setWorldTranslation(const Vector3 &value)
{
    if (m_context->position != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, worldTranslationWillChange(value, this));
        m_context->position = value;
    }
}

void Model::setWorldOrientation(const Quaternion &value)
{
    if (m_context->rotation != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, worldOrientationWillChange(value, this));
        m_context->rotation = value;
    }
}

void Model::setOpacity(const Scalar &value)
{
    if (m_context->opacity != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, opacityWillChange(value, this));
        m_context->opacity = value;
    }
}

void Model::setScaleFactor(const Scalar &value)
{
    if (m_context->scaleFactor != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, scaleFactorWillChange(value, this));
        m_context->scaleFactor = value;
    }
}

void Model::setEdgeColor(const Vector3 &value)
{
    if (m_context->edgeColor != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, edgeColorWillChange(value, this));
        m_context->edgeColor = value;
    }
}

void Model::setEdgeWidth(const IVertex::EdgeSizePrecision &value)
{
    if (m_context->edgeWidth != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, edgeWidthWillChange(value, this));
        m_context->edgeWidth = value;
    }
}

void Model::setParentSceneRef(Scene *value)
{
    m_context->sceneRef = value;
}

void Model::setParentModelRef(IModel *value)
{
    if (m_context->parentModelRef != value && !internal::ModelHelper::hasModelLoopChain(value, this)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, parentModelRefWillChange(value, this));
        m_context->parentModelRef = value;
    }
}

void Model::setParentBoneRef(IBone *value)
{
    if (m_context->parentBoneRef != value && !internal::ModelHelper::hasBoneLoopChain(value, m_context->parentModelRef)) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, parentBoneRefWillChange(value, this));
        m_context->parentBoneRef = value;
    }
}

void Model::setPhysicsEnable(bool value)
{
    if (m_context->physicsEnabled != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, physicsEnableWillChange(value, this));
        m_context->physicsEnabled = value;
    }
}

void Model::setVisible(bool value)
{
    if (m_context->visible != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, visibleWillChange(value, this));
        m_context->visible = value;
    }
}

void Model::getAabb(Vector3 &min, Vector3 &max) const
{
    min = m_context->aabbMin;
    max = m_context->aabbMax;
}

void Model::setAabb(const Vector3 &min, const Vector3 &max)
{
    if (m_context->aabbMin != min || m_context->aabbMax != max) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, aabbWillChange(min, max, this));
        m_context->aabbMin = min;
        m_context->aabbMax = max;
    }
}

float32 Model::version() const
{
    return 1.0f;
}

void Model::setVersion(float32 /* value */)
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
    return new Label(this, m_context->encodingRef, reinterpret_cast<const uint8 *>(""), Label::kBoneCategoryLabel);
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

IBone *Model::findBoneRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Bone, IBone>(m_context->bones, value);
}

IJoint *Model::findJointRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Joint, IJoint>(m_context->joints, value);
}

ILabel *Model::findLabelRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Label, ILabel>(m_context->labels, value);
}

IMaterial *Model::findMaterialRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Material, IMaterial>(m_context->materials, value);
}

IMorph *Model::findMorphRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<Morph, IMorph>(m_context->morphs, value);
}

IRigidBody *Model::findRigidBodyRefAt(int value) const
{
    return internal::ModelHelper::findObjectAt<RigidBody, IRigidBody>(m_context->rigidBodies, value);
}

IVertex *Model::findVertexRefAt(int value) const
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

void Model::addTexture(const IString *value)
{
    if (value) {
        const HashString &key = value->toHashString();
        if (!m_context->textures.find(key)) {
            m_context->textures.insert(key, value->clone());
        }
    }
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

} /* namespace pmd2 */
} /* namespace vpvl2 */
