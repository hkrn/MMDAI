/**

 Copyright (c) 2010-2013  hkrn

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

#include "CameraKeyframeRefObject.h"

#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "Util.h"
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

CameraKeyframeRefObject::CameraKeyframeRefObject(CameraMotionTrack *trackRef, ICameraKeyframe *data)
    : BaseKeyframeRefObject(trackRef->parentMotion()),
      m_parentTrackRef(trackRef),
      m_keyframeRef(data)
{
    Q_ASSERT(m_parentTrackRef);
    Q_ASSERT(m_keyframeRef);
}

CameraKeyframeRefObject::~CameraKeyframeRefObject()
{
    m_parentTrackRef = 0;
    m_keyframeRef = 0;
}

QVector4D CameraKeyframeRefObject::interpolationParameter(int type) const
{
    QuadWord value;
    m_keyframeRef->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(type), value);
    return QVector4D(value.x(), value.y(), value.z(), value.w());
}

void CameraKeyframeRefObject::setInterpolationParameter(int type, const QVector4D &value)
{
    QuadWord v(value.x(), value.y(), value.z(), value.w());
    m_keyframeRef->setInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(type), v);
}

BaseMotionTrack *CameraKeyframeRefObject::parentTrack() const
{
    return m_parentTrackRef;
}

CameraRefObject *CameraKeyframeRefObject::parentCamera() const
{
    return m_parentTrackRef->parentCamera();
}

QObject *CameraKeyframeRefObject::opaque() const
{
    return parentCamera();
}

QString CameraKeyframeRefObject::name() const
{
    return QString();
}

void CameraKeyframeRefObject::setName(const QString &value)
{
    Q_UNUSED(value);
}

QVector3D CameraKeyframeRefObject::lookAt() const
{
    return Util::fromVector3(m_keyframeRef->lookAt());
}

void CameraKeyframeRefObject::setLookAt(const QVector3D &value)
{
    if (!qFuzzyCompare(value, lookAt())) {
        m_keyframeRef->setLookAt(Util::toVector3(value));
        emit lookAtChanged();
    }
}

QVector3D CameraKeyframeRefObject::angle() const
{
    return Util::fromVector3(m_keyframeRef->angle());
}

void CameraKeyframeRefObject::setAngle(const QVector3D &value)
{
    if (!qFuzzyCompare(value, angle())) {
        m_keyframeRef->setAngle(Util::toVector3(value));
        emit angleChanged();
    }
}

qreal CameraKeyframeRefObject::distance() const
{
    return static_cast<qreal>(m_keyframeRef->distance());
}

void CameraKeyframeRefObject::setDistance(const qreal &value)
{
    if (!qFuzzyCompare(value, distance())) {
        m_keyframeRef->setDistance(static_cast<Scalar>(value));
        emit distanceChanged();
    }
}

qreal CameraKeyframeRefObject::fov() const
{
    return static_cast<qreal>(m_keyframeRef->fov());
}

void CameraKeyframeRefObject::setFov(const qreal &value)
{
    if (!qFuzzyCompare(value, fov())) {
        m_keyframeRef->setFov(static_cast<Scalar>(value));
        emit fovChanged();
    }
}

ICameraKeyframe *CameraKeyframeRefObject::data() const
{
    return m_keyframeRef;
}

IKeyframe *CameraKeyframeRefObject::baseKeyframeData() const
{
    return data();
}
