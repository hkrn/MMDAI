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

#include "CameraRefObject.h"

#include <vpvl2/vpvl2.h>
#include <QtCore>

#include "CameraMotionTrack.h"
#include "ModelProxy.h"
#include "MotionProxy.h"
#include "ProjectProxy.h"
#include "Util.h"

using namespace vpvl2;

CameraRefObject::CameraRefObject(ProjectProxy *project)
    : QObject(project),
      m_projectRef(project),
      m_motionRef(0),
      m_cameraRef(0),
      m_name(tr("Camera")),
      m_cameraTranslateRatio(100),
      m_index(0),
      m_seekable(false)
{
    connect(this, &CameraRefObject::cameraDidReset, this, &CameraRefObject::lookAtChanged);
    connect(this, &CameraRefObject::cameraDidReset, this, &CameraRefObject::angleChanged);
    connect(this, &CameraRefObject::cameraDidReset, this, &CameraRefObject::distanceChanged);
    connect(this, &CameraRefObject::cameraDidReset, this, &CameraRefObject::fovChanged);
}

CameraRefObject::~CameraRefObject()
{
    releaseMotion();
    m_cameraRef = 0;
    m_motionRef = 0;
    m_projectRef = 0;
    m_cameraTranslateRatio = 0;
    m_index = 0;
    m_seekable = false;
}

void CameraRefObject::translate(qreal x, qreal y)
{
    Q_ASSERT(m_cameraRef);
    const Matrix3x3 &cameraTransformMatrix = m_cameraRef->modelViewTransform().getBasis();
    const Scalar &ratio = m_cameraRef->distance() / m_cameraTranslateRatio;
    Vector3 newDelta(kZeroV3);
    newDelta += cameraTransformMatrix[0] * x;
    newDelta += cameraTransformMatrix[1] * y;
    newDelta *= ratio;
    setLookAt(m_lookAt + Util::fromVector3(newDelta));
}

void CameraRefObject::rotate(qreal x, qreal y)
{
    setAngle(m_angle + QVector3D(y, x, 0));
}

void CameraRefObject::zoom(qreal value)
{
    Q_ASSERT(m_cameraRef);
    setDistance(m_cameraRef->distance() + value);
}

void CameraRefObject::setPreset(PresetType value)
{
    switch (value) {
    case FrontPreset:
        setAngle(QVector3D(0, 0, 0));
        break;
    case BackPreset:
        setAngle(QVector3D(0, 180, 0));
        break;
    case TopPreset:
        setAngle(QVector3D(90, 0, 0));
        break;
    case RightPreset:
        setAngle(QVector3D(0, -90, 0));
        break;
    case LeftPreset:
        setAngle(QVector3D(0, 90, 0));
        break;
    default:
        break;
    }
}

void CameraRefObject::reset()
{
    Q_ASSERT(m_cameraRef);
    m_cameraRef->resetDefault();
    refresh();
}

void CameraRefObject::assignCameraRef(ICamera *cameraRef, MotionProxy *motionProxyRef)
{
    Q_ASSERT(cameraRef);
    cameraRef->setMotion(motionProxyRef->data());
    m_track.reset(new CameraMotionTrack(motionProxyRef, this));
    motionProxyRef->setCameraMotionTrack(m_track.data(), m_projectRef->factoryInstanceRef());
    m_motionRef = motionProxyRef;
    m_cameraRef = cameraRef;
    emit motionChanged();
    refresh();
}

void CameraRefObject::refresh()
{
    Q_ASSERT(m_cameraRef);
    m_lookAt = Util::fromVector3(m_cameraRef->lookAt());
    m_angle = Util::fromVector3(m_cameraRef->angle());
    emit cameraDidReset();
}

MotionProxy *CameraRefObject::releaseMotion()
{
    MotionProxy *previousMotionRef = m_motionRef;
    if (previousMotionRef) {
        m_cameraRef->setMotion(0);
        m_track.reset();
        m_motionRef = 0;
    }
    return previousMotionRef;
}

ProjectProxy *CameraRefObject::project() const
{
    return m_projectRef;
}

MotionProxy *CameraRefObject::motion() const
{
    return m_motionRef;
}

CameraMotionTrack *CameraRefObject::track() const
{
    return m_track.data();
}

ICamera *CameraRefObject::data() const
{
    return m_cameraRef;
}

QVector3D CameraRefObject::lookAt() const
{
    return m_lookAt;
}

void CameraRefObject::setLookAt(const QVector3D &value)
{
    Q_ASSERT(m_cameraRef);
    if (!qFuzzyCompare(value, lookAt())) {
        m_cameraRef->setLookAt(Util::toVector3(value));
        m_lookAt = value;
        emit lookAtChanged();
    }
}

QVector3D CameraRefObject::angle() const
{
    return m_angle;
}

void CameraRefObject::setAngle(const QVector3D &value)
{
    Q_ASSERT(m_cameraRef);
    if (!qFuzzyCompare(value, angle())) {
        m_cameraRef->setAngle(Util::toVector3(value));
        m_angle = value;
        emit angleChanged();
    }
}

qreal CameraRefObject::distance() const
{
    Q_ASSERT(m_cameraRef);
    return m_cameraRef->distance();
}

void CameraRefObject::setDistance(const qreal &value)
{
    Q_ASSERT(m_cameraRef);
    if (!qFuzzyCompare(value, distance())) {
        m_cameraRef->setDistance(value);
        emit distanceChanged();
    }
}

qreal CameraRefObject::fov() const
{
    Q_ASSERT(m_cameraRef);
    return m_cameraRef->fov();
}

void CameraRefObject::setFov(const qreal &value)
{
    Q_ASSERT(m_cameraRef);
    if (!qFuzzyCompare(value, fov())) {
        m_cameraRef->setFov(value);
        emit fovChanged();
    }
}

qreal CameraRefObject::translateRatio() const
{
    return m_cameraTranslateRatio;
}

void CameraRefObject::setTranslateRatio(qreal value)
{
    if (!qFuzzyCompare(value, translateRatio())) {
        m_cameraTranslateRatio = value;
        emit translateRatioChanged();
    }
}

bool CameraRefObject::isSeekable() const
{
    return m_seekable;
}

void CameraRefObject::setSeekable(bool value)
{
    if (value != isSeekable()) {
        m_seekable = value;
        emit seekableChanged();
    }
}
