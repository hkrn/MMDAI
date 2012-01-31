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
    uint8_t signature[3];
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
    // Header[3] + Version[4] + Name[20] + Comment[256]
    if (!data || sizeof(Header) > rest) {
        m_error = kInvalidHeaderError;
        return false;
    }

    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header *header = reinterpret_cast<Header *>(ptr);
    info.basePtr = ptr;

    // Check the signature and version is correct
    if (memcmp(header->signature, "PMX", 3) != 0) {
        m_error = kInvalidSignatureError;
        return false;
    }
    // version
    if (header->version != 2.0) {
        m_error = kInvalidVersionError;
        return false;
    }
    // flags
    if (sizeof(uint8_t) > rest) {
        m_error = 0;
        return false;
    }
    // name
    size_t nameSize;
    if (!internal::size32(ptr, rest, nameSize)) {
        m_error = 0;
        return false;
    }
    // english name
    size_t englishNameSize;
    if (!internal::size32(ptr, rest, englishNameSize)) {
        m_error = 0;
        return false;
    }
    // comment
    size_t commentSize;
    if (!internal::size32(ptr, rest, commentSize)) {
        m_error = 0;
        return false;
    }
    // english comment
    size_t englishCommentSize;
    if (!internal::size32(ptr, rest, englishCommentSize)) {
        m_error = 0;
        return false;
    }
    // vertex size
    if (!Vertex::preparse(ptr, rest)) {
        m_error = 0;
        return false;
    }
    // indices
    size_t nindices;
    if (!internal::size32(ptr, rest, nindices)) {
        m_error = 0;
        return false;
    }
    // texture lookup table
    if (sizeof(int) > rest) {
        m_error = 0;
        return false;
    }
    int ntextures = *reinterpret_cast<int *>(ptr);
    for (int i = 0; i < ntextures; i++) {
        int nTextureSize;
        if (!internal::size32(ptr, rest, nTextureSize)) {
            m_error = 0;
            return false;
        }
    }
    // material
    if (!Material::preparse(ptr, rest)) {
        m_error = 0;
        return false;
    }
    // bone
    if (!Bone::preparse(ptr, rest)) {
        m_error = 0;
        return false;
    }
    // morph
    if (!Morph::preparse(ptr, rest)) {
        m_error = 0;
        return false;
    }
    // TODO

    // rigidbody
    if (!RigidBody::preparse(ptr, rest)) {
        m_error = 0;
        return false;
    }
    // constraint
    if (!Constraint::preparse(ptr, rest)) {
        m_error = 0;
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

void release()
{
}

}
}
