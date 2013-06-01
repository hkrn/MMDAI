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

#include "vpvl2/mvd/CameraKeyframe.h"

namespace vpvl2
{
namespace mvd
{

#pragma pack(push, 1)

struct CameraKeyframeChunk {
    CameraKeyframeChunk() {}
    int32_t layerIndex;
    uint64_t timeIndex;
    float32_t distance;
    float32_t position[3];
    float32_t rotation[3];
    float32_t fov;
    uint8_t perspective;
    InterpolationPair positionIP;
    InterpolationPair rotationIP;
    InterpolationPair distanceIP;
    InterpolationPair fovIP;
};

#pragma pack(pop)

CameraKeyframe::CameraKeyframe(const Motion *motionRef)
    : VPVL2_KEYFRAME_INITIALIZE_FIELDS(),
      m_ptr(0),
      m_motionRef(motionRef),
      m_position(kZeroV3),
      m_angle(kZeroV3),
      m_distance(0),
      m_fov(0),
      m_perspective(false)
{
}

CameraKeyframe::~CameraKeyframe()
{
    VPVL2_KEYFRAME_DESTROY_FIELDS()
    delete m_ptr;
    m_ptr = 0;
    m_position.setZero();
    m_angle.setZero();
    m_distance = 0;
    m_fov = 0;
    m_perspective = false;
}

size_t CameraKeyframe::size()
{
    static const CameraKeyframeChunk keyframe;
    return sizeof(keyframe);
}

bool CameraKeyframe::preparse(uint8_t *&ptr, size_t &rest, size_t reserved, Motion::DataInfo & /* info */)
{
    if (!internal::validateSize(ptr, size(), rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD camera keyframe detected: ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
        return false;
    }
    if (!internal::validateSize(ptr, reserved, rest)) {
        VPVL2_LOG(WARNING, "Invalid size of MVD reserved camera keyframe detected: ptr=" << static_cast<const void *>(ptr) << " size=" << reserved << " rest=" << rest);
        return false;
    }
    return true;
}

int CameraKeyframe::interpolationTableSize()
{
    return 256;
}

void CameraKeyframe::read(const uint8_t *data)
{
    CameraKeyframeChunk chunk;
    internal::getData(data, chunk);
    Vector3 angle;
    internal::setPosition(chunk.position, m_position);
    internal::setPositionRaw(chunk.rotation, angle);
#ifdef VPVL2_COORDINATE_OPENGL
    setAngle(Vector3(btDegrees(angle[0]), btDegrees(angle[1]) - 180, btDegrees(angle[2])));
#else
    setAngle(Vector3(btDegrees(angle[0]), btDegrees(angle[1]), btDegrees(angle[2])));
#endif
    setDistance(chunk.distance);
    setTimeIndex(TimeIndex(chunk.timeIndex));
    setLayerIndex(chunk.layerIndex);
    setFov(btDegrees(chunk.fov));
    setPerspective(chunk.perspective != 0);
    setInterpolationParameter(kCameraLookAtX, Motion::InterpolationTable::toQuadWord(chunk.positionIP));
    setInterpolationParameter(kCameraAngle, Motion::InterpolationTable::toQuadWord(chunk.rotationIP));
    setInterpolationParameter(kCameraFov, Motion::InterpolationTable::toQuadWord(chunk.fovIP));
    setInterpolationParameter(kCameraDistance, Motion::InterpolationTable::toQuadWord(chunk.distanceIP));
}

void CameraKeyframe::write(uint8_t *data) const
{
    CameraKeyframeChunk chunk;
    internal::getPosition(lookAt(), chunk.position);
    const Vector3 &a = angle();
#ifdef VPVL2_COORDINATE_OPENGL
    chunk.rotation[0] = btRadians(a.x());
    chunk.rotation[1] = btRadians(a.y() + 180);
    chunk.rotation[2] = btRadians(a.z());
#else
    chunk.rotation[0] = btRadians(a.x());
    chunk.rotation[1] = btRadians(a.y());
    chunk.rotation[2] = btRadians(a.z());
#endif
    chunk.distance = distance();
    chunk.timeIndex = uint64_t(timeIndex());
    chunk.layerIndex = layerIndex();
    chunk.fov = btRadians(fov());
    chunk.perspective = isPerspective() ? 1 : 0;
    tableForPosition().getInterpolationPair(chunk.positionIP);
    tableForRotation().getInterpolationPair(chunk.rotationIP);
    tableForFov().getInterpolationPair(chunk.fovIP);
    tableForDistance().getInterpolationPair(chunk.distanceIP);
    internal::writeBytes(&chunk, sizeof(chunk), data);
}

size_t CameraKeyframe::estimateSize() const
{
    return size();
}

ICameraKeyframe *CameraKeyframe::clone() const
{
    CameraKeyframe *keyframe = m_ptr = new CameraKeyframe(m_motionRef);
    keyframe->setTimeIndex(m_timeIndex);
    keyframe->setLayerIndex(m_layerIndex);
    keyframe->setDistance(m_distance);
    keyframe->setFov(m_fov);
    keyframe->setLookAt(m_position);
    keyframe->setAngle(m_angle);
    keyframe->setPerspective(m_perspective);
    keyframe->setInterpolationParameter(kCameraLookAtX, m_interpolationPosition.parameter);
    keyframe->setInterpolationParameter(kCameraAngle, m_interpolationRotation.parameter);
    keyframe->setInterpolationParameter(kCameraFov, m_interpolationFov.parameter);
    keyframe->setInterpolationParameter(kCameraDistance, m_interpolationDistance.parameter);
    m_ptr = 0;
    return keyframe;
}

void CameraKeyframe::setDefaultInterpolationParameter()
{
    m_interpolationPosition.reset();
    m_interpolationRotation.reset();
    m_interpolationFov.reset();
    m_interpolationDistance.reset();
}

void CameraKeyframe::setInterpolationParameter(InterpolationType type, const QuadWord &value)
{
    switch (type) {
    case kCameraLookAtX:
    case kCameraLookAtY:
    case kCameraLookAtZ:
        m_interpolationPosition.build(value, interpolationTableSize());
        break;
    case kCameraAngle:
        m_interpolationRotation.build(value, interpolationTableSize());
        break;
    case kCameraFov:
        m_interpolationFov.build(value, interpolationTableSize());
        break;
    case kCameraDistance:
        m_interpolationDistance.build(value, interpolationTableSize());
        break;
    default:
        break;
    }
}

void CameraKeyframe::getInterpolationParameter(InterpolationType type, QuadWord &value) const
{
    switch (type) {
    case kCameraLookAtX:
    case kCameraLookAtY:
    case kCameraLookAtZ:
        value = m_interpolationPosition.parameter;
        break;
    case kCameraAngle:
        value = m_interpolationRotation.parameter;
        break;
    case kCameraFov:
        value = m_interpolationFov.parameter;
        break;
    case kCameraDistance:
        value = m_interpolationDistance.parameter;
        break;
    default:
        break;
    }
}

Vector3 CameraKeyframe::lookAt() const
{
    return m_position;
}

Vector3 CameraKeyframe::angle() const
{
    return m_angle;
}

Scalar CameraKeyframe::distance() const
{
    return m_distance;
}

Scalar CameraKeyframe::fov() const
{
    return m_fov;
}

bool CameraKeyframe::isPerspective() const
{
    return m_perspective;
}

void CameraKeyframe::setLookAt(const Vector3 &value)
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
    m_perspective = value;
}

void CameraKeyframe::setName(const IString * /* value */)
{
}

IKeyframe::Type CameraKeyframe::type() const
{
    return kCameraKeyframe;
}

const Motion *CameraKeyframe::parentMotionRef() const
{
    return m_motionRef;
}

const Motion::InterpolationTable &CameraKeyframe::tableForPosition() const
{
    return m_interpolationPosition;
}

const Motion::InterpolationTable &CameraKeyframe::tableForRotation() const
{
    return m_interpolationRotation;
}

const Motion::InterpolationTable &CameraKeyframe::tableForFov() const
{
    return m_interpolationFov;
}

const Motion::InterpolationTable &CameraKeyframe::tableForDistance() const
{
    return m_interpolationDistance;
}

} /* namespace mvd */
} /* namespace vpvl2 */
