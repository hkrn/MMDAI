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

#include "vpvl2/mvd/CameraKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct CameraKeyframeChunk {
    int layerIndex;
    uint64_t timeIndex;
    float radius;
    float position[3];
    float rotation[3];
    float fov;
    uint8_t perspective;
    InterpolationPair positionIP;
    InterpolationPair rotationIP;
    InterpolationPair distanceIP;
    InterpolationPair fovIP;
};

#pragma pack(pop)

CameraKeyframe::CameraKeyframe()
    : BaseKeyframe(),
      m_ptr(0),
      m_position(kZeroV3),
      m_angle(kZeroV3),
      m_distance(0),
      m_fov(0),
      m_noPerspective(false)
{
}

CameraKeyframe::~CameraKeyframe()
{
    delete m_ptr;
    m_ptr = 0;
    m_position.setZero();
    m_angle.setZero();
    m_distance = 0;
    m_fov = 0;
    m_noPerspective = false;
}

size_t CameraKeyframe::size()
{
    static CameraKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool CameraKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        return false;
    }
    return true;
}

void CameraKeyframe::read(const uint8_t *data)
{
    const CameraKeyframeChunk *chunk = reinterpret_cast<const CameraKeyframeChunk *>(data);
    internal::setPosition(chunk->position, m_position);
    internal::setPositionRaw(chunk->rotation, m_angle);
    m_angle.setValue(-degree(m_angle[0]), -degree(m_angle[1]), degree(m_angle[2]));
    setTimeIndex(chunk->timeIndex);
    setLayerIndex(chunk->layerIndex);
}

void CameraKeyframe::write(uint8_t *data) const
{
}

size_t CameraKeyframe::estimateSize() const
{
    return 0;
}

ICameraKeyframe *CameraKeyframe::clone() const
{
    CameraKeyframe *frame = m_ptr = new CameraKeyframe();
    frame->setTimeIndex(m_timeIndex);
    frame->setLayerIndex(m_layerIndex);
    frame->setDistance(m_distance);
    frame->setFov(m_fov);
    frame->setPosition(m_position);
    frame->setAngle(m_angle);
    frame->setPerspective(!m_noPerspective);
    frame->m_parameter = m_parameter;
    m_ptr = 0;
    return frame;
}

void CameraKeyframe::setDefaultInterpolationParameter()
{
}

void CameraKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
}

void CameraKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
}

const Vector3 &CameraKeyframe::position() const
{
    return m_position;
}

const Vector3 &CameraKeyframe::angle() const
{
    return m_angle;
}

const Scalar &CameraKeyframe::distance() const
{
    return m_distance;
}

const Scalar &CameraKeyframe::fov() const
{
    return m_fov;
}

bool CameraKeyframe::isPerspective() const
{
    return !m_noPerspective;
}

void CameraKeyframe::setPosition(const Vector3 &value)
{
    m_position = value;
}

void CameraKeyframe::setAngle(const Vector3 &value)
{
    m_angle = value;
}

void CameraKeyframe::setDistance(const Scalar &value)
{
    m_distance = value;
}

void CameraKeyframe::setFov(const Scalar &value)
{
    m_fov = value;
}

void CameraKeyframe::setPerspective(bool value)
{
    m_noPerspective = !value;
}

void CameraKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type CameraKeyframe::type() const
{
    return kCamera;
}

} /* namespace mvd */
} /* namespace vpvl2 */
