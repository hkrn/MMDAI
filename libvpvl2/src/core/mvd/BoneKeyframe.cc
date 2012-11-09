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

#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/NameListSection.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct BoneKeyframeChunk {
    BoneKeyframeChunk() {}
    int layerIndex;
    uint64_t timeIndex;
    float position[3];
    float rotation[4];
    InterpolationPair x;
    InterpolationPair y;
    InterpolationPair z;
    InterpolationPair r;
};

#pragma pack(pop)

BoneKeyframe::BoneKeyframe(NameListSection *nameListSectionRef)
    : BaseKeyframe(),
      m_ptr(0),
      m_nameListSectionRef(nameListSectionRef),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity())
{
}

BoneKeyframe::~BoneKeyframe()
{
    m_ptr = 0;
    m_nameListSectionRef = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
}

size_t BoneKeyframe::size()
{
    static const BoneKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool BoneKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

int BoneKeyframe::interpolationTableSize()
{
    return 256;
}

void BoneKeyframe::read(const uint8_t *data)
{
    BoneKeyframeChunk chunk;
    internal::getData(data, chunk);
    internal::setPosition(chunk.position, m_position);
    internal::setRotation2(chunk.rotation, m_rotation);
    setTimeIndex(TimeIndex(chunk.timeIndex));
    setLayerIndex(chunk.layerIndex);
    setInterpolationParameter(kX, Motion::InterpolationTable::toQuadWord(chunk.x));
    setInterpolationParameter(kY, Motion::InterpolationTable::toQuadWord(chunk.y));
    setInterpolationParameter(kZ, Motion::InterpolationTable::toQuadWord(chunk.z));
    setInterpolationParameter(kRotation, Motion::InterpolationTable::toQuadWord(chunk.r));
}

void BoneKeyframe::write(uint8_t *data) const
{
    BoneKeyframeChunk chunk;
    internal::getPosition(m_position, chunk.position);
    internal::getRotation2(m_rotation, chunk.rotation);
    chunk.timeIndex = uint64_t(timeIndex());
    chunk.layerIndex = layerIndex();
    tableForX().getInterpolationPair(chunk.x);
    tableForY().getInterpolationPair(chunk.y);
    tableForZ().getInterpolationPair(chunk.z);
    tableForRotation().getInterpolationPair(chunk.r);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk), data);
}

size_t BoneKeyframe::estimateSize() const
{
    return size();
}

IBoneKeyframe *BoneKeyframe::clone() const
{
    BoneKeyframe *frame = m_ptr = new BoneKeyframe(m_nameListSectionRef);
    frame->setName(m_namePtr);
    frame->setTimeIndex(m_timeIndex);
    frame->setLayerIndex(m_layerIndex);
    frame->setLocalPosition(m_position);
    frame->setLocalRotation(m_rotation);
    frame->setInterpolationParameter(kX, m_interpolationX.parameter);
    frame->setInterpolationParameter(kY, m_interpolationY.parameter);
    frame->setInterpolationParameter(kZ, m_interpolationZ.parameter);
    frame->setInterpolationParameter(kRotation, m_interpolationRotation.parameter);
    m_ptr = 0;
    return frame;
}

void BoneKeyframe::setDefaultInterpolationParameter()
{
    m_interpolationX.reset();
    m_interpolationY.reset();
    m_interpolationZ.reset();
    m_interpolationRotation.reset();
}

void BoneKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
    switch (type) {
    case kX:
        m_interpolationX.build(value, interpolationTableSize());
        break;
    case kY:
        m_interpolationY.build(value, interpolationTableSize());
        break;
    case kZ:
        m_interpolationZ.build(value, interpolationTableSize());
        break;
    case kRotation:
        m_interpolationRotation.build(value, interpolationTableSize());
        break;
    default:
        break;
    }
}

void BoneKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
    switch (type) {
    case kX:
        value = m_interpolationX.parameter;
        break;
    case kY:
        value = m_interpolationY.parameter;
        break;
    case kZ:
        value = m_interpolationZ.parameter;
        break;
    case kRotation:
        value = m_interpolationRotation.parameter;
        break;
    default:
        break;
    }
}

const Vector3 &BoneKeyframe::localPosition() const
{
    return m_position;
}

const Quaternion &BoneKeyframe::localRotation() const
{
    return m_rotation;
}

void BoneKeyframe::setLocalPosition(const Vector3 &value)
{
    m_position = value;
}

void BoneKeyframe::setLocalRotation(const Quaternion &value)
{
    m_rotation = value;
}

void BoneKeyframe::setName(const IString *value)
{
    internal::setString(value, m_namePtr);
    m_nameListSectionRef->addName(value);
}

IKeyframe::Type BoneKeyframe::type() const
{
    return kBone;
}

const Motion::InterpolationTable &BoneKeyframe::tableForX() const
{
    return m_interpolationX;
}

const Motion::InterpolationTable &BoneKeyframe::tableForY() const
{
    return m_interpolationY;
}

const Motion::InterpolationTable &BoneKeyframe::tableForZ() const
{
    return m_interpolationZ;
}

const Motion::InterpolationTable &BoneKeyframe::tableForRotation() const
{
    return m_interpolationRotation;
}

} /* namespace mvd */
} /* namespace vpvl2 */
