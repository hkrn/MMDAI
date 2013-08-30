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

#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "MotionProxy.h"

#include <vpvl2/vpvl2.h>
#include <QtCore>

using namespace vpvl2;

BoneMotionTrack::BoneMotionTrack(MotionProxy *motionProxy, const QString &name)
    : BaseMotionTrack(motionProxy, name)
{
}

BoneMotionTrack::~BoneMotionTrack()
{
    m_timeIndex2RefObjects.clear();
    m_keyframe2RefObjects.clear();
    qDeleteAll(m_keyframes);
    m_keyframes.clear();
    m_parentMotionRef = 0;
}

BaseKeyframeRefObject *BoneMotionTrack::convert(IKeyframe *value)
{
    if (value->type() == IKeyframe::kBoneKeyframe) {
        IBoneKeyframe *keyframe = static_cast<IBoneKeyframe *>(value);
        return convertBoneKeyframe(keyframe);
    }
    return 0;
}

BoneKeyframeRefObject *BoneMotionTrack::convertBoneKeyframe(IBoneKeyframe *keyframe)
{
    Q_ASSERT(keyframe);
    if (BaseKeyframeRefObject *keyframeRefObject = m_keyframe2RefObjects.value(keyframe)) {
        BoneKeyframeRefObject *boneKeyframeRefObject = qobject_cast<BoneKeyframeRefObject *>(keyframeRefObject);
        Q_ASSERT(boneKeyframeRefObject);
        return boneKeyframeRefObject;
    }
    else if (keyframe) {
        BoneKeyframeRefObject *boneKeyframeRefObject = new BoneKeyframeRefObject(this, keyframe);
        m_keyframe2RefObjects.insert(keyframe, boneKeyframeRefObject);
        return boneKeyframeRefObject;
    }
    return 0;
}

void BoneMotionTrack::addKeyframe(BoneKeyframeRefObject *keyframe, bool doUpdate)
{
    Q_ASSERT(keyframe);
    IMotion *motionRef = m_parentMotionRef->data();
    motionRef->addKeyframe(keyframe->data());
    if (doUpdate) {
        motionRef->update(type());
    }
    add(keyframe, doUpdate);
    emit keyframeDidAdd(keyframe);
}

void BoneMotionTrack::addKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<BoneKeyframeRefObject *>(value));
    if (BoneKeyframeRefObject *v = qobject_cast<BoneKeyframeRefObject *>(value)) {
        addKeyframe(v, doUpdate);
    }
}

void BoneMotionTrack::removeKeyframe(BoneKeyframeRefObject *keyframe, bool doUpdate)
{
    Q_ASSERT(keyframe);
    IMotion *motionRef = m_parentMotionRef->data();
    motionRef->removeKeyframe(keyframe->data());
    if (doUpdate) {
        motionRef->update(type());
    }
    remove(keyframe);
    emit keyframeDidRemove(keyframe);
}

void BoneMotionTrack::removeKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<BoneKeyframeRefObject *>(value));
    if (BoneKeyframeRefObject *v = qobject_cast<BoneKeyframeRefObject *>(value)) {
        removeKeyframe(v, doUpdate);
    }
}

BaseKeyframeRefObject *BoneMotionTrack::copy(BaseKeyframeRefObject *value, const qint64 &timeIndex, bool doUpdate)
{
    Q_ASSERT(value);
    BoneKeyframeRefObject *newKeyframe = 0;
    if (timeIndex != value->timeIndex()) {
        Q_ASSERT(qobject_cast<BoneKeyframeRefObject *>(value));
        if (BoneKeyframeRefObject *v = qobject_cast<BoneKeyframeRefObject *>(value)) {
            newKeyframe = convertBoneKeyframe(v->data()->clone());
            newKeyframe->setTimeIndex(static_cast<IKeyframe::TimeIndex>(timeIndex));
            addKeyframe(newKeyframe, doUpdate);
        }
    }
    return newKeyframe;
}

void BoneMotionTrack::replace(BoneKeyframeRefObject *dst, BoneKeyframeRefObject *src, bool doUpdate)
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

IKeyframe::Type BoneMotionTrack::type() const
{
    return IKeyframe::kBoneKeyframe;
}
