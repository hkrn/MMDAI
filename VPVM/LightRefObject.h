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

#ifndef LIGHTREFOBJECT_H
#define LIGHTREFOBJECT_H

#include <QObject>
#include <QVector3D>

class LightMotionTrack;
class MotionProxy;
class ProjectProxy;

namespace vpvl2 {
class ILight;
}

class LightRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int index MEMBER m_index)
    Q_PROPERTY(ProjectProxy *project READ project CONSTANT FINAL)
    Q_PROPERTY(MotionProxy *motion READ motion NOTIFY motionChanged FINAL)
    Q_PROPERTY(LightMotionTrack *track READ track CONSTANT FINAL)
    Q_PROPERTY(QVector3D color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged FINAL)

public:
    explicit LightRefObject(ProjectProxy *project);
    ~LightRefObject();

    Q_INVOKABLE void reset();
    void assignLightRef(vpvl2::ILight *lightRef, MotionProxy *motionProxyRef);
    void refresh();
    MotionProxy *releaseMotion();

    ProjectProxy *project() const;
    MotionProxy *motion() const;
    LightMotionTrack *track() const;
    vpvl2::ILight *data() const;
    QVector3D color() const;
    void setColor(const QVector3D &value);
    QVector3D direction() const;
    void setDirection(const QVector3D &value);

signals:
    void motionChanged();
    void lightDidReset();
    void colorChanged();
    void directionChanged();

private:
    ProjectProxy *m_projectRef;
    MotionProxy *m_motionRef;
    vpvl2::ILight *m_lightRef;
    QScopedPointer<LightMotionTrack> m_track;
    QVector3D m_color;
    QVector3D m_direction;
    QString m_name;
    int m_index;
};

#endif // LIGHTREFOBJECT_H
