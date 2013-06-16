/**

 Copyright (c) 2009-2011  Nagoya Institute of Technology
                          Department of Computer Science
               2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/vmd/LightKeyframe.h"

namespace vpvl2
{
namespace vmd
{

#pragma pack(push, 1)

struct LightKeyframeChunk
{
    int32_t timeIndex;
    float32_t color[3];
    float32_t direction[3];
};

#pragma pack(pop)

size_t LightKeyframe::strideSize()
{
    return sizeof(LightKeyframeChunk);
}

LightKeyframe::LightKeyframe()
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_color(0.0f, 0.0f, 0.0f),
      m_direction(0.0f, 0.0f, 0.0f)
{
}

LightKeyframe::~LightKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
    m_color.setZero();
    m_direction.setZero();
}

void LightKeyframe::read(const uint8_t *data)
{
    LightKeyframeChunk chunk;
    internal::getData(data, chunk);
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    setTimeIndex(static_cast<const TimeIndex>(chunk.timeIndex));
    internal::setPositionRaw(chunk.color, m_color);
    internal::setPosition(chunk.direction, m_direction);
}

void LightKeyframe::write(uint8_t *data) const
{
    LightKeyframeChunk chunk;
    chunk.timeIndex = static_cast<int>(m_timeIndex);
    internal::getPositionRaw(m_color, chunk.color);
    internal::getPosition(m_direction, chunk.direction);
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

size_t LightKeyframe::estimateSize() const
{
    return strideSize();
}

ILightKeyframe *LightKeyframe::clone() const
{
    LightKeyframe *keyframe = new LightKeyframe();
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setColor(m_color);
    keyframe->setDirection(m_direction);
    return keyframe;
}

void LightKeyframe::setColor(const Vector3 &value)
{
    m_color = value;
}

void LightKeyframe::setDirection(const Vector3 &value)
{
    m_direction = value;
}

} /* namespace vmd */
} /* namespace vpvl2 */
