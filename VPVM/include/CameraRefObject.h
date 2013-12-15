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

#ifndef CAMERAREFOBJECT_H
#define CAMERAREFOBJECT_H

#include <QObject>
#include <QVector3D>
#include <QUuid>

class CameraMotionTrack;
class ModelProxy;
class MotionProxy;
class ProjectProxy;

namespace vpvl2 {
class ICamera;
}

class CameraRefObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(PresetType)
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int index MEMBER m_index)
    Q_PROPERTY(ProjectProxy *project READ project CONSTANT FINAL)
    Q_PROPERTY(MotionProxy *motion READ motion NOTIFY motionChanged FINAL)
    Q_PROPERTY(CameraMotionTrack *track READ track CONSTANT FINAL)
    Q_PROPERTY(QVector3D lookAt READ lookAt WRITE setLookAt NOTIFY lookAtChanged FINAL)
    Q_PROPERTY(QVector3D angle READ angle WRITE setAngle NOTIFY angleChanged FINAL)
    Q_PROPERTY(qreal distance READ distance WRITE setDistance NOTIFY distanceChanged FINAL)
    Q_PROPERTY(qreal fov READ fov WRITE setFov NOTIFY fovChanged FINAL)
    Q_PROPERTY(qreal translateRatio READ translateRatio WRITE setTranslateRatio NOTIFY translateRatioChanged FINAL)
    Q_PROPERTY(bool seekable READ isSeekable WRITE setSeekable NOTIFY seekableChanged)

public:
    enum PresetType {
        FrontPreset,
        BackPreset,
        TopPreset,
        LeftPreset,
        RightPreset
    };
    explicit CameraRefObject(ProjectProxy *project);
    ~CameraRefObject();

    Q_INVOKABLE void translate(qreal x, qreal y);
    Q_INVOKABLE void rotate(qreal x, qreal y);
    Q_INVOKABLE void zoom(qreal value);
    Q_INVOKABLE void setPreset(PresetType value);
    Q_INVOKABLE void reset();
    void assignCameraRef(vpvl2::ICamera *cameraRef, MotionProxy *motionProxyRef);
    void refresh();
    MotionProxy *releaseMotion();

    ProjectProxy *project() const;
    MotionProxy *motion() const;
    CameraMotionTrack *track() const;
    vpvl2::ICamera *data() const;
    QVector3D lookAt() const;
    void setLookAt(const QVector3D &value);
    QVector3D angle() const;
    void setAngle(const QVector3D &value);
    qreal distance() const;
    void setDistance(const qreal &value);
    qreal fov() const;
    void setFov(const qreal &value);
    qreal translateRatio() const;
    void setTranslateRatio(qreal value);
    bool isSeekable() const;
    void setSeekable(bool value);

signals:
    void motionChanged();
    void lookAtChanged();
    void angleChanged();
    void distanceChanged();
    void fovChanged();
    void cameraDidReset();
    void translateRatioChanged();
    void seekableChanged();

private:
    ProjectProxy *m_projectRef;
    MotionProxy *m_motionRef;
    vpvl2::ICamera *m_cameraRef;
    QScopedPointer<CameraMotionTrack> m_track;
    QVector3D m_lookAt;
    QVector3D m_angle;
    QString m_name;
    qreal m_cameraTranslateRatio;
    int m_index;
    bool m_seekable;
};

#endif // MORPHREFOBJECT_H
