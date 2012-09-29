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

namespace
{

using namespace vpvl2::pmd;

#pragma pack(push, 1)

struct Header
{
    uint8_t signature[3];
    float version;
    uint8_t name[Model::kNameSize];
    uint8_t comment[Model::kCommentSize];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd
{

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
      m_enableSkinning(true)
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
    if (!Bone::preparseIKJoints(ptr, rest, info)) {
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
        parseMorphs(info);
        parseLabels(info);
        parseCustomToonTextures(info);
        parseRigidBodies(info);
        parseJoints(info);
        if (!Bone::loadBones(m_bones)
                || !Material::loadMaterials(m_materials, m_customToonTextures, m_indices.count())
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
    return 0;
}

void Model::resetVertices()
{
}

void Model::performUpdate(const Vector3 &/*cameraPosition*/, const Vector3 &/*lightDirection*/)
{
}

void Model::joinWorld(btDiscreteDynamicsWorld * /* world */)
{
}

void Model::leaveWorld(btDiscreteDynamicsWorld * /* world */)
{
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

void Model::getBones(Array<IBone *> &value) const
{
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        IBone *bone = m_bones[i];
        value.add(bone);
    }
}

void Model::getLabels(Array<ILabel *> &value) const
{
    const int nlabels = m_labels.count();
    for (int i = 0; i < nlabels; i++) {
        ILabel *label = m_labels[i];
        value.add(label);
    }
}

void Model::getMorphs(Array<IMorph *> &value) const
{
    const int nmorphs = m_morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        IMorph *morph = m_morphs[i];
        value.add(morph);
    }
}

void Model::getBoundingBox(Vector3 &min, Vector3 &max) const
{
    min.setZero();
    max.setZero();
    /*
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
    */
}

void Model::getBoundingSphere(Vector3 &center, Scalar &radius) const
{
    center.setZero();
    radius = 0;
    /*
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
    */
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

void Model::getSkinningMeshes(SkinningMeshes &/*meshes*/) const
{
    /*
    const vpvl::MaterialList &materials = m_model.materials();
    const vpvl::VertexList &vertices = m_model.vertices();
    const vpvl::IndexList &vertexIndices = m_model.indices();
    const int nmaterials = materials.count();
    btHashMap<btHashInt, int> set;
    BoneIndices boneIndices;
    meshes.transforms.resize(m_model.bones().count());
    int offset = 0;
    size_t stride = m_model.strideSize(vpvl::PMDModel::kVerticesStride);
    uint8_t *ptr = static_cast<uint8_t *>(const_cast<void *>(m_model.boneAttributesPointer()));
    for (int i = 0; i < nmaterials; i++) {
        const vpvl::Material *material = materials[i];
        const int nindices = material->countIndices();
        int boneIndexInteral = 0;
        for (int j = 0; j < nindices; j++) {
            const int vertexIndex = vertexIndices[offset + j];
            const vpvl::Vertex *vertex = vertices[vertexIndex];
            const int boneIndex1 = vertex->bone1();
            int *normalizedBoneIndex1Ptr = set.find(boneIndex1), normalizedBoneIndex1;
            if (!normalizedBoneIndex1Ptr) {
                normalizedBoneIndex1 = boneIndexInteral++;
                set.insert(boneIndex1, normalizedBoneIndex1);
                boneIndices.push_back(boneIndex1);
            }
            else {
                normalizedBoneIndex1 = *normalizedBoneIndex1Ptr;
            }
            const int boneIndex2 = vertex->bone2();
            int *normalizedBoneIndex2Ptr = set.find(boneIndex2), normalizedBoneIndex2;
            if (!normalizedBoneIndex2Ptr) {
                normalizedBoneIndex2 = boneIndexInteral++;
                set.insert(boneIndex2, normalizedBoneIndex2);
                boneIndices.push_back(boneIndex2);
            }
            else {
                normalizedBoneIndex2 = *normalizedBoneIndex2Ptr;
            }
            Vector3 *v = reinterpret_cast<Vector3 *>(ptr + vertexIndex * stride);
            v->setValue(Scalar(normalizedBoneIndex1), Scalar(normalizedBoneIndex2), vertex->weight());
        }
        meshes.matrices.add(new Scalar[boneIndices.size() * 16]);
        meshes.bones.push_back(boneIndices);
        boneIndices.clear();
        set.clear();
        offset += nindices;
    }
    */
}

void Model::updateSkinningMeshes(SkinningMeshes &/*meshes*/) const
{
    /*
    const vpvl::BoneList &bones = m_model.bones();
    const int nbones = bones.count();
    MeshLocalTransforms &transforms = meshes.transforms;
    for (int i = 0; i < nbones; i++) {
        const vpvl::Bone *bone = bones[i];
        bone->getSkinTransform(transforms[i]);
    }
    const int nmaterials = m_model.materials().count();
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
    */
}

void Model::setSkinnningEnable(bool value)
{
    m_enableSkinning = value;
}

void Model::release()
{
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
    m_opacity = 0;
    m_scaleFactor = 0;
    m_edgeColor.setZero();
    m_edgeWidth = 0;
    m_enableSkinning = false;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    internal::setStringDirect(m_encodingRef->toString(info.namePtr, kNameSize, IString::kShiftJIS), m_name);
    internal::setStringDirect(m_encodingRef->toString(info.englishNamePtr, kNameSize, IString::kShiftJIS), m_englishName);
    internal::setStringDirect(m_encodingRef->toString(info.commentPtr, kCommentSize, IString::kShiftJIS), m_comment);
    internal::setStringDirect(m_encodingRef->toString(info.englishCommentPtr, kCommentSize, IString::kShiftJIS), m_englishComment);
}

void Model::parseVertices(const DataInfo &info)
{
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.verticesPtr;
    size_t size;
    for(int i = 0; i < nvertices; i++) {
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
    for(int i = 0; i < nindices; i++) {
        uint16_t index = internal::readUnsignedIndex(ptr, 2);
        m_indices.add(index);
    }
}

void Model::parseMaterials(const DataInfo &info)
{
    const int nmaterials = info.materialsCount;
    uint8_t *ptr = info.materialsPtr;
    size_t size;
    for(int i = 0; i < nmaterials; i++) {
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
    for(int i = 0; i < nbones; i++) {
        Bone *bone = new Bone(m_encodingRef);
        m_bones.add(bone);
        bone->readBone(ptr, info, size);
        ptr += size;
    }
}

void Model::parseIKJoints(const DataInfo &info)
{
    const int njoints = info.IKJointsCount;
    uint8_t *ptr = info.IKJointsPtr;
    size_t size;
    for(int i = 0; i < njoints; i++) {
        Bone::readIKJoints(ptr, m_bones, size);
        ptr += size;
    }
}

void Model::parseMorphs(const DataInfo &info)
{
    const int nmorphs = info.morphsCount;
    uint8_t *ptr = info.morphsPtr;
    size_t size;
    for(int i = 0; i < nmorphs; i++) {
        Morph *morph = new Morph(m_encodingRef);
        m_morphs.add(morph);
        morph->read(ptr, m_vertices, size);
        m_name2morphRefs.insert(morph->name()->toHashString(), morph);
        ptr += size;
    }
}

void Model::parseLabels(const DataInfo &info)
{
}

void Model::parseCustomToonTextures(const DataInfo &info)
{
    uint8_t *ptr = info.customToonTextureNamesPtr;
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
    for(int i = 0; i < njoints; i++) {
        Joint *joint = new Joint(m_encodingRef);
        m_joints.add(joint);
        joint->read(ptr, info, size);
        ptr += size;
    }
}

}
}
