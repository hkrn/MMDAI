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
    // header
    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header *header = reinterpret_cast<Header *>(ptr);
    info.basePtr = ptr;

    // Check the signature and version is correct
    if (memcmp(header->signature, "PMX ", 4) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }

    // version
    if (header->version != 2.0) {
        m_error = kInvalidVersionError;
        return false;
    }

    // flags
    size_t flagSize;
    if (!internal::size8(ptr, rest, flagSize) || flagSize != 8) {
        m_error = kInvalidFlagSizeError;
        return false;
    }
    info.isUTF8 = *reinterpret_cast<uint8_t *>(ptr);
    info.additionalUVSize = *reinterpret_cast<uint8_t *>(ptr + 1);
    info.vertexIndexSize = *reinterpret_cast<uint8_t *>(ptr + 2);
    info.textureIndexSize = *reinterpret_cast<uint8_t *>(ptr + 3);
    info.materialIndexSize = *reinterpret_cast<uint8_t *>(ptr + 4);
    info.boneIndexSize = *reinterpret_cast<uint8_t *>(ptr + 5);
    info.morphIndexSize = *reinterpret_cast<uint8_t *>(ptr + 6);
    info.rigidBodyIndexSize = *reinterpret_cast<uint8_t *>(ptr + 7);
    ptr += flagSize;

    // name
    size_t nameSize;
    if (!internal::size32(ptr, rest, nameSize)) {
        m_error = kInvalidNameSizeError;
        return false;
    }
    info.namePtr = ptr;
    info.nameSize = nameSize;

    // english name
    size_t englishNameSize;
    if (!internal::size32(ptr, rest, englishNameSize)) {
        m_error = kInvalidEnglishNameSizeError;
        return false;
    }
    info.englishNamePtr = ptr;
    info.englishNameSize = englishNameSize;

    // comment
    size_t commentSize;
    if (!internal::size32(ptr, rest, commentSize)) {
        m_error = kInvalidCommentSizeError;
        return false;
    }
    info.commentPtr = ptr;
    info.commentSize = commentSize;

    // english comment
    size_t englishCommentSize;
    if (!internal::size32(ptr, rest, englishCommentSize)) {
        m_error = kInvalidEnglishCommentSizeError;
        return false;
    }
    info.englishCommentPtr = ptr;
    info.englishCommentSize = englishCommentSize;

    // vertex size
    info.verticesPtr = ptr + sizeof(int);
    if (!Vertex::preparse(ptr, rest, info.vertexIndexSize)) {
        m_error = kInvalidVerticesError;
        return false;
    }

    // indices
    size_t nindices;
    info.indicesPtr = ptr + sizeof(int);
    if (!internal::size32(ptr, rest, nindices)) {
        m_error = kInvalidIndicesError;
        return false;
    }

    // texture lookup table
    size_t ntextures;
    if (!internal::size32(ptr, rest, ntextures)) {
        m_error = kInvalidTextureSizeError;
        return false;
    }
    info.texturesPtr = ptr;
    for (size_t i = 0; i < ntextures; i++) {
        size_t nTextureSize;
        if (!internal::size32(ptr, rest, nTextureSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
    }

    // material
    info.materialsPtr = ptr + sizeof(int);
    if (!Material::preparse(ptr, rest, info.materialIndexSize)) {
        m_error = kInvalidMaterialsError;
        return false;
    }

    // bone
    info.bonesPtr = ptr + sizeof(int);
    if (!Bone::preparse(ptr, rest, info.boneIndexSize)) {
        m_error = kInvalidBonesError;
        return false;
    }

    // morph
    info.morphsPtr = ptr + sizeof(int);
    if (!Morph::preparse(ptr, rest, info.morphIndexSize)) {
        m_error = kInvalidMorphsError;
        return false;
    }

    // display name table
    size_t nDisplayNames;
    if (!internal::size32(ptr, rest, nDisplayNames)) {
        m_error = kInvalidDisplayNameSizeError;
        return false;
    }
    info.displayNamesPtr = ptr;
    for (size_t i = 0; i < nDisplayNames; i++) {
        size_t nNameSize;
        if (!internal::size32(ptr, rest, nNameSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
        size_t nEnglishNameSize;
        if (!internal::size32(ptr, rest, nEnglishNameSize)) {
            m_error = kInvalidTextureError;
            return false;
        }
        if (sizeof(uint8_t) > rest) {
            return false;
        }
        if (!internal::size32(ptr, rest, size)) {
            return false;
        }
        // TODO:
    }

    // rigidbody
    info.rigidBodiesPtr = ptr + sizeof(int);
    if (!RigidBody::preparse(ptr, rest, info.rigidBodyIndexSize)) {
        m_error = kInvalidRigidBodiesError;
        return false;
    }

    // constraint
    info.constraintsPtr = ptr + sizeof(int);
    if (!Constraint::preparse(ptr, rest)) {
        m_error = kInvalidConstraintsError;
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
        return true;
    }
    return false;
}

void Model::save(uint8_t *data) const
{
}

void Model::release()
{
}

}
}
