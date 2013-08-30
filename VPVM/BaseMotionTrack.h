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

#ifndef BASEMOTIONTRACK_H
#define BASEMOTIONTRACK_H

#include <QObject>
#include <QHash>

#include <vpvl2/IKeyframe.h>

class BaseKeyframeRefObject;
class MotionProxy;

class BaseMotionTrack : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MotionProxy *parentMotion READ parentMotion CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int length READ length CONSTANT FINAL)
    Q_PROPERTY(bool locked MEMBER m_locked NOTIFY lockedChanged FINAL)
    Q_PROPERTY(bool visible MEMBER m_visible NOTIFY visibleChanged FINAL)

public:
    BaseMotionTrack(MotionProxy *motionProxy, const QString &name);
    virtual ~BaseMotionTrack();

    Q_INVOKABLE BaseKeyframeRefObject *findKeyframeAt(int index) const;
    Q_INVOKABLE BaseKeyframeRefObject *findKeyframeByTimeIndex(const qint64 &timeIndex) const;

    bool contains(BaseKeyframeRefObject *value) const;
    bool containsKeyframe(const vpvl2::IKeyframe *keyframe) const;
    void add(BaseKeyframeRefObject *value, bool doSort);
    void remove(BaseKeyframeRefObject *value);
    void replaceTimeIndex(const qint64 &newTimeIndex, const qint64 &oldTimeIndex);
    void refresh();
    void sort();

    virtual BaseKeyframeRefObject *copy(BaseKeyframeRefObject *value, const qint64 &timeIndex, bool doUpdate) = 0;
    virtual BaseKeyframeRefObject *convert(vpvl2::IKeyframe *value) = 0;
    virtual void addKeyframe(QObject *value, bool doUpdate) = 0;
    virtual void removeKeyframe(QObject *value, bool doUpdate) = 0;

    MotionProxy *parentMotion() const;
    QString name() const;
    int length() const;

    virtual vpvl2::IKeyframe::Type type() const = 0;

signals:
    void keyframeDidAdd(BaseKeyframeRefObject *keyframe);
    void keyframeDidRemove(BaseKeyframeRefObject *keyframe);
    void keyframeDidSwap(BaseKeyframeRefObject *dst, BaseKeyframeRefObject *src);
    void timeIndexDidChange(BaseKeyframeRefObject *keyframe, qint64 newTimeIndex, qint64 oldTimeIndex);
    void lockedChanged();
    void visibleChanged();

protected:
    void internalAdd(BaseKeyframeRefObject *value);
    void internalRemove(BaseKeyframeRefObject *value);

    typedef QList<BaseKeyframeRefObject *> BaseKeyframeRefObjectList;
    MotionProxy *m_parentMotionRef;
    BaseKeyframeRefObjectList m_keyframes;
    QHash<const qint64, BaseKeyframeRefObject *> m_timeIndex2RefObjects;
    QHash<const vpvl2::IKeyframe *, BaseKeyframeRefObject *> m_keyframe2RefObjects;
    const QString m_name;
    bool m_locked;
    bool m_visible;
};

#endif // BASEMOTIONTRACK_H
