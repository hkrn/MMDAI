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
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmd/Morph.h"
#include "vpvl2/pmd/Vertex.h"

namespace vpvl2
{
namespace pmd
{

#pragma pack(push, 1)

struct Header
{
    uint8_t signature[3];
    float version;
    uint8_t name[Model::kNameSize];
    uint8_t comment[Model::kCommentSize];
};

#pragma pack(pop)

Model::Model(IEncoding *encoding)
    : m_encodingRef(encoding),
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

bool Model::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    // Header[3] + Version[4] + Name[20] + Comment[256]
    if (!data || sizeof(Header) > rest) {
        m_info.error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header *header = reinterpret_cast<Header *>(ptr);
    info.encoding = m_encodingRef;
    info.basePtr = ptr;

    // Check the signature and version is correct
    if (memcmp(header->signature, "Pmd", 3) != 0) {
        m_info.error = kInvalidSignatureError;
        return false;
    }
    if (1.0f != header->version) {
        m_info.error = kInvalidVersionError;
        return false;
    }

    // Name and Comment (in Shift-JIS)
    info.namePtr = header->name;
    info.commentPtr = header->comment;
    ptr += sizeof(Header);
    rest -= sizeof(Header);

    /*
    size_t nVertices = 0, nIndices = 0, nMaterials = 0, nBones = 0, nIKs = 0, nFaces = 0,
            nFaceNames = 0, nBoneFrames = 0, nBoneNames = 0, nRigidBodies = 0, nConstranits = 0;
    // Vertices
    if (!internal::size32(ptr, rest, nVertices)) {
        m_info.error = kInvalidVerticesError;
        return false;
    }
    info.verticesPtr = ptr;
    if (!internal::validateSize(ptr, Vertex::stride(), nVertices, rest)) {
        m_info.error = kInvalidVerticesError;
        return false;
    }
    info.verticesCount = nVertices;

    // Indices
    if (!internal::size32(ptr, rest, nIndices)) {
        m_info.error = kInvalidIndicesError;
        return false;
    }
    info.indicesPtr = ptr;
    if (!internal::validateSize(ptr, sizeof(uint16_t), nIndices, rest)) {
        m_info.error = kInvalidIndicesError;
        return false;
    }
    info.indicesCount = nIndices;

    // Materials
    if (!internal::size32(ptr, rest, nMaterials)) {
        m_info.error = kInvalidMaterialsError;
        return false;
    }
    info.materialsPtr = ptr;
    if (!internal::validateSize(ptr, Material::stride(), nMaterials, rest)) {
        m_info.error = kInvalidMaterialsError;
        return false;
    }
    info.materialsCount = nMaterials;

    // Bones
    if (!internal::size16(ptr, rest, nBones)) {
        m_info.error = kInvalidBonesError;
        return false;
    }
    info.bonesPtr = ptr;
    if (!internal::validateSize(ptr, Bone::stride(), nBones, rest)) {
        m_info.error = kInvalidBonesError;
        return false;
    }
    if (nBones == 0) {
        m_info.error = kInvalidBonesError;
        return false;
    }
    info.bonesCount = nBones;

    // IKs
    if (!internal::size16(ptr, rest, nIKs)) {
        m_info.error = kInvalidBonesError;
        return false;
    }
    info.IKBonesPtr = ptr;

    bool ok = false;
    size_t s = IK::totalSize(ptr, rest, nIKs, ok);
    if (!ok || !internal::validateSize(ptr, s, 1, rest)) {
        m_info.error = kInvalidBonesError;
        return false;
    }
    info.IKBonesCount = nIKs;

    // Faces
    if (!internal::size16(ptr, rest, nFaces)) {
        m_info.error = kInvalidMorphsError;
        return false;
    }
    info.morphsPtr = ptr;

    ok = false;
    s = Face::totalSize(ptr, rest, nFaces, ok);
    if (!ok || !internal::validateSize(ptr, s, 1, rest)) {
        m_info.error = kInvalidMorphsError;
        return false;
    }
    info.morphsCount = nFaces;

    // Face display names
    if (!internal::size8(ptr, rest, nFaceNames)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    info.morphLabelsPtr = ptr;
    if (!internal::validateSize(ptr, sizeof(uint16_t), nFaceNames, rest)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    info.morphLabelsCount = nFaceNames;

    // Bone frame names
    if (!internal::size8(ptr, rest, nBoneFrames)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    info.boneCategoryNamesPtr = ptr;
    if (!internal::validateSize(ptr, kBoneCategoryNameSize, nBoneFrames, rest)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    info.boneCategoryNamesCount = nBoneFrames;

    // Bone display names
    if (!internal::size32(ptr, rest, nBoneNames)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    info.boneLabelsPtr = ptr;
    if (!internal::validateSize(ptr, sizeof(uint16_t) + sizeof(uint8_t), nBoneNames, rest)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }
    info.boneLabelsCount = nBoneNames;

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
        const size_t required = kNameSize + kCommentSize
                + englishBoneNamesSize + englishFaceNamesSize + englishBoneCategoryNameSize;
        if (required > rest) {
            m_info.error = kInvalidEnglishNameSizeError;
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
        m_info.error = kInvalidTextureSizeError;
        return false;
    }
    info.toonTextureNamesPtr = ptr;
    ptr += customTextureNameSize;
    rest -= customTextureNameSize;

    if (rest == 0)
        return true;

    // Rigid body
    if (!internal::size32(ptr, rest, nRigidBodies)) {
        m_info.error = kInvalidRigidBodiesError;
        return false;
    }
    info.rigidBodiesPtr = ptr;
    if (!internal::validateSize(ptr, RigidBody::stride(), nRigidBodies, rest)) {
        m_info.error = kInvalidRigidBodiesError;
        return false;
    }
    info.rigidBodiesCount = nRigidBodies;

    // Constranint
    if (!internal::size32(ptr, rest, nConstranits)) {
        m_info.error = kInvalidJointsError;
        return false;
    }
    info.constraintsPtr = ptr;
    if (!internal::validateSize(ptr, Constraint::stride(), nConstranits, rest)) {
        m_info.error = kInvalidJointsError;
        return false;
    }
    info.constranitsCount = nConstranits;
    */

    return rest == 0;
}

bool Model::load(const uint8_t *data, size_t size)
{
    bool ret = false;
    return ret;
}

void Model::save(uint8_t *data) const
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

void Model::performUpdate(const Vector3 &cameraPosition, const Vector3 &lightDirection)
{
}

void Model::joinWorld(btDiscreteDynamicsWorld *world)
{
}

void Model::leaveWorld(btDiscreteDynamicsWorld *world)
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
    /*
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

void Model::getSkinningMeshes(SkinningMeshes &meshes) const
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

void Model::updateSkinningMeshes(SkinningMeshes &meshes) const
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

}
}
