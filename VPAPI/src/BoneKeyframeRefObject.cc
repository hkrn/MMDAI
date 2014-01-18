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
#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "ModelProxy.h"
#include "MotionProxy.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions;

BoneKeyframeRefObject::BoneKeyframeRefObject(BoneMotionTrack *trackRef, IBoneKeyframe *keyframeRef)
    : BaseKeyframeRefObject(trackRef->parentMotion()),
      m_parentTrackRef(trackRef),
      m_keyframeRef(keyframeRef)
{
    Q_ASSERT(m_parentTrackRef);
    Q_ASSERT(m_keyframeRef);
}

BoneKeyframeRefObject::~BoneKeyframeRefObject()
{
    m_parentTrackRef = 0;
    m_keyframeRef = 0;
}

QVector4D BoneKeyframeRefObject::interpolationParameter(int type) const
{
    Q_ASSERT(m_keyframeRef);
    QuadWord value;
    m_keyframeRef->getInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(type), value);
    return QVector4D(value.x(), value.y(), value.z(), value.w());
}

void BoneKeyframeRefObject::setInterpolationParameter(int type, const QVector4D &value)
{
    Q_ASSERT(m_keyframeRef);
    QuadWord v(value.x(), value.y(), value.z(), value.w());
    m_keyframeRef->setInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(type), v);
}

BaseMotionTrack *BoneKeyframeRefObject::parentTrack() const
{
    return m_parentTrackRef;
}

BoneRefObject *BoneKeyframeRefObject::parentBone() const
{
    if (ModelProxy *modelProxy = parentMotion()->parentModel()) {
        return modelProxy->findBoneByName(name());
    }
    return 0;
}

IKeyframe *BoneKeyframeRefObject::baseKeyframeData() const
{
    return data();
}

QObject *BoneKeyframeRefObject::opaque() const
{
    return parentBone();
}

QString BoneKeyframeRefObject::name() const
{
    Q_ASSERT(m_keyframeRef);
    return Util::toQString(m_keyframeRef->name());
}

void BoneKeyframeRefObject::setName(const QString &value)
{
    Q_ASSERT(m_keyframeRef);
    if (!Util::equalsString(value, m_keyframeRef->name())) {
        qt::String s(value);
        m_keyframeRef->setName(&s);
    }
}

QVector3D BoneKeyframeRefObject::localTranslation() const
{
    Q_ASSERT(m_keyframeRef);
    return Util::fromVector3(m_keyframeRef->localTranslation());
}

void BoneKeyframeRefObject::setLocalTranslation(const QVector3D &value)
{
    Q_ASSERT(m_keyframeRef);
    if (!qFuzzyCompare(value, localTranslation())) {
        m_keyframeRef->setLocalTranslation(Util::toVector3(value));
        emit localTranslationChanged();
    }
}

QQuaternion BoneKeyframeRefObject::localOrientation() const
{
    Q_ASSERT(m_keyframeRef);
    return Util::fromQuaternion(m_keyframeRef->localOrientation());
}

void BoneKeyframeRefObject::setLocalOrientation(const QQuaternion &value)
{
    Q_ASSERT(m_keyframeRef);
    if (!qFuzzyCompare(value, localOrientation())) {
        m_keyframeRef->setLocalOrientation(Util::toQuaternion(value));
        emit localRotationChanged();
    }
}

IBoneKeyframe *BoneKeyframeRefObject::data() const
{
    return m_keyframeRef;
}
