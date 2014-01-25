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

#include "JointRefObject.h"
#include "ModelProxy.h"
#include "RigidBodyRefObject.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

JointRefObject::JointRefObject(ModelProxy *parentModel,
                               RigidBodyRefObject *bodyA,
                               RigidBodyRefObject *bodyB,
                               IJoint *jointRef,
                               const QUuid &uuid)
    : m_parentModelRef(parentModel),
      m_bodyARef(bodyA),
      m_bodyBRef(bodyB),
      m_jointRef(jointRef),
      m_uuid(uuid)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_jointRef);
    Q_ASSERT(!m_uuid.isNull());
}

JointRefObject::~JointRefObject()
{
    m_bodyARef = 0;
    m_bodyBRef = 0;
    m_jointRef = 0;
}

IJoint *JointRefObject::data() const
{
    Q_ASSERT(m_jointRef);
    return m_jointRef;
}

ModelProxy *JointRefObject::parentModel() const
{
    Q_ASSERT(m_parentModelRef);
    return m_parentModelRef;
}

RigidBodyRefObject *JointRefObject::bodyA() const
{
    return m_bodyARef;
}

RigidBodyRefObject *JointRefObject::bodyB() const
{
    return m_bodyBRef;
}

QUuid JointRefObject::uuid() const
{
    return m_uuid;
}

int JointRefObject::index() const
{
    Q_ASSERT(m_jointRef);
    return m_jointRef->index();
}

QString JointRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_jointRef);
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
    return Util::toQString(m_jointRef->name(language));
}

void JointRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_jointRef);
    if (name() != value) {
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_jointRef->setName(s.data(), language);
        m_parentModelRef->markDirty();
        emit nameChanged();
    }
}

JointRefObject::Type JointRefObject::type() const
{
    Q_ASSERT(m_jointRef);
    return static_cast<Type>(m_jointRef->type());
}

void JointRefObject::setType(const Type &value)
{
    if (type() != value) {
        m_jointRef->setType(static_cast<IJoint::Type>(value));
        emit typeChanged();
    }
}

QVector3D JointRefObject::position() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->position());
}

void JointRefObject::setPosition(const QVector3D &value)
{
    if (!qFuzzyCompare(position(), value)) {
        m_jointRef->setPosition(Util::toVector3(value));
        emit positionChanged();
    }
}

QVector3D JointRefObject::rotation() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->rotation());
}

void JointRefObject::setRotation(const QVector3D &value)
{
    if (!qFuzzyCompare(position(), value)) {
        m_jointRef->setRotation(Util::toVector3(value));
        emit rotationChanged();
    }
}

QVector3D JointRefObject::positionUpperLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->positionUpperLimit());
}

void JointRefObject::setPositionUpperLimit(const QVector3D &value)
{
    if (!qFuzzyCompare(positionUpperLimit(), value)) {
        m_jointRef->setPositionUpperLimit(Util::toVector3(value));
        emit positionUpperLimitChanged();
    }
}

QVector3D JointRefObject::rotationUpperLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->rotationUpperLimit());
}

void JointRefObject::setRotationUpperLimit(const QVector3D &value)
{
    if (!qFuzzyCompare(rotationUpperLimit(), value)) {
        m_jointRef->setRotationUpperLimit(Util::toVector3(value));
        emit rotationUpperLimitChanged();
    }
}

QVector3D JointRefObject::positionLowerLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->positionLowerLimit());
}

void JointRefObject::setPositionLowerLimit(const QVector3D &value)
{
    if (!qFuzzyCompare(positionLowerLimit(), value)) {
        m_jointRef->setPositionLowerLimit(Util::toVector3(value));
        emit positionLowerLimitChanged();
    }
}

QVector3D JointRefObject::rotationLowerLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->rotationLowerLimit());
}

void JointRefObject::setRotationLowerLimit(const QVector3D &value)
{
    if (!qFuzzyCompare(rotationLowerLimit(), value)) {
        m_jointRef->setRotationLowerLimit(Util::toVector3(value));
        emit rotationLowerLimitChanged();
    }
}

QVector3D JointRefObject::positionStiffness() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->positionStiffness());
}

void JointRefObject::setPositionStiffness(const QVector3D &value)
{
    if (!qFuzzyCompare(positionStiffness(), value)) {
        m_jointRef->setPositionStiffness(Util::toVector3(value));
        emit positionStiffnessChanged();
    }
}

QVector3D JointRefObject::rotationStiffness() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->rotationStiffness());
}

void JointRefObject::setRotationStiffness(const QVector3D &value)
{
    if (!qFuzzyCompare(rotationStiffness(), value)) {
        m_jointRef->setRotationStiffness(Util::toVector3(value));
        emit rotationStiffnessChanged();
    }
}
