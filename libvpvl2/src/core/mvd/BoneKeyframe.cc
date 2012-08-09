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

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct BoneKeyframeChunk {
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
    static BoneKeyframeChunk keyframe;
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

void BoneKeyframe::read(const uint8_t *data)
{
    const BoneKeyframeChunk *chunk = reinterpret_cast<const BoneKeyframeChunk *>(data);
    internal::setPosition(chunk->position, m_position);
    internal::setRotation(chunk->rotation, m_rotation);
    setTimeIndex(chunk->timeIndex);
    setLayerIndex(chunk->layerIndex);
}

void BoneKeyframe::write(uint8_t *data) const
{
}

size_t BoneKeyframe::estimateSize() const
{
    return 0;
}

IBoneKeyframe *BoneKeyframe::clone() const
{
    BoneKeyframe *frame = m_ptr = new BoneKeyframe(m_nameListSectionRef);
    frame->setName(m_name);
    frame->setTimeIndex(m_timeIndex);
    frame->setLayerIndex(m_layerIndex);
    frame->setPosition(m_position);
    frame->setRotation(m_rotation);
    frame->m_parameter = m_parameter;
    m_ptr = 0;
    return frame;
}

void BoneKeyframe::setDefaultInterpolationParameter()
{
}

void BoneKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
}

void BoneKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
}

const Vector3 &BoneKeyframe::position() const
{
    return m_position;
}

const Quaternion &BoneKeyframe::rotation() const
{
    return m_rotation;
}

void BoneKeyframe::setPosition(const Vector3 &value)
{
    m_position = value;
}

void BoneKeyframe::setRotation(const Quaternion &value)
{
    m_rotation = value;
}

void BoneKeyframe::setName(const IString *value)
{
}

IKeyframe::Type BoneKeyframe::type() const
{
    return kBone;
}

} /* namespace mvd */
} /* namespace vpvl2 */
