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

#ifndef BONEKEYFRAMEREFOBJECT_H
#define BONEKEYFRAMEREFOBJECT_H

#include "BaseKeyframeRefObject.h"

#include <QQuaternion>
#include <QVector3D>

class BoneMotionTrack;
class BoneRefObject;

namespace vpvl2 {
class IBoneKeyframe;
}

class BoneKeyframeRefObject : public BaseKeyframeRefObject
{
    Q_OBJECT
    Q_PROPERTY(BoneRefObject *parentBone READ parentBone CONSTANT FINAL)
    Q_PROPERTY(QVector3D localTranslation READ localTranslation WRITE setLocalTranslation NOTIFY localTranslationChanged FINAL)
    Q_PROPERTY(QQuaternion localOrientation READ localOrientation WRITE setLocalOrientation NOTIFY localRotationChanged FINAL)

public:
    BoneKeyframeRefObject(BoneMotionTrack *trackRef, vpvl2::IBoneKeyframe *data);
    ~BoneKeyframeRefObject();

    Q_INVOKABLE QVector4D interpolationParameter(int type) const;
    Q_INVOKABLE void setInterpolationParameter(int type, const QVector4D &value);

    BaseMotionTrack *parentTrack() const;
    BoneRefObject *parentBone() const;
    QObject *opaque() const;
    QString name() const;
    void setName(const QString &value);
    QVector3D localTranslation() const;
    void setLocalTranslation(const QVector3D &value);
    QQuaternion localOrientation() const;
    void setLocalOrientation(const QQuaternion &value);

    vpvl2::IBoneKeyframe *data() const;

protected:
    vpvl2::IKeyframe *baseKeyframeData() const;

signals:
    void localTranslationChanged();
    void localRotationChanged();

private:
    BoneMotionTrack *m_parentTrackRef;
    vpvl2::IBoneKeyframe *m_keyframeRef;
};

#endif // BONEKEYFRAMEREFOBJECT_H
