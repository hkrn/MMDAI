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

#include "RigidBodyRefObject.h"
#include "ModelProxy.h"
#include "Util.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

RigidBodyRefObject::RigidBodyRefObject(ModelProxy *parentModelRef,
                                       BoneRefObject *parentBoneRef,
                                       vpvl2::IRigidBody *rigidBodyRef,
                                       const QUuid &uuid)
    : m_parentModelRef(parentModelRef),
      m_parentBoneRef(parentBoneRef),
      m_rigidBodyRef(rigidBodyRef),
      m_uuid(uuid)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_rigidBodyRef);
    Q_ASSERT(!m_uuid.isNull());
}

RigidBodyRefObject::~RigidBodyRefObject()
{
    m_parentModelRef = 0;
    m_rigidBodyRef = 0;
}

IRigidBody *RigidBodyRefObject::data() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef;
}

ModelProxy *RigidBodyRefObject::parentModel() const
{
    return m_parentModelRef;
}

BoneRefObject *RigidBodyRefObject::parentBone() const
{
    return m_parentBoneRef;
}

QUuid RigidBodyRefObject::uuid() const
{
    return m_uuid;
}

int RigidBodyRefObject::index() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->index();
}

QString RigidBodyRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_rigidBodyRef);
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
    return Util::toQString(m_rigidBodyRef->name(language));
}

void RigidBodyRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_rigidBodyRef);
    if (name() != value) {
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_rigidBodyRef->setName(s.data(), language);
        m_parentModelRef->markDirty();
        emit nameChanged();
    }
}

QVector3D RigidBodyRefObject::position() const
{
    Q_ASSERT(m_rigidBodyRef);
    return Util::fromVector3(m_rigidBodyRef->position());
}

void RigidBodyRefObject::setPosition(const QVector3D &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (!qFuzzyCompare(position(), value)) {
        m_rigidBodyRef->setPosition(Util::toVector3(value));
        m_parentModelRef->markDirty();
        emit positionChanged();
    }
}

QVector3D RigidBodyRefObject::rotation() const
{
    Q_ASSERT(m_rigidBodyRef);
    return Util::fromVector3(m_rigidBodyRef->rotation());
}

RigidBodyRefObject::ObjectType RigidBodyRefObject::objectType() const
{
    Q_ASSERT(m_rigidBodyRef);
    return static_cast<ObjectType>(m_rigidBodyRef->objectType());
}

void RigidBodyRefObject::setObjectType(const ObjectType &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (objectType() != value) {
        m_rigidBodyRef->setObjectType(static_cast<IRigidBody::ObjectType>(value));
        emit objectTypeChanged();
    }
}

RigidBodyRefObject::ShapeType RigidBodyRefObject::shapeType() const
{
    Q_ASSERT(m_rigidBodyRef);
    return static_cast<ShapeType>(m_rigidBodyRef->shapeType());
}

void RigidBodyRefObject::setShapeType(const ShapeType &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (shapeType() != value) {
        m_rigidBodyRef->setShapeType(static_cast<IRigidBody::ShapeType>(value));
        emit shapeTypeChanged();
    }
}

void RigidBodyRefObject::setRotation(const QVector3D &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (!qFuzzyCompare(rotation(), value)) {
        m_rigidBodyRef->setRotation(Util::toVector3(value));
        m_parentModelRef->markDirty();
        emit rotationChanged();
    }
}

qreal RigidBodyRefObject::mass() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->mass();
}

void RigidBodyRefObject::setMass(const qreal &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (qFuzzyCompare(mass(), value)) {
        m_rigidBodyRef->setMass(value);
        m_parentModelRef->markDirty();
        emit massChanged();
    }
}

qreal RigidBodyRefObject::linearDamping() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->linearDamping();
}

void RigidBodyRefObject::setLinearDamping(const qreal &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (qFuzzyCompare(linearDamping(), value)) {
        m_rigidBodyRef->setLinearDamping(value);
        m_parentModelRef->markDirty();
        emit linearDampingChanged();
    }
}

qreal RigidBodyRefObject::angularDamping() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->angularDamping();
}

void RigidBodyRefObject::setAngularDamping(const qreal &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (qFuzzyCompare(angularDamping(), value)) {
        m_rigidBodyRef->setAngularDamping(value);
        m_parentModelRef->markDirty();
        emit angularDampingChanged();
    }
}

qreal RigidBodyRefObject::friction() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->friction();
}

void RigidBodyRefObject::setFriction(const qreal &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (qFuzzyCompare(friction(), value)) {
        m_rigidBodyRef->setFriction(value);
        m_parentModelRef->markDirty();
        emit frictionChanged();
    }
}

qreal RigidBodyRefObject::restitution() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->restitution();
}

void RigidBodyRefObject::setRestitution(const qreal &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (qFuzzyCompare(restitution(), value)) {
        m_rigidBodyRef->setRestitution(value);
        m_parentModelRef->markDirty();
        emit restitutionChanged();
    }
}

quint16 RigidBodyRefObject::collisionGroupMask() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->collisionGroupMask();
}

void RigidBodyRefObject::setCollisionGroupMask(const quint16 &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (collisionGroupMask() != value) {
        m_rigidBodyRef->setCollisionMask(value);
        m_parentModelRef->markDirty();
        emit collisionGroupMaskChanged();
    }
}

quint8 RigidBodyRefObject::collisionGroupID() const
{
    Q_ASSERT(m_rigidBodyRef);
    return m_rigidBodyRef->collisionGroupID();
}

void RigidBodyRefObject::setCollisionGroupID(const quint8 &value)
{
    Q_ASSERT(m_rigidBodyRef);
    if (collisionGroupID() != value) {
        m_rigidBodyRef->setCollisionGroupID(value);
        m_parentModelRef->markDirty();
        emit collisionGroupMaskChanged();
    }
}
