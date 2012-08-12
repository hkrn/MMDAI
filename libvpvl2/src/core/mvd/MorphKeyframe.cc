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

#include "vpvl2/mvd/MorphKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct MorphKeyframeChunk {
    uint64_t timeIndex;
    float weight;
    InterpolationPair weightIP;
};

#pragma pack(pop)

MorphKeyframe::MorphKeyframe(NameListSection *nameListSectionRef)
    : BaseKeyframe(),
      m_ptr(0),
      m_nameListSectionRef(nameListSectionRef),
      m_weight(0)
{
}

MorphKeyframe::~MorphKeyframe()
{
    delete m_ptr;
    m_ptr = 0;
    m_nameListSectionRef = 0;
    m_weight = 0;
}

size_t MorphKeyframe::size()
{
    static MorphKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool MorphKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

void MorphKeyframe::read(const uint8_t *data)
{
    const MorphKeyframeChunk *chunk = reinterpret_cast<const MorphKeyframeChunk *>(data);
    setWeight(chunk->weight);
    setTimeIndex(chunk->timeIndex);
}

void MorphKeyframe::write(uint8_t * /* data */) const
{
}

size_t MorphKeyframe::estimateSize() const
{
    return size();
}

IMorphKeyframe *MorphKeyframe::clone() const
{
    MorphKeyframe *frame = m_ptr = new MorphKeyframe(m_nameListSectionRef);
    frame->setTimeIndex(m_timeIndex);
    frame->setWeight(m_weight);
    m_ptr = 0;
    return frame;
}

const IMorph::WeightPrecision &MorphKeyframe::weight() const
{
    return m_weight;
}

void MorphKeyframe::setWeight(const IMorph::WeightPrecision &value)
{
    m_weight = value;
}

void MorphKeyframe::setName(const IString * /* value */)
{
}

IMorphKeyframe::Type MorphKeyframe::type() const
{
    return kMorph;
}

const Motion::InterpolationTable &MorphKeyframe::tableForWeight() const
{
    return m_interpolationWeight;
}

} /* namespace mvd */
} /* namespace vpvl2 */
