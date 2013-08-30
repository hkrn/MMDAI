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

#ifndef CAMERAKEYFRAMEREFOBJECT_H
#define CAMERAKEYFRAMEREFOBJECT_H

#include "BaseKeyframeRefObject.h"

#include <QVector3D>
#include <QVector4D>

class CameraMotionTrack;
class CameraRefObject;

namespace vpvl2 {
class ICameraKeyframe;
}

class CameraKeyframeRefObject : public BaseKeyframeRefObject
{
    Q_OBJECT
    Q_PROPERTY(QVector3D lookAt READ lookAt WRITE setLookAt NOTIFY lookAtChanged FINAL)
    Q_PROPERTY(QVector3D angle READ angle WRITE setAngle NOTIFY angleChanged FINAL)
    Q_PROPERTY(qreal distance READ distance WRITE setDistance NOTIFY distanceChanged FINAL)
    Q_PROPERTY(qreal fov READ fov WRITE setFov NOTIFY fovChanged FINAL)

public:
    CameraKeyframeRefObject(CameraMotionTrack *trackRef, vpvl2::ICameraKeyframe *data);
    ~CameraKeyframeRefObject();

    Q_INVOKABLE QVector4D interpolationParameter(int type) const;
    Q_INVOKABLE void setInterpolationParameter(int type, const QVector4D &value);

    BaseMotionTrack *parentTrack() const;
    CameraRefObject *parentCamera() const;
    QObject *opaque() const;
    QString name() const;
    void setName(const QString &value);

    QVector3D lookAt() const;
    void setLookAt(const QVector3D &value);
    QVector3D angle() const;
    void setAngle(const QVector3D &value);
    qreal distance() const;
    void setDistance(const qreal &value);
    qreal fov() const;
    void setFov(const qreal &value);
    vpvl2::ICameraKeyframe *data() const;

protected:
    vpvl2::IKeyframe *baseKeyframeData() const;

signals:
    void lookAtChanged();
    void angleChanged();
    void distanceChanged();
    void fovChanged();

private:
    CameraMotionTrack *m_parentTrackRef;
    vpvl2::ICameraKeyframe *m_keyframeRef;
};

#endif // CAMERAKEYFRAMEREFOBJECT_H
