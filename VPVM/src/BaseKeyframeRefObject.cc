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

#include "BaseKeyframeRefObject.h"
#include "BaseMotionTrack.h"
#include "MotionProxy.h"

#include <cmath>
#include <QtCore>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

BaseKeyframeRefObject::BaseKeyframeRefObject(MotionProxy *motionRef)
    : QObject(motionRef),
      m_parentMotionRef(motionRef)
{
    Q_ASSERT(m_parentMotionRef);
}

BaseKeyframeRefObject::~BaseKeyframeRefObject()
{
    m_parentMotionRef = 0;
}

MotionProxy *BaseKeyframeRefObject::parentMotion() const
{
    return m_parentMotionRef;
}

qreal BaseKeyframeRefObject::time() const
{
    return timeIndex() / Scene::defaultFPS();
}

void BaseKeyframeRefObject::setTime(qreal value)
{
    setTimeIndex(value * Scene::defaultFPS());
}

qint64 BaseKeyframeRefObject::timeIndex() const
{
    return static_cast<qint64>(baseKeyframeData()->timeIndex());
}

void BaseKeyframeRefObject::setTimeIndex(const qint64 &value)
{
    IKeyframe *keyframe = baseKeyframeData();
    if (value != keyframe->timeIndex()) {
        /* after calling BaseKeyframeRefObject#setTimeIndex, you should call BaseMotionTrack#replaceTimeIndex too */
        keyframe->setTimeIndex(static_cast<IKeyframe::TimeIndex>(value));
        emit timeIndexChanged();
    }
}

int BaseKeyframeRefObject::layerIndex() const
{
    return static_cast<int>(baseKeyframeData()->layerIndex());
}

void BaseKeyframeRefObject::setLayerIndex(int value)
{
    IKeyframe *keyframe = baseKeyframeData();
    if (value != keyframe->layerIndex()) {
        keyframe->setLayerIndex(static_cast<IKeyframe::LayerIndex>(value));
        emit layerIndexChanged();
    }
}
