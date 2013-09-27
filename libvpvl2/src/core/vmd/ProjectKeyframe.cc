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

#include "vpvl2/vmd/ProjectKeyframe.h"

namespace vpvl2
{
namespace vmd
{

#pragma pack(push, 1)

struct ProjetKeyframeChunk
{
    uint32 timeIndex;
    uint8 mode;
    float32 distance;
};

#pragma pack(pop)

vsize ProjectKeyframe::strideSize()
{
    return sizeof(ProjetKeyframeChunk);
}

ProjectKeyframe::ProjectKeyframe(IEncoding *encoding)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_encodingRef(encoding),
      m_distance(0.0f)
{
}

ProjectKeyframe::~ProjectKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
}

void ProjectKeyframe::read(const uint8 *data)
{
    ProjetKeyframeChunk chunk;
    internal::getData(data, chunk);
    setTimeIndex(static_cast<const TimeIndex>(chunk.timeIndex));
    setShadowDistance(chunk.distance);
}

void ProjectKeyframe::write(uint8 *data) const
{
    ProjetKeyframeChunk chunk;
    chunk.timeIndex = static_cast<int>(m_timeIndex);
    chunk.distance = float32(m_distance);
    chunk.mode = 0;
    internal::copyBytes(data, reinterpret_cast<const uint8 *>(&chunk), sizeof(chunk));
}

vsize ProjectKeyframe::estimateSize() const
{
    return strideSize();
}

IProjectKeyframe *ProjectKeyframe::clone() const
{
    ProjectKeyframe *keyframe = new ProjectKeyframe(m_encodingRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setShadowDistance(m_distance);
    return keyframe;
}

float32 ProjectKeyframe::gravityFactor() const
{
    return 0;
}

Vector3 ProjectKeyframe::gravityDirection() const
{
    return kZeroV3;
}

int ProjectKeyframe::shadowMode() const
{
    return 0;
}

float32 ProjectKeyframe::shadowDistance() const
{
    return m_distance;
}

float32 ProjectKeyframe::shadowDepth() const
{
    return 0;
}

IKeyframe::Type ProjectKeyframe::type() const
{
    return kProjectKeyframe;
}

void ProjectKeyframe::setGravityFactor(float32 /* value */)
{
}

void ProjectKeyframe::setName(const IString * /* value */)
{
}

void ProjectKeyframe::setGravityDirection(const Vector3 & /* value */)
{
}

void ProjectKeyframe::setShadowMode(int /* value */)
{
}

void ProjectKeyframe::setShadowDistance(float32 value)
{
    m_distance = value;
}

void ProjectKeyframe::setShadowDepth(float32 /* value */)
{
}


} /* namespace vmd */
} /* namespace vpvl2 */
