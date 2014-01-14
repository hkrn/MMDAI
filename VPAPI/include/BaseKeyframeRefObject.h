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

#ifndef BASEKEYFRAMEREFOBJECT_H
#define BASEKEYFRAMEREFOBJECT_H

#include <QObject>
#include <QString>
#include <vpvl2/IKeyframe.h>

class BaseMotionTrack;
class MotionProxy;

namespace vpvl2 {
class IKeyframe;
}

class BaseKeyframeRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(BaseMotionTrack *parentTrack READ parentTrack CONSTANT FINAL)
    Q_PROPERTY(MotionProxy *parentMotion READ parentMotion CONSTANT FINAL)
    Q_PROPERTY(QObject *opaque READ opaque CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(qreal time READ time WRITE setTime FINAL)
    Q_PROPERTY(qreal timeIndex READ timeIndex WRITE setTimeIndex NOTIFY timeIndexChanged FINAL)
    Q_PROPERTY(int layerIndex READ layerIndex WRITE setLayerIndex NOTIFY layerIndexChanged FINAL)

public:
    explicit BaseKeyframeRefObject(MotionProxy *motionRef);
    ~BaseKeyframeRefObject();

    MotionProxy *parentMotion() const;
    qreal time() const;
    void setTime(qreal value);
    qint64 timeIndex() const;
    void setTimeIndex(const qint64 &value);
    int layerIndex() const;
    void setLayerIndex(int value);

    virtual BaseMotionTrack *parentTrack() const = 0;
    virtual QObject *opaque() const = 0;
    virtual QString name() const = 0;
    virtual void setName(const QString &value) = 0;
    virtual vpvl2::IKeyframe *baseKeyframeData() const = 0;

signals:
    void timeIndexChanged();
    void layerIndexChanged();

private:
    MotionProxy *m_parentMotionRef;
};

#endif // BASEKEYFRAMEREFOBJECT_H
