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

namespace vpvl2
{
namespace pmx
{

#pragma pack(push, 1)

struct VertexUnit {
    float position[3];
    float normal[3];
    float texcoord[2];
};

struct AdditinalUVUnit {
    float value[4];
};

#pragma pack(pop)

Vertex::Vertex()
{
}

Vertex::~Vertex()
{
}

bool Vertex::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.verticesPtr = ptr;
    size_t baseSize = sizeof(VertexUnit) + sizeof(AdditinalUVUnit) * info.additionalUVSize;
    for (int i = 0; i < size; i++) {
        if (baseSize > rest)
            return false;
        internal::drain(baseSize, ptr, rest);
        size_t type;
        /* bone type */
        if (!internal::size8(ptr, rest, type))
            return false;
        size_t boneSize;
        switch (type) {
        case 0: /* BDEF1 */
            boneSize = info.boneIndexSize;
            break;
        case 1: /* BDEF2 */
            boneSize = info.boneIndexSize * 2 + sizeof(float);
            break;
        case 2: /* BDEF4 */
            boneSize = info.boneIndexSize * 4 + sizeof(float) * 4;
            break;
        case 3: /* SDEF */
            boneSize = info.boneIndexSize * 2 + sizeof(float) * 10;
            break;
        default: /* unexpected value */
            assert(0);
            return false;
        }
        boneSize += sizeof(float); /* edge */
        if (boneSize > rest)
            return false;
        internal::drain(boneSize, ptr, rest);
    }
    info.verticesCount = size;
    return rest > 0;
}

void Vertex::read(const uint8_t *data)
{
}

void Vertex::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */

