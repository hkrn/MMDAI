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

#include "vpvl2/mvd/EffectKeyframe.h"
#include "vpvl2/mvd/EffectSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct EffectSectionHeader {
    int reserved;
    int sizeOfKeyframe;
    int countOfKeyframes;
    int parameterSize;
    int parameterCount;
};

struct EffectParameter {
    int pid;
    int type;
};

#pragma pack(pop)

EffectSection::EffectSection(NameListSection *nameListSectionRef)
    : BaseSection(nameListSectionRef)
{
}

EffectSection::~EffectSection()
{
}

bool EffectSection::preparse(uint8_t *&ptr, size_t &rest, Motion::DataInfo &info)
{
    const EffectSectionHeader &header = *reinterpret_cast<const EffectSectionHeader *>(ptr);
    if (!internal::validateSize(ptr, sizeof(header), rest)) {
        return false;
    }
    const int nparameters = header.parameterCount;
    const size_t parameterArraySize = nparameters * sizeof(int);
    if (!internal::validateSize(ptr, parameterArraySize, rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, header.parameterSize - 8 * nparameters - 4, rest)) {
        return false;
    }
    const int nkeyframes = header.countOfKeyframes;
    const size_t reserved = header.sizeOfKeyframe - (EffectKeyframe::size() + parameterArraySize);
    for (int i = 0; i < nkeyframes; i++) {
        if (!EffectKeyframe::preparse(ptr, rest, reserved, info)) {
            return false;
        }
    }
    return true;
}

void EffectSection::read(const uint8_t *data)
{
}

void EffectSection::write(uint8_t *data) const
{
}

size_t EffectSection::estimateSize() const
{
    return 0;
}

} /* namespace mvd */
} /* namespace vpvl2 */
