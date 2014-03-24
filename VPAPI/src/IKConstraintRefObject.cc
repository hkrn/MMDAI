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

#include "IKConstraintRefObject.h"

#include "BoneRefObject.h"
#include "ModelProxy.h"
#include "Util.h"

using namespace vpvl2;

ChildIKJoint::ChildIKJoint(IKConstraintRefObject *parentConstraintRef, IBone::IKJoint *jointRef)
    : QObject(parentConstraintRef),
      m_parentConstraintRef(parentConstraintRef),
      m_jointRef(jointRef)
{
    Q_ASSERT(m_parentConstraintRef);
    Q_ASSERT(m_jointRef);
    connect(targetBone(), &BoneRefObject::nameChanged, this, &ChildIKJoint::nameChanged);
}

ChildIKJoint::~ChildIKJoint()
{
    m_parentConstraintRef = 0;
    m_jointRef = 0;
}

IKConstraintRefObject *ChildIKJoint::parentConstraint() const
{
    return m_parentConstraintRef;
}

BoneRefObject *ChildIKJoint::targetBone() const
{
    Q_ASSERT(m_parentConstraintRef);
    Q_ASSERT(m_jointRef);
    return m_parentConstraintRef->parentModel()->resolveBoneRef(m_jointRef->targetBoneRef());
}

QString ChildIKJoint::name() const
{
    return targetBone()->name();
}

QVector3D ChildIKJoint::upperLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->upperLimit());
}

void ChildIKJoint::setUpperLimit(const QVector3D &value)
{
    Q_ASSERT(m_jointRef);
    Q_ASSERT(m_parentConstraintRef);
    if (!qFuzzyCompare(upperLimit(), value)) {
        m_jointRef->setUpperLimit(Util::toVector3(value));
        m_parentConstraintRef->setDirty(true);
        emit upperLimitChanged();
    }
}

QVector3D ChildIKJoint::lowerLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3(m_jointRef->lowerLimit());
}

void ChildIKJoint::setLowerLimit(const QVector3D &value)
{
    Q_ASSERT(m_jointRef);
    Q_ASSERT(m_parentConstraintRef);
    if (!qFuzzyCompare(lowerLimit(), value)) {
        m_jointRef->setLowerLimit(Util::toVector3(value));
        m_parentConstraintRef->setDirty(true);
        emit lowerLimitChanged();
    }
}

QVector3D ChildIKJoint::degreeUpperLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3Radian(m_jointRef->upperLimit());
}

void ChildIKJoint::setDegreeUpperLimit(const QVector3D &value)
{
    Q_ASSERT(m_jointRef);
    Q_ASSERT(m_parentConstraintRef);
    if (!qFuzzyCompare(upperLimit(), value)) {
        m_jointRef->setUpperLimit(Util::toVector3Radian(value));
        m_parentConstraintRef->setDirty(true);
        emit upperLimitChanged();
    }
}

QVector3D ChildIKJoint::degreeLowerLimit() const
{
    Q_ASSERT(m_jointRef);
    return Util::fromVector3Radian(m_jointRef->lowerLimit());
}

void ChildIKJoint::setDegreeLowerLimit(const QVector3D &value)
{
    Q_ASSERT(m_jointRef);
    Q_ASSERT(m_parentConstraintRef);
    if (!qFuzzyCompare(lowerLimit(), value)) {
        m_jointRef->setLowerLimit(Util::toVector3Radian(value));
        m_parentConstraintRef->setDirty(true);
        emit lowerLimitChanged();
    }
}

bool ChildIKJoint::hasAngleLimit() const
{
    Q_ASSERT(m_jointRef);
    return m_jointRef->hasAngleLimit();
}

void ChildIKJoint::setHasAngleLimit(bool value)
{
    Q_ASSERT(m_jointRef);
    Q_ASSERT(m_parentConstraintRef);
    if (hasAngleLimit() != value) {
        m_jointRef->setHasAngleLimit(value);
        m_parentConstraintRef->setDirty(true);
        emit hasAngleLimit();
    }
}

