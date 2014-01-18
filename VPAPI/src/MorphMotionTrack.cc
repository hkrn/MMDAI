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

#include "BaseMotionTrack.h"
#include "MorphKeyframeRefObject.h"
#include "MorphMotionTrack.h"
#include "MotionProxy.h"

#include <vpvl2/vpvl2.h>
#include <QtCore>

using namespace vpvl2;

MorphMotionTrack::MorphMotionTrack(MotionProxy *motionProxy, const QString &name)
    : BaseMotionTrack(motionProxy, name)
{
}

MorphMotionTrack::~MorphMotionTrack()
{
    qDeleteAll(m_keyframes);
    m_parentMotionRef = 0;
}

BaseKeyframeRefObject *MorphMotionTrack::convert(IKeyframe *value)
{
    Q_ASSERT(value);
    if (value->type() == IKeyframe::kMorphKeyframe) {
        IMorphKeyframe *keyframe = static_cast<IMorphKeyframe *>(value);
        return convertMorphKeyframe(keyframe);
    }
    return 0;
}

MorphKeyframeRefObject *MorphMotionTrack::convertMorphKeyframe(IMorphKeyframe *keyframe)
{
    Q_ASSERT(keyframe);
    if (BaseKeyframeRefObject *keyframeRefObject = m_keyframe2RefObjects.value(keyframe)) {
        MorphKeyframeRefObject *morphKeyframeRefObject = qobject_cast<MorphKeyframeRefObject *>(keyframeRefObject);
        Q_ASSERT(morphKeyframeRefObject);
        return morphKeyframeRefObject;
    }
    else if (keyframe) {
        MorphKeyframeRefObject *morphKeyframeRefObject = new MorphKeyframeRefObject(this, keyframe);
        m_keyframe2RefObjects.insert(keyframe, morphKeyframeRefObject);
        return morphKeyframeRefObject;
    }
    return 0;
}

void MorphMotionTrack::addKeyframe(MorphKeyframeRefObject *keyframe, bool doUpdate)
{
    Q_ASSERT(m_parentMotionRef);
    Q_ASSERT(keyframe);
    IMotion *motionRef = m_parentMotionRef->data();
    motionRef->addKeyframe(keyframe->data());
    if (doUpdate) {
        motionRef->update(type());
    }
    add(keyframe, doUpdate);
    emit keyframeDidAdd(keyframe);
}

void MorphMotionTrack::addKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<MorphKeyframeRefObject *>(value));
    if (MorphKeyframeRefObject *v = qobject_cast<MorphKeyframeRefObject *>(value)) {
        addKeyframe(v, doUpdate);
    }
}

void MorphMotionTrack::removeKeyframe(MorphKeyframeRefObject *keyframe, bool doUpdate)
{
    Q_ASSERT(m_parentMotionRef);
    Q_ASSERT(keyframe);
    IMotion *motionRef = m_parentMotionRef->data();
    motionRef->removeKeyframe(keyframe->data());
    if (doUpdate) {
        motionRef->update(type());
    }
    remove(keyframe);
    emit keyframeDidRemove(keyframe);
}

void MorphMotionTrack::removeKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<MorphKeyframeRefObject *>(value));
    if (MorphKeyframeRefObject *v = qobject_cast<MorphKeyframeRefObject *>(value)) {
        removeKeyframe(v, doUpdate);
    }
}

BaseKeyframeRefObject *MorphMotionTrack::copy(BaseKeyframeRefObject *value, const qint64 &timeIndex, bool doUpdate)
{
    Q_ASSERT(value);
    MorphKeyframeRefObject *newKeyframe = 0;
    if (timeIndex != value->timeIndex()) {
        Q_ASSERT(qobject_cast<MorphKeyframeRefObject *>(value));
        if (MorphKeyframeRefObject *v = qobject_cast<MorphKeyframeRefObject *>(value)) {
            newKeyframe = convertMorphKeyframe(v->data()->clone());
            newKeyframe->setTimeIndex(static_cast<IKeyframe::TimeIndex>(timeIndex));
            addKeyframe(newKeyframe, doUpdate);
        }
    }
    return newKeyframe;
}

void MorphMotionTrack::replace(MorphKeyframeRefObject *dst, MorphKeyframeRefObject *src, bool doUpdate)
{
    Q_ASSERT(src && dst && dst != src && src->parentTrack() == dst->parentTrack());
    IMotion *motionRef = m_parentMotionRef->data();
    motionRef->replaceKeyframe(dst->data(), false);
    if (doUpdate) {
        motionRef->update(type());
    }
    remove(src);
    add(dst, doUpdate);
    emit keyframeDidSwap(dst, src);
}

IKeyframe::Type MorphMotionTrack::type() const
{
    return IKeyframe::kMorphKeyframe;
}
