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

#include "LightMotionTrack.h"

#include "LightKeyframeRefObject.h"
#include "LightRefObject.h"
#include "MotionProxy.h"

#include <vpvl2/vpvl2.h>
#include <QString>

using namespace vpvl2;

LightMotionTrack::LightMotionTrack(MotionProxy *motionProxy, LightRefObject *cameraRef)
    : BaseMotionTrack(motionProxy, QString()),
      m_lightRef(cameraRef)
{
}

LightMotionTrack::~LightMotionTrack()
{
    m_timeIndex2RefObjects.clear();
    m_keyframe2RefObjects.clear();
    qDeleteAll(m_keyframes);
    m_keyframes.clear();
    m_parentMotionRef = 0;
    m_lightRef = 0;
}

BaseKeyframeRefObject *LightMotionTrack::copy(BaseKeyframeRefObject *value, const qint64 &timeIndex, bool doUpdate)
{
    Q_ASSERT(value);
    LightKeyframeRefObject *newKeyframe = 0;
    if (timeIndex != value->timeIndex()) {
        Q_ASSERT(qobject_cast<LightKeyframeRefObject *>(value));
        if (LightKeyframeRefObject *v = qobject_cast<LightKeyframeRefObject *>(value)) {
            newKeyframe = convertLightKeyframe(v->data()->clone());
            newKeyframe->setTimeIndex(static_cast<IKeyframe::TimeIndex>(timeIndex));
            addKeyframe(newKeyframe, doUpdate);
        }
    }
    return newKeyframe;
}

BaseKeyframeRefObject *LightMotionTrack::convert(IKeyframe *value)
{
    if (value->type() == IKeyframe::kLightKeyframe) {
        ILightKeyframe *keyframe = static_cast<ILightKeyframe *>(value);
        return convertLightKeyframe(keyframe);
    }
    return 0;
}

LightKeyframeRefObject *LightMotionTrack::convertLightKeyframe(ILightKeyframe *keyframe)
{
    Q_ASSERT(keyframe);
    if (BaseKeyframeRefObject *keyframeRefObject = m_keyframe2RefObjects.value(keyframe)) {
        LightKeyframeRefObject *lightKeyframeRefObject = qobject_cast<LightKeyframeRefObject *>(keyframeRefObject);
        Q_ASSERT(lightKeyframeRefObject);
        return lightKeyframeRefObject;
    }
    else if (keyframe) {
        LightKeyframeRefObject *lightKeyframeRefObject = new LightKeyframeRefObject(this, keyframe);
        m_keyframe2RefObjects.insert(keyframe, lightKeyframeRefObject);
        return lightKeyframeRefObject;
    }
    return 0;
}

void LightMotionTrack::addKeyframe(LightKeyframeRefObject *keyframe, bool doUpdate)
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

void LightMotionTrack::addKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<LightKeyframeRefObject *>(value));
    if (LightKeyframeRefObject *v = qobject_cast<LightKeyframeRefObject *>(value)) {
        addKeyframe(v, doUpdate);
    }
}

void LightMotionTrack::removeKeyframe(LightKeyframeRefObject *keyframe, bool doUpdate)
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

void LightMotionTrack::removeKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<LightKeyframeRefObject *>(value));
    if (LightKeyframeRefObject *v = qobject_cast<LightKeyframeRefObject *>(value)) {
        removeKeyframe(v, doUpdate);
    }
}

void LightMotionTrack::replace(LightKeyframeRefObject *dst, LightKeyframeRefObject *src, bool doUpdate)
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

IKeyframe::Type LightMotionTrack::type() const
{
    return IKeyframe::kLightKeyframe;
}

LightRefObject *LightMotionTrack::parentLight() const
{
    return m_lightRef;
}
