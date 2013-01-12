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

#include "vpvl2/mvd/ProjectKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct ProjectKeyframeChunk {
    ProjectKeyframeChunk() {}
    uint64_t timeIndex;
    float gravityFactor;
    float gravityDirection[3];
    int shadowMode;
    float shadowDistance;
    float shadowDepth;
};

#pragma pack(pop)

ProjectKeyframe::ProjectKeyframe(const Motion *motionRef)
    : BaseKeyframe(),
      m_ptr(0),
      m_motionRef(motionRef),
      m_gravityDirection(kZeroV3),
      m_gravityFactor(0),
      m_shadowDistance(0),
      m_shadowDepth(0),
      m_shadowMode(0)
{
}

ProjectKeyframe::~ProjectKeyframe()
{
    delete m_ptr;
    m_ptr = 0;
    m_motionRef = 0;
    m_gravityDirection.setZero();
    m_gravityFactor = 0;
    m_shadowDistance = 0;
    m_shadowDepth = 0;
    m_shadowMode = 0;
}

size_t ProjectKeyframe::size()
{
    static const ProjectKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool ProjectKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

void ProjectKeyframe::read(const uint8_t *data)
{
    ProjectKeyframeChunk chunk;
    internal::getData(data, chunk);
    setTimeIndex(TimeIndex(chunk.timeIndex));
    setGravityFactor(chunk.gravityFactor);
    Vector3 gravityDirection;
    internal::setPositionRaw(chunk.gravityDirection, gravityDirection);
    setGravityDirection(gravityDirection);
    setShadowMode(chunk.shadowMode);
    setShadowDistance(chunk.shadowDistance);
    setShadowDepth(chunk.shadowDepth);
}

void ProjectKeyframe::write(uint8_t * /* data */) const
{
    ProjectKeyframeChunk chunk;
    chunk.timeIndex = uint64_t(timeIndex());
    chunk.gravityFactor = gravityFactor();
    internal::getPositionRaw(gravityDirection(), chunk.gravityDirection);
    chunk.shadowMode = shadowMode();
    chunk.shadowDistance = shadowDistance();
    chunk.shadowDepth = shadowDepth();
}

size_t ProjectKeyframe::estimateSize() const
{
    return size();
}

IProjectKeyframe *ProjectKeyframe::clone() const
{
    ProjectKeyframe *keyframe = m_ptr = new ProjectKeyframe(m_motionRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setLayerIndex(m_layerIndex);
    keyframe->setGravityFactor(m_gravityFactor);
    keyframe->setGravityDirection(m_gravityDirection);
    keyframe->setShadowMode(m_shadowMode);
    keyframe->setShadowDistance(m_shadowDistance);
    keyframe->setShadowDepth(m_shadowDepth);
    m_ptr = 0;
    return keyframe;
}

void ProjectKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type ProjectKeyframe::type() const
{
    return kProjectKeyframe;
}

const Motion *ProjectKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

float ProjectKeyframe::gravityFactor() const
{
    return m_gravityFactor;
}

Vector3 ProjectKeyframe::gravityDirection() const
{
    return m_gravityDirection;
}

int ProjectKeyframe::shadowMode() const
{
    return m_shadowMode;
}

float ProjectKeyframe::shadowDistance() const
{
    return m_shadowDistance;
}

float ProjectKeyframe::shadowDepth() const
{
    return m_shadowDepth;
}

void ProjectKeyframe::setGravityFactor(float value)
{
    m_gravityFactor = value;
}

void ProjectKeyframe::setGravityDirection(const Vector3 &value)
{
    m_gravityDirection = value;
}

void ProjectKeyframe::setShadowMode(int value)
{
    m_shadowMode = value;
}

void ProjectKeyframe::setShadowDistance(float value)
{
    m_shadowDistance = value;
}

void ProjectKeyframe::setShadowDepth(float value)
{
    m_shadowDepth = value;
}

} /* namespace mvd */
} /* namespace vpvl2 */
