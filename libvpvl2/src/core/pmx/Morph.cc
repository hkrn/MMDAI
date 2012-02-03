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

struct MorphUnit {
    uint8_t category;
    uint8_t type;
    int size;
};

struct VertexMorph {
    float position[3];
};

struct UVMorph {
    float position[4];
};

struct BoneMorph {
    float position[3];
    float rotation[4];
};

struct MaterialMorph {
    uint8_t operation;
    float diffuse[4];
    float specular[3];
    float shininess;
    float ambient[3];
    float edgeColor[4];
    float edgeSize;
    float textureWeight[4];
    float sphereTextureWeight[4];
    float toonTextureWeight[4];
};
struct GroupMorph {
    float weight;
};

#pragma pack(pop)

Morph::Morph()
{
}

Morph::~Morph()
{
}

bool Morph::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.morphsPtr = ptr;
    for (size_t i = 0; i < size; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        if (sizeof(MorphUnit) > rest) {
            return false;
        }
        MorphUnit *morph = reinterpret_cast<MorphUnit *>(ptr);
        internal::drain(sizeof(MorphUnit), ptr, rest);
        int nmorphs = morph->size;
        size_t extraSize;
        switch (morph->type) {
        case 0: /* group */
            extraSize = info.morphIndexSize + sizeof(GroupMorph);
            break;
        case 1: /* vertex */
            extraSize = info.vertexIndexSize + sizeof(VertexMorph);
            break;
        case 2: /* bone */
            extraSize = info.boneIndexSize + sizeof(BoneMorph);
            break;
        case 3: /* UV */
        case 4: /* UV1 */
        case 5: /* UV2 */
        case 6: /* UV3 */
        case 7: /* UV4 */
            extraSize = info.vertexIndexSize + sizeof(UVMorph);
            break;
        case 8: /* material */
            extraSize = info.materialIndexSize + sizeof(MaterialMorph);
            break;
        default:
            assert(0);
            return false;
        }
        for (int j = 0; j < nmorphs; j++) {
            if (!internal::validateSize(ptr, extraSize, rest)) {
                return false;
            }
        }
    }
    info.morphsCount = size;
    return true;
}

void Morph::read(const uint8_t *data)
{
}

void Morph::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */

