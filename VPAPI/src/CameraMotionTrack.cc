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

#include "CameraMotionTrack.h"

#include "CameraKeyframeRefObject.h"
#include "CameraRefObject.h"
#include "MotionProxy.h"

#include <vpvl2/vpvl2.h>
#include <QString>

using namespace vpvl2;

CameraMotionTrack::CameraMotionTrack(MotionProxy *motionProxy, CameraRefObject *cameraRef)
    : BaseMotionTrack(motionProxy, QString()),
      m_cameraRef(cameraRef)
{
}

CameraMotionTrack::~CameraMotionTrack()
{
    m_timeIndex2RefObjects.clear();
    m_keyframe2RefObjects.clear();
    qDeleteAll(m_keyframes);
    m_keyframes.clear();
    m_parentMotionRef = 0;
    m_cameraRef = 0;
}

BaseKeyframeRefObject *CameraMotionTrack::copy(BaseKeyframeRefObject *value, const qint64 &timeIndex, bool doUpdate)
{
    Q_ASSERT(value);
    CameraKeyframeRefObject *newKeyframe = 0;
    if (timeIndex != value->timeIndex()) {
        Q_ASSERT(qobject_cast<CameraKeyframeRefObject *>(value));
        if (CameraKeyframeRefObject *v = qobject_cast<CameraKeyframeRefObject *>(value)) {
            newKeyframe = convertCameraKeyframe(v->data()->clone());
            newKeyframe->setTimeIndex(static_cast<IKeyframe::TimeIndex>(timeIndex));
            addKeyframe(newKeyframe, doUpdate);
        }
    }
    return newKeyframe;
}

BaseKeyframeRefObject *CameraMotionTrack::convert(IKeyframe *value)
{
    Q_ASSERT(value);
    if (value->type() == IKeyframe::kCameraKeyframe) {
        ICameraKeyframe *keyframe = static_cast<ICameraKeyframe *>(value);
        return convertCameraKeyframe(keyframe);
    }
    return 0;
}

CameraKeyframeRefObject *CameraMotionTrack::convertCameraKeyframe(ICameraKeyframe *keyframe)
{
    Q_ASSERT(keyframe);
    if (BaseKeyframeRefObject *keyframeRefObject = m_keyframe2RefObjects.value(keyframe)) {
        CameraKeyframeRefObject *cameraKeyframeRefObject = qobject_cast<CameraKeyframeRefObject *>(keyframeRefObject);
        Q_ASSERT(cameraKeyframeRefObject);
        return cameraKeyframeRefObject;
    }
    else if (keyframe) {
        CameraKeyframeRefObject *cameraKeyframeRefObject = new CameraKeyframeRefObject(this, keyframe);
        m_keyframe2RefObjects.insert(keyframe, cameraKeyframeRefObject);
        return cameraKeyframeRefObject;
    }
    return 0;
}

void CameraMotionTrack::addKeyframe(CameraKeyframeRefObject *keyframe, bool doUpdate)
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

void CameraMotionTrack::addKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<CameraKeyframeRefObject *>(value));
    if (CameraKeyframeRefObject *v = qobject_cast<CameraKeyframeRefObject *>(value)) {
        addKeyframe(v, doUpdate);
    }
}

void CameraMotionTrack::removeKeyframe(CameraKeyframeRefObject *keyframe, bool doUpdate)
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

void CameraMotionTrack::removeKeyframe(QObject *value, bool doUpdate)
{
    Q_ASSERT(qobject_cast<CameraKeyframeRefObject *>(value));
    if (CameraKeyframeRefObject *v = qobject_cast<CameraKeyframeRefObject *>(value)) {
        removeKeyframe(v, doUpdate);
    }
}

void CameraMotionTrack::replace(CameraKeyframeRefObject *dst, CameraKeyframeRefObject *src, bool doUpdate)
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

IKeyframe::Type CameraMotionTrack::type() const
{
    return IKeyframe::kCameraKeyframe;
}

CameraRefObject *CameraMotionTrack::parentCamera() const
{
    return m_cameraRef;
}
