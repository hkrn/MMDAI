/**

 Copyright (c) 2010-2014  hkrn

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

#include <QtCore>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

CameraKeyframeRefObject::CameraKeyframeRefObject(CameraMotionTrack *trackRef, ICameraKeyframe *data)
    : BaseKeyframeRefObject(trackRef->parentMotion()),
      m_parentTrackRef(trackRef),
      m_keyframe(data)
{
    Q_ASSERT(m_parentTrackRef);
    Q_ASSERT(m_keyframe);
}

CameraKeyframeRefObject::~CameraKeyframeRefObject()
{
    if (isDeleteable()) {
        delete m_keyframe;
    }
    m_parentTrackRef = 0;
    m_keyframe = 0;
}

QJsonValue CameraKeyframeRefObject::toJson() const
{
    QJsonObject v = BaseKeyframeRefObject::toJson().toObject(), i;
    v.insert("lookAt", Util::toJson(lookAt()));
    v.insert("angle", Util::toJson(angle()));
    v.insert("fov", fov());
    v.insert("distance", distance());
    addInterpolationParameterToJson("lookAtX", ICameraKeyframe::kCameraLookAtX, i);
    addInterpolationParameterToJson("lookAtY", ICameraKeyframe::kCameraLookAtY, i);
    addInterpolationParameterToJson("lookAtZ", ICameraKeyframe::kCameraLookAtZ, i);
    addInterpolationParameterToJson("angle", ICameraKeyframe::kCameraAngle, i);
    addInterpolationParameterToJson("fov", ICameraKeyframe::kCameraFov, i);
    addInterpolationParameterToJson("distance", ICameraKeyframe::kCameraDistance, i);
    v.insert("interpolation", i);
    return v;
}

QVector4D CameraKeyframeRefObject::interpolationParameter(int type) const
{
    Q_ASSERT(m_keyframe);
    QuadWord value;
    m_keyframe->getInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(type), value);
    return QVector4D(value.x(), value.y(), value.z(), value.w());
}

void CameraKeyframeRefObject::setInterpolationParameter(int type, const QVector4D &value)
{
    Q_ASSERT(m_keyframe);
    QuadWord v(value.x(), value.y(), value.z(), value.w());
    m_keyframe->setInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(type), v);
}

BaseMotionTrack *CameraKeyframeRefObject::parentTrack() const
{
    return m_parentTrackRef;
}

CameraRefObject *CameraKeyframeRefObject::parentCamera() const
{
    Q_ASSERT(m_parentTrackRef);
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
    Q_ASSERT(m_keyframe);
    return Util::fromVector3(m_keyframe->lookAt());
}

void CameraKeyframeRefObject::setLookAt(const QVector3D &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, lookAt())) {
        m_keyframe->setLookAt(Util::toVector3(value));
        emit lookAtChanged();
    }
}

QVector3D CameraKeyframeRefObject::angle() const
{
    Q_ASSERT(m_keyframe);
    return Util::fromVector3(m_keyframe->angle());
}

void CameraKeyframeRefObject::setAngle(const QVector3D &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, angle())) {
        m_keyframe->setAngle(Util::toVector3(value));
        emit angleChanged();
    }
}

qreal CameraKeyframeRefObject::distance() const
{
    Q_ASSERT(m_keyframe);
    return static_cast<qreal>(m_keyframe->distance());
}

void CameraKeyframeRefObject::setDistance(const qreal &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, distance())) {
        m_keyframe->setDistance(static_cast<Scalar>(value));
        emit distanceChanged();
    }
}

qreal CameraKeyframeRefObject::fov() const
{
    Q_ASSERT(m_keyframe);
    return static_cast<qreal>(m_keyframe->fov());
}

void CameraKeyframeRefObject::setFov(const qreal &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, fov())) {
        m_keyframe->setFov(static_cast<Scalar>(value));
        emit fovChanged();
    }
}

ICameraKeyframe *CameraKeyframeRefObject::data() const
{
    return m_keyframe;
}

IKeyframe *CameraKeyframeRefObject::baseKeyframeData() const
{
    return data();
}

void CameraKeyframeRefObject::addInterpolationParameterToJson(const QString &key, int type, QJsonObject &v) const
{
    const QVector4D &v2 = interpolationParameter(type);
    QJsonObject i;
    i.insert("x1", v2.x());
    i.insert("y1", v2.y());
    i.insert("x2", v2.z());
    i.insert("y2", v2.w());
    v.insert(key, i);
}
