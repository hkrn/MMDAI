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

#include "LightKeyframeRefObject.h"

#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "Util.h"
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

LightKeyframeRefObject::LightKeyframeRefObject(LightMotionTrack *trackRef, ILightKeyframe *data)
    : BaseKeyframeRefObject(trackRef->parentMotion()),
      m_parentTrackRef(trackRef),
      m_keyframe(data)
{
    Q_ASSERT(m_parentTrackRef);
    Q_ASSERT(m_keyframe);
}

LightKeyframeRefObject::~LightKeyframeRefObject()
{
    if (isDeleteable()) {
        delete m_keyframe;
    }
    m_parentTrackRef = 0;
    m_keyframe = 0;
}

BaseMotionTrack *LightKeyframeRefObject::parentTrack() const
{
    return m_parentTrackRef;
}

LightRefObject *LightKeyframeRefObject::parentLight() const
{
    Q_ASSERT(m_parentTrackRef);
    return m_parentTrackRef->parentLight();
}

QObject *LightKeyframeRefObject::opaque() const
{
    return parentLight();
}

QString LightKeyframeRefObject::name() const
{
    return QString();
}

void LightKeyframeRefObject::setName(const QString &value)
{
    Q_UNUSED(value);
}

QVector3D LightKeyframeRefObject::color() const
{
    Q_ASSERT(m_keyframe);
    return Util::fromVector3(m_keyframe->color());
}

void LightKeyframeRefObject::setColor(const QVector3D &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, color())) {
        m_keyframe->setColor(Util::toVector3(value));
        emit colorChanged();
    }
}

QVector3D LightKeyframeRefObject::direction() const
{
    Q_ASSERT(m_keyframe);
    return Util::fromVector3(m_keyframe->direction());
}

void LightKeyframeRefObject::setDirection(const QVector3D &value)
{
    Q_ASSERT(m_keyframe);
    if (!qFuzzyCompare(value, direction())) {
        m_keyframe->setDirection(Util::toVector3(value));
        emit directionChanged();
    }
}

ILightKeyframe *LightKeyframeRefObject::data() const
{
    return m_keyframe;
}

IKeyframe *LightKeyframeRefObject::baseKeyframeData() const
{
    return data();
}
