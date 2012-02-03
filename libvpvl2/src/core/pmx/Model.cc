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

#ifndef vpvl2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
vpvl2_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

namespace vpvl2
{
namespace pmx
{

#pragma pack(push, 1)
struct Header
{
    uint8_t signature[4];
    float version;
};
#pragma pack(pop)

Model::Model()
    : m_name(0),
      m_englishName(0),
      m_comment(0),
      m_englishComment(0),
      m_error(kNoError),
      m_encoding(kUTF16)
{
}

Model::~Model()
{
    release();
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
    internal::drain(sizeof(Header), ptr, rest);
    if (!internal::size8(ptr, rest, flagSize) || flagSize != 8) {
        m_error = kInvalidFlagSizeError;
        return false;
    }
    info.isUTF8 = *reinterpret_cast<uint8_t *>(ptr) == 1;
    info.additionalUVSize = *reinterpret_cast<uint8_t *>(ptr + 1);
    info.vertexIndexSize = *reinterpret_cast<uint8_t *>(ptr + 2);
    info.textureIndexSize = *reinterpret_cast<uint8_t *>(ptr + 3);
    info.materialIndexSize = *reinterpret_cast<uint8_t *>(ptr + 4);
    info.boneIndexSize = *reinterpret_cast<uint8_t *>(ptr + 5);
    info.morphIndexSize = *reinterpret_cast<uint8_t *>(ptr + 6);
    info.rigidBodyIndexSize = *reinterpret_cast<uint8_t *>(ptr + 7);
    internal::drain(flagSize, ptr, rest);

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
    internal::drain(nindices * info.vertexIndexSize, ptr, rest);

    /* texture lookup table */
    size_t ntextures;
    if (!internal::size32(ptr, rest, ntextures)) {
        m_error = kInvalidTextureSizeError;
        return false;
    }
    info.texturesPtr = ptr;
    for (size_t i = 0; i < ntextures; i++) {
        size_t nTextureSize;
        uint8_t *texturePtr;
        if (!internal::sizeText(ptr, rest, texturePtr, nTextureSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
    }

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
    size_t nDisplayNames;
    if (!internal::size32(ptr, rest, nDisplayNames)) {
        m_error = kInvalidDisplayNameSizeError;
        return false;
    }
    info.displayNamesPtr = ptr;
    for (size_t i = 0; i < nDisplayNames; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
        if (!internal::validateSize(ptr, sizeof(uint8_t), rest)) {
            return false;
        }
        if (!internal::size32(ptr, rest, size)) {
            return false;
        }
        for (size_t j = 0; j < size; j++) {
            size_t type;
            if (!internal::size8(ptr, rest, type)) {
                return false;
            }
            switch (type) {
            case 0:
                if (!internal::validateSize(ptr, info.boneIndexSize, rest)) {
                    return false;
                }
                break;
            case 1:
                if (!internal::validateSize(ptr, info.morphIndexSize, rest)) {
                    return false;
                }
                break;
            default:
                assert(0);
                break;
            }
        }
        // TODO:
    }
    info.displayNamesCount = nDisplayNames;

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

    return true;
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
        parseDisplayNames(info);
        parseRigidBodies(info);
        parseJoints(info);
        return true;
    }
    return false;
}

void Model::save(uint8_t *data) const
{
}

void Model::release()
{
    m_vertices.releaseAll();
    // m_texture.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_rigidBodies.releaseAll();
    m_joints.releaseAll();
    delete[] m_name;
    m_name = 0;
    delete[] m_englishName;
    m_englishName = 0;
    delete[] m_comment;
    m_comment = 0;
    delete[] m_englishComment;
    m_englishComment = 0;
    m_error = kNoError;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    m_encoding = info.isUTF8 ? kUTF8 : kUTF16;
    m_name = internal::allocateText(info.namePtr, info.nameSize);
    m_englishName = internal::allocateText(info.englishNamePtr, info.englishNameSize);
    m_comment = internal::allocateText(info.commentPtr, info.commentSize);
    m_englishComment = internal::allocateText(info.englishCommentPtr, info.englishCommentSize);
}

void Model::parseVertices(const DataInfo &info)
{
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.verticesPtr;
    size_t size;
    for(int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        vertex->read(ptr, info, size);
        m_vertices.add(vertex);
        ptr += size;
    }
}

void Model::parseIndices(const DataInfo &info)
{
}

void Model::parseTextures(const DataInfo &info)
{
}

void Model::parseMaterials(const DataInfo &info)
{
}

void Model::parseBones(const DataInfo &info)
{
}

void Model::parseMorphs(const DataInfo &info)
{
}

void Model::parseDisplayNames(const DataInfo &info)
{
}

void Model::parseRigidBodies(const DataInfo &info)
{
}

void Model::parseJoints(const DataInfo &info)
{
}

}
}
