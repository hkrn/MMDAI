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

#include "BaseKeyframeRefObject.h"
#include "BaseMotionTrack.h"
#include "MotionProxy.h"

#include <QtCore>

#include <cmath>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

BaseMotionTrack::BaseMotionTrack(MotionProxy *motionProxy, const QString &name)
    : QObject(motionProxy),
      m_parentMotionRef(motionProxy),
      m_name(name),
      m_locked(false),
      m_visible(true)
{
    Q_ASSERT(m_parentMotionRef);
}

BaseMotionTrack::~BaseMotionTrack()
{
    m_parentMotionRef = 0;
    m_locked = false;
    m_visible = false;
}

BaseKeyframeRefObject *BaseMotionTrack::findKeyframeAt(int index) const
{
    return m_keyframes.at(index);
}

BaseKeyframeRefObject *BaseMotionTrack::findKeyframeByTimeIndex(const qint64 &timeIndex) const
{
    return m_timeIndex2RefObjects.value(timeIndex);
}

bool BaseMotionTrack::contains(BaseKeyframeRefObject *value) const
{
    BaseKeyframeRefObjectList::ConstIterator it = qBinaryFind(m_keyframes, value);
    return it != m_keyframes.constEnd();
}

bool BaseMotionTrack::containsKeyframe(const IKeyframe *keyframe) const
{
    Q_ASSERT(keyframe);
    return m_keyframe2RefObjects.contains(keyframe);
}

void BaseMotionTrack::add(BaseKeyframeRefObject *value, bool doSort)
{
    internalAdd(value);
    if (doSort) {
        sort();
    }
}

void BaseMotionTrack::remove(BaseKeyframeRefObject *value)
{
    internalRemove(value);
}

void BaseMotionTrack::replaceTimeIndex(const qint64 &newTimeIndex, const qint64 &oldTimeIndex)
{
    BaseKeyframeRefObject *keyframeRef = 0;
    if (m_timeIndex2RefObjects.contains(oldTimeIndex)) {
        keyframeRef = m_timeIndex2RefObjects.value(oldTimeIndex);
        m_timeIndex2RefObjects.remove(oldTimeIndex);
    }
    if (keyframeRef) {
        m_timeIndex2RefObjects.insert(newTimeIndex, keyframeRef);
        emit timeIndexDidChange(keyframeRef, newTimeIndex, oldTimeIndex);
    }
}

void BaseMotionTrack::refresh()
{
    m_parentMotionRef->data()->update(type());
    sort();
}

void BaseMotionTrack::sort()
{
    qSort(m_keyframes);
}

MotionProxy *BaseMotionTrack::parentMotion() const
{
    return m_parentMotionRef;
}

QString BaseMotionTrack::name() const
{
    return m_name;
}

int BaseMotionTrack::length() const
{
    return m_keyframes.size();
}

void BaseMotionTrack::internalAdd(BaseKeyframeRefObject *value)
{
    Q_ASSERT(value);
    IKeyframe *keyframe = value->baseKeyframeData();
    m_keyframes.append(value);
    m_keyframe2RefObjects.insert(keyframe, value);
    m_timeIndex2RefObjects.insert(value->timeIndex(), value);
}

void BaseMotionTrack::internalRemove(BaseKeyframeRefObject *value)
{
    Q_ASSERT(value);
    BaseKeyframeRefObjectList::Iterator it = qBinaryFind(m_keyframes.begin(), m_keyframes.end(), value);
    if (it != m_keyframes.end()) {
        m_keyframes.erase(it);
    }
    IKeyframe *keyframe = value->baseKeyframeData();
    m_keyframe2RefObjects.remove(keyframe);
    m_timeIndex2RefObjects.remove(value->timeIndex());
}

