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

#ifndef LIGHTKEYFRAMEREFOBJECT_H
#define LIGHTKEYFRAMEREFOBJECT_H

#include "BaseKeyframeRefObject.h"

#include <QVector3D>

class LightMotionTrack;
class LightRefObject;

namespace vpvl2 {
class ILightKeyframe;
}

class LightKeyframeRefObject : public BaseKeyframeRefObject
{
    Q_OBJECT
    Q_PROPERTY(QVector3D color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged FINAL)

public:
    LightKeyframeRefObject(LightMotionTrack *trackRef, vpvl2::ILightKeyframe *data);
    ~LightKeyframeRefObject();

    BaseMotionTrack *parentTrack() const;
    LightRefObject *parentLight() const;
    QObject *opaque() const;
    QString name() const;
    void setName(const QString &value);

    QVector3D color() const;
    void setColor(const QVector3D &value);
    QVector3D direction() const;
    void setDirection(const QVector3D &value);
    vpvl2::ILightKeyframe *data() const;

protected:
    vpvl2::IKeyframe *baseKeyframeData() const;

signals:
    void colorChanged();
    void directionChanged();

private:
    LightMotionTrack *m_parentTrackRef;
    vpvl2::ILightKeyframe *m_keyframeRef;
};

#endif // LIGHTKEYFRAMEREFOBJECT_H