IKConstraintRefObject::IKConstraintRefObject(ModelProxy *parentModel,
                         IBone::IKConstraint *constraintRef,
                         const QUuid &uuid,
                         int index)
    : QObject(parentModel),
      m_parentModelRef(parentModel),
      m_constraintRef(constraintRef),
      m_uuid(uuid),
      m_index(index),
      m_dirty(false)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_constraintRef);
    Q_ASSERT(!m_uuid.isNull());
}

IKConstraintRefObject::~IKConstraintRefObject()
{
    qDeleteAll(m_childJoints);
    m_childJoints.clear();
    m_parentModelRef = 0;
    m_constraintRef = 0;
    m_dirty = false;
}

void IKConstraintRefObject::initialize()
{
    Array<IBone::IKJoint *> joints;
    m_constraintRef->getJointRefs(joints);
    const int njoints = joints.count();
    for (int i = 0; i < njoints; i++) {
        IBone::IKJoint *joint = joints[i];
        m_childJoints.append(new ChildIKJoint(this, joint));
    }
}

bool IKConstraintRefObject::isDirty() const
{
    return m_dirty;
}

ModelProxy *IKConstraintRefObject::parentModel() const
{
    return m_parentModelRef;
}

QUuid IKConstraintRefObject::uuid() const
{
    return m_uuid;
}

QString IKConstraintRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_constraintRef);
    BoneRefObject *bone = m_parentModelRef->resolveBoneRef(m_constraintRef->effectorBoneRef());
    return bone ? bone->name() : QString();
}

int IKConstraintRefObject::index() const
{
    return m_index;
}

QQmlListProperty<ChildIKJoint> IKConstraintRefObject::allChildJoints()
{
    return QQmlListProperty<ChildIKJoint>(this, m_childJoints);
}

BoneRefObject *IKConstraintRefObject::rootBone() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_constraintRef);
    return m_parentModelRef->resolveBoneRef(m_constraintRef->rootBoneRef());
}

void IKConstraintRefObject::setRootBone(BoneRefObject *value)
{
    Q_ASSERT(m_constraintRef);
    if (rootBone() != value) {
        m_constraintRef->setRootBoneRef(value->data());
        emit rootBoneChanged();
    }
}

BoneRefObject *IKConstraintRefObject::effectorBone() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_constraintRef);
    return m_parentModelRef->resolveBoneRef(m_constraintRef->effectorBoneRef());
}

void IKConstraintRefObject::setEffectorBone(BoneRefObject *value)
{
    Q_ASSERT(m_constraintRef);
    if (effectorBone() != value) {
        m_constraintRef->setEffectorBoneRef(value->data());
        emit effectorBoneChanged();
    }
}

qreal IKConstraintRefObject::angleLimit() const
{
    Q_ASSERT(m_constraintRef);
    return m_constraintRef->angleLimit();
}

void IKConstraintRefObject::setAngleLimit(qreal value)
{
    Q_ASSERT(m_constraintRef);
    if (!qFuzzyCompare(angleLimit(), value)) {
        m_constraintRef->setAngleLimit(value);
        emit angleLimitChanged();
    }
}

int IKConstraintRefObject::numIterations() const
{
    Q_ASSERT(m_constraintRef);
    return m_constraintRef->numIterations();
}

void IKConstraintRefObject::setNumIterations(int value)
{
    Q_ASSERT(m_constraintRef);
    if (numIterations() != value) {
        m_constraintRef->setNumIterations(value);
        emit numIterationsChanged();
    }
}

void IKConstraintRefObject::setDirty(bool value)
{
    if (isDirty() != value) {
        m_dirty = value;
        if (value) {
            Q_ASSERT(m_parentModelRef);
            m_parentModelRef->setDirty(value);
        }
        emit dirtyChanged();
    }
}
