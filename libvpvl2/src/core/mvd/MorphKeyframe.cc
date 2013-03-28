/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct MorphKeyframeChunk {
    MorphKeyframeChunk() {}
    uint64_t timeIndex;
    float weight;
    InterpolationPair weightIP;
};

#pragma pack(pop)

MorphKeyframe::MorphKeyframe(const Motion *motionRef)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_ptr(0),
      m_motionRef(motionRef),
      m_weight(0)
{
}

MorphKeyframe::~MorphKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
    delete m_ptr;
    m_ptr = 0;
    m_motionRef = 0;
    m_weight = 0;
}

size_t MorphKeyframe::size()
{
    static const MorphKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool MorphKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        VPVL2_LOG(LOG(ERROR) << "Invalid size of MVD morph keyframe detected: ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        VPVL2_LOG(LOG(ERROR) << "Invalid size of MVD reserved morph keyframe detected: ptr=" << static_cast<const void *>(ptr) << " size=" << reserved << " rest=" << rest);
        return false;
    }
    return true;
}

int MorphKeyframe::interpolationTableSize()
{
    return 256;
}

void MorphKeyframe::read(const uint8_t *data)
{
    MorphKeyframeChunk chunk;
    internal::getData(data, chunk);
    setWeight(chunk.weight);
    setTimeIndex(TimeIndex(chunk.timeIndex));
    setInterpolationParameter(kWeight, Motion::InterpolationTable::toQuadWord(chunk.weightIP));
}

void MorphKeyframe::write(uint8_t *data) const
{
    MorphKeyframeChunk chunk;
    chunk.weight = float(weight());
    chunk.timeIndex = uint64_t(timeIndex());
    tableForWeight().getInterpolationPair(chunk.weightIP);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk), data);
}

size_t MorphKeyframe::estimateSize() const
{
    return size();
}

IMorphKeyframe *MorphKeyframe::clone() const
{
    MorphKeyframe *keyframe = m_ptr = new MorphKeyframe(m_motionRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setLayerIndex(m_layerIndex);
    keyframe->setWeight(m_weight);
    keyframe->setInterpolationParameter(kWeight, m_interpolationWeight.parameter);
    m_ptr = 0;
    return keyframe;
}

void MorphKeyframe::setDefaultInterpolationParameter()
{
    m_interpolationWeight.reset();
}

void MorphKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
    switch (type) {
    case kWeight:
    default:
        m_interpolationWeight.build(value, interpolationTableSize());
        break;
    }
}

void MorphKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
    switch (type) {
    case kWeight:
    default:
        value = m_interpolationWeight.parameter;
        break;
    }
}

IMorph::WeightPrecision MorphKeyframe::weight() const
{
    return m_weight;
}

void MorphKeyframe::setWeight(const IMorph::WeightPrecision &value)
{
    m_weight = value;
}

void MorphKeyframe::setName(const IString *value)
{
    internal::setString(value, m_namePtr);
    m_motionRef->nameListSection()->addName(value);
}

IMorphKeyframe::Type MorphKeyframe::type() const
{
    return kMorphKeyframe;
}

const Motion *MorphKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

const Motion::InterpolationTable &MorphKeyframe::tableForWeight() const
{
    return m_interpolationWeight;
}

} /* namespace mvd */
} /* namespace vpvl2 */
