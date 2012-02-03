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

struct MaterialUnit {
    float diffuse[4];
    float specular[3];
    float shininess;
    float ambient[3];
    uint8_t flags;
    float edgeColor[4];
    float edgeSize;
};

#pragma pack(pop)

Material::Material()
{
}

Material::~Material()
{
}

bool Material::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.materialsPtr = ptr;
    size_t nTextureIndexSize = info.textureIndexSize * 2;
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
        if (!internal::validateSize(ptr, sizeof(MaterialUnit), rest)) {
            return false;
        }
        /* normal texture + sphere map texture */
        if (!internal::validateSize(ptr, nTextureIndexSize, rest)) {
            return false;
        }
        if (sizeof(uint16_t) > rest) {
            return false;
        }
        bool isSharedToonTexture = *(ptr + sizeof(uint8_t)) == 1;
        internal::drain(sizeof(uint16_t), ptr, rest);
        /* shared toon texture index */
        if (isSharedToonTexture) {
            if (!internal::validateSize(ptr, sizeof(uint8_t), rest)) {
                return false;
            }
        }
        /* independent toon texture index */
        else {
            if (!internal::validateSize(ptr, info.textureIndexSize, rest)) {
                return false;
            }
        }
        /* free area */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* number of indices */
        if (!internal::validateSize(ptr, sizeof(int), rest)) {
            return false;
        }
    }
    info.materialsCount = size;
    return true;
}

void Material::read(const uint8_t *data)
{
}

void Material::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */

