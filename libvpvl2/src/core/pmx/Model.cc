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

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

namespace
{

#pragma pack(push, 1)

    struct Header
    {
        uint8_t signature[4];
        float version;
    };

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

struct Model::SkinnedVertex {
    SkinnedVertex() {}
    Vector3 position;
    Vector3 normal;
    Vector3 texcoord;
    Vector4 uva0;
    Vector4 uva1;
    Vector4 uva2;
    Vector4 uva3;
    Vector4 uva4;
};

Model::Model(IEncoding *encoding)
    : m_world(0),
      m_encoding(encoding),
      m_skinnedVertices(0),
      m_skinnedIndices(0),
      m_name(0),
      m_englishName(0),
      m_comment(0),
      m_englishComment(0),
      m_userData(0),
      m_error(kNoError),
      m_visible(false)
{
}

Model::~Model()
{
    release();
}

size_t Model::strideOffset(StrideType type)
{
    const SkinnedVertex v;
    switch (type) {
    case kVertexStride:
        return reinterpret_cast<const uint8_t *>(&v.position) - reinterpret_cast<const uint8_t *>(&v.position);
    case kNormalStride:
        return reinterpret_cast<const uint8_t *>(&v.normal) - reinterpret_cast<const uint8_t *>(&v.position);
    case kTexCoordStride:
        return reinterpret_cast<const uint8_t *>(&v.texcoord) - reinterpret_cast<const uint8_t *>(&v.position);
    case kUVA0Stride:
        return reinterpret_cast<const uint8_t *>(&v.uva0) - reinterpret_cast<const uint8_t *>(&v.position);
    case kUVA1Stride:
        return reinterpret_cast<const uint8_t *>(&v.uva1) - reinterpret_cast<const uint8_t *>(&v.position);
    case kUVA2Stride:
        return reinterpret_cast<const uint8_t *>(&v.uva2) - reinterpret_cast<const uint8_t *>(&v.position);
    case kUVA3Stride:
        return reinterpret_cast<const uint8_t *>(&v.uva3) - reinterpret_cast<const uint8_t *>(&v.position);
    case kUVA4Stride:
        return reinterpret_cast<const uint8_t *>(&v.uva4) - reinterpret_cast<const uint8_t *>(&v.position);
    default:
        return 0;
    }
}

size_t Model::strideSize(StrideType type)
{
    switch (type) {
    case kVertexStride:
    case kNormalStride:
    case kTexCoordStride:
    case kUVA0Stride:
    case kUVA1Stride:
    case kUVA2Stride:
    case kUVA3Stride:
    case kUVA4Stride:
        return sizeof(SkinnedVertex);
    case kIndexStride:
        return sizeof(uint32_t);
    default:
        return 0;
    }
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
        if (!Bone::loadBones(m_bones, m_BPSOrderedBones, m_APSOrderedBones)
                || !Material::loadMaterials(m_materials, m_textures, m_indices.count())
                || !Vertex::loadVertices(m_vertices, m_bones)
                || !Morph::loadMorphs(m_morphs, m_bones, m_materials, m_vertices)
                || !RigidBody::loadRigidBodies(m_rigidBodies, m_bones)
                || !Joint::loadJoints(m_joints, m_rigidBodies))
            return false;
        return true;
    }
    return false;
}

void Model::save(uint8_t * /* data */) const
{
}

void Model::resetVertices()
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertices[i];
        vertex->reset();
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
        rigidBody->setKinematic(false);
        world->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->collisionGroupMask());
    }
    const int njoints = m_joints.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = m_joints[i];
        world->addConstraint(joint->constraint());
    }
    m_world = world;
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
        rigidBody->setKinematic(true);
        world->removeCollisionObject(rigidBody->body());
    }
    const int njoints = m_joints.count();
    for (int i = njoints - 1; i >= 0; i--) {
        Joint *joint = m_joints[i];
        world->removeConstraint(joint->constraint());
    }
    m_world = 0;
#endif /* VPVL2_NO_BULLET */
}

IBone *Model::findBone(const IString *value) const
{
    IBone **bone = const_cast<IBone **>(m_name2bones.find(value->toHashString()));
    return bone ? *bone : 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    IMorph **morph = const_cast<IMorph **>(m_name2morphs.find(value->toHashString()));
    return morph ? *morph : 0;
}

bool Model::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    if (!data || sizeof(Header) > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }
    /* header */
    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header *header = reinterpret_cast<Header *>(ptr);
    info.basePtr = ptr;

    /* Check the signature and version is correct */
    if (memcmp(header->signature, "PMX ", 4) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }

    /* version */
    if (header->version != 2.0) {
        m_error = kInvalidVersionError;
        return false;
    }

    /* flags */
    size_t flagSize;
    internal::readBytes(sizeof(Header), ptr, rest);
    if (!internal::size8(ptr, rest, flagSize) || flagSize != 8) {
        m_error = kInvalidFlagSizeError;
        return false;
    }
    info.codec = *reinterpret_cast<uint8_t *>(ptr) == 1 ? IString::kUTF8 : IString::kUTF16;
    info.additionalUVSize = *reinterpret_cast<uint8_t *>(ptr + 1);
    info.vertexIndexSize = *reinterpret_cast<uint8_t *>(ptr + 2);
    info.textureIndexSize = *reinterpret_cast<uint8_t *>(ptr + 3);
    info.materialIndexSize = *reinterpret_cast<uint8_t *>(ptr + 4);
    info.boneIndexSize = *reinterpret_cast<uint8_t *>(ptr + 5);
    info.morphIndexSize = *reinterpret_cast<uint8_t *>(ptr + 6);
    info.rigidBodyIndexSize = *reinterpret_cast<uint8_t *>(ptr + 7);
    internal::readBytes(flagSize, ptr, rest);

    /* name in Japanese */
    if (!internal::sizeText(ptr, rest, info.namePtr, info.nameSize)) {
        m_error = kInvalidNameSizeError;
        return false;
    }
    /* name in English */
    if (!internal::sizeText(ptr, rest, info.englishNamePtr, info.englishNameSize)) {
        m_error = kInvalidEnglishNameSizeError;
        return false;
    }
    /* comment in Japanese */
    if (!internal::sizeText(ptr, rest, info.commentPtr, info.commentSize)) {
        m_error = kInvalidCommentSizeError;
        return false;
    }
    /* comment in English */
    if (!internal::sizeText(ptr, rest, info.englishCommentPtr, info.englishCommentSize)) {
        m_error = kInvalidEnglishCommentSizeError;
        return false;
    }

    /* vertex */
    if (!Vertex::preparse(ptr, rest, info)) {
        m_error = kInvalidVerticesError;
        return false;
    }

    /* indices */
    size_t nindices;
    if (!internal::size32(ptr, rest, nindices) || nindices * info.vertexIndexSize > rest) {
        m_error = kInvalidIndicesError;
        return false;
    }
    info.indicesPtr = ptr;
    info.indicesCount = nindices;
    internal::readBytes(nindices * info.vertexIndexSize, ptr, rest);

    /* texture lookup table */
    size_t ntextures;
    if (!internal::size32(ptr, rest, ntextures)) {
        m_error = kInvalidTextureSizeError;
        return false;
    }
    info.texturesPtr = ptr;
    for (size_t i = 0; i < ntextures; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
    }
    info.texturesCount = ntextures;

    /* material */
    if (!Material::preparse(ptr, rest, info)) {
        m_error = kInvalidMaterialsError;
        return false;
    }

    /* bone */
    if (!Bone::preparse(ptr, rest, info)) {
        m_error = kInvalidBonesError;
        return false;
    }

    /* morph */
    if (!Morph::preparse(ptr, rest, info)) {
        m_error = kInvalidMorphsError;
        return false;
    }

    /* display name table */
    if (!Label::preparse(ptr, rest, info)) {
        m_error = kInvalidLabelsError;
        return false;
    }

    /* rigid body */
    if (!RigidBody::preparse(ptr, rest, info)) {
        m_error = kInvalidRigidBodiesError;
        return false;
    }

    /* constraint */
    if (!Joint::preparse(ptr, rest, info)) {
        m_error = kInvalidJointsError;
        return false;
    }
    info.endPtr = ptr;
    info.encoding = m_encoding;

    return rest == 0;
}

void Model::setUserData(UserData *value)
{
    m_userData = value;
}

void Model::setVisible(bool value)
{
    m_visible = value;
}

void Model::update()
{
    // update local transform matrix
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        bone->resetIKLink();
    }
    // before physics simulation
    const int nBPSBones = m_BPSOrderedBones.count();
    for (int i = 0; i < nBPSBones; i++) {
        Bone *bone = m_BPSOrderedBones[i];
        bone->performFullTransform();
        bone->performInverseKinematics();
    }
    for (int i = 0; i < nBPSBones; i++) {
        Bone *bone = m_BPSOrderedBones[i];
        bone->performUpdateLocalTransform();
    }
    // physics simulation
    if (m_world) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_rigidBodies[i];
            rigidBody->performTransformBone();
        }
    }
    // after physics simulation
    const int nAPSBones = m_APSOrderedBones.count();
    for (int i = 0; i < nAPSBones; i++) {
        Bone *bone = m_APSOrderedBones[i];
        bone->performFullTransform();
        bone->performInverseKinematics();
    }
    for (int i = 0; i < nAPSBones; i++) {
        Bone *bone = m_APSOrderedBones[i];
        bone->performUpdateLocalTransform();
    }
    // skinning
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertices[i];
        SkinnedVertex &v = m_skinnedVertices[i];
        v.texcoord = vertex->texcoord();
        v.uva0 = vertex->uv(0);
        v.uva1 = vertex->uv(1);
        v.uva2 = vertex->uv(2);
        v.uva3 = vertex->uv(3);
        v.uva4 = vertex->uv(4);
        vertex->performSkinning(v.position, v.normal);
    }
}

const void *Model::vertexPtr() const
{
    return &m_skinnedVertices[0].position;
}

const void *Model::indicesPtr() const
{
    return &m_skinnedIndices[0];
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
    leaveWorld(m_world);
    m_vertices.releaseAll();
    m_textures.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_rigidBodies.releaseAll();
    m_joints.releaseAll();
    delete[] m_skinnedVertices;
    m_skinnedVertices = 0;
    delete[] m_skinnedIndices;
    m_skinnedIndices = 0;
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    delete m_comment;
    m_comment = 0;
    delete m_englishComment;
    m_englishComment = 0;
    m_error = kNoError;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    setName(m_encoding->toString(info.namePtr, info.nameSize, info.codec));
    setEnglishName(m_encoding->toString(info.englishNamePtr, info.englishNameSize, info.codec));
    setComment(m_encoding->toString(info.commentPtr, info.commentSize, info.codec));
    setEnglishComment(m_encoding->toString(info.englishCommentPtr, info.englishCommentSize, info.codec));
}

void Model::parseVertices(const DataInfo &info)
{
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.verticesPtr;
    size_t size;
    delete[] m_skinnedVertices;
    m_skinnedVertices = new SkinnedVertex[nvertices];
    for(int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        vertex->read(ptr, info, size);
        m_vertices.add(vertex);
        ptr += size;
    }
}

void Model::parseIndices(const DataInfo &info)
{
    const int nindices = info.indicesCount;
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.indicesPtr;
    size_t size = info.vertexIndexSize;
    delete[] m_skinnedIndices;
    m_skinnedIndices = new int[nindices];
    for(int i = 0; i < nindices; i++) {
        int index = internal::readUnsignedIndex(ptr, size);
        if (index >= 0 && index < nvertices) {
            m_indices.add(index);
            m_skinnedIndices[i] = index;
        }
        else {
            m_indices.add(0);
            m_skinnedIndices[i] = 0;
        }
    }
#ifdef VPVL2_COORDINATE_OPENGL
    for (int i = 0; i < nindices; i += 3)
        btSwap(m_skinnedIndices[i], m_skinnedIndices[i + 1]);
#endif
}

void Model::parseTextures(const DataInfo &info)
{
    const int ntextures = info.texturesCount;
    uint8_t *ptr = info.texturesPtr;
    uint8_t *texturePtr;
    size_t nTextureSize, rest = SIZE_MAX;
    for(int i = 0; i < ntextures; i++) {
        internal::sizeText(ptr, rest, texturePtr, nTextureSize);
        m_textures.add(m_encoding->toString(texturePtr, nTextureSize, info.codec));
    }
}

void Model::parseMaterials(const DataInfo &info)
{
    const int nmaterials = info.materialsCount;
    uint8_t *ptr = info.materialsPtr;
    size_t size;
    for(int i = 0; i < nmaterials; i++) {
        Material *material = new Material();
        material->read(ptr, info, size);
        m_materials.add(material);
        ptr += size;
    }
}

void Model::parseBones(const DataInfo &info)
{
    const int nbones = info.bonesCount;
    uint8_t *ptr = info.bonesPtr;
    size_t size;
    for(int i = 0; i < nbones; i++) {
        Bone *bone = new Bone();
        bone->read(ptr, info, size);
        bone->performTransform();
        bone->performUpdateLocalTransform();
        m_bones.add(bone);
        m_name2bones.insert(bone->name()->toHashString(), bone);
        m_name2bones.insert(bone->englishName()->toHashString(), bone);
        ptr += size;
    }
}

void Model::parseMorphs(const DataInfo &info)
{
    const int nmorphs = info.morphsCount;
    uint8_t *ptr = info.morphsPtr;
    size_t size;
    for(int i = 0; i < nmorphs; i++) {
        Morph *morph = new Morph();
        morph->read(ptr, info, size);
        m_morphs.add(morph);
        m_name2morphs.insert(morph->name()->toHashString(), morph);
        m_name2morphs.insert(morph->englishName()->toHashString(), morph);
        ptr += size;
    }
}

void Model::parseLabels(const DataInfo &info)
{
    const int nlabels = info.displayNamesCount;
    uint8_t *ptr = info.displayNamesPtr;
    size_t size;
    for(int i = 0; i < nlabels; i++) {
        Label *label = new Label();
        label->read(ptr, info, size);
        m_labels.add(label);
        ptr += size;
    }
}

void Model::parseRigidBodies(const DataInfo &info)
{
    const int nRigidBodies = info.rigidBodiesCount;
    uint8_t *ptr = info.rigidBodiesPtr;
    size_t size;
    for(int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody();
        rigidBody->read(ptr, info, size);
        m_rigidBodies.add(rigidBody);
        ptr += size;
    }
}

void Model::parseJoints(const DataInfo &info)
{
    const int nJoints = info.jointsCount;
    uint8_t *ptr = info.jointsPtr;
    size_t size;
    for(int i = 0; i < nJoints; i++) {
        Joint *joint = new Joint();
        joint->read(ptr, info, size);
        m_joints.add(joint);
        ptr += size;
    }
}

}
}
