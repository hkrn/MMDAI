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

#include "ModelProxy.h"
#include "MorphKeyframeRefObject.h"
#include "MorphMotionTrack.h"
#include "MorphRefObject.h"
#include "MotionProxy.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions;

MorphKeyframeRefObject::MorphKeyframeRefObject(MorphMotionTrack *trackRef, IMorphKeyframe *keyframeRef)
    : BaseKeyframeRefObject(trackRef->parentMotion()),
      m_parentTrackRef(trackRef),
      m_keyframeRef(keyframeRef)
{
    Q_ASSERT(m_parentTrackRef);
    Q_ASSERT(m_keyframeRef);
}

MorphKeyframeRefObject::~MorphKeyframeRefObject()
{
    m_keyframeRef = 0;
}

BaseMotionTrack *MorphKeyframeRefObject::parentTrack() const
{
    return m_parentTrackRef;
}

MorphRefObject *MorphKeyframeRefObject::parentMorph() const
{
    if (ModelProxy *modelProxy = parentMotion()->parentModel()) {
        return modelProxy->findMorphByName(name());
    }
    return 0;
}

IKeyframe *MorphKeyframeRefObject::baseKeyframeData() const
{
    return data();
}

QObject *MorphKeyframeRefObject::opaque() const
{
    return parentMorph();
}

QString MorphKeyframeRefObject::name() const
{
    return Util::toQString(m_keyframeRef->name());
}

void MorphKeyframeRefObject::setName(const QString &value)
{
    if (!Util::equalsString(value, m_keyframeRef->name())) {
        qt::String s(value);
        m_keyframeRef->setName(&s);
    }
}

qreal MorphKeyframeRefObject::weight() const
{
    return static_cast<qreal>(m_keyframeRef->weight());
}

void MorphKeyframeRefObject::setWeight(const qreal &value)
{
    if (!qFuzzyCompare(value, weight())) {
        m_keyframeRef->setWeight(static_cast<IMorph::WeightPrecision>(value));
        emit weightChanged();
    }
}

IMorphKeyframe *MorphKeyframeRefObject::data() const
{
    return m_keyframeRef;
}
