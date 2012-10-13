/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/common/RigidBody.h"

#define QT_NO_DEBUG_STREAM
#include <QtDebug>

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btCollisionShape);
BT_DECLARE_HANDLE(btMotionState);
BT_DECLARE_HANDLE(btRigidBody);
#endif

namespace
{

#ifndef VPVL2_NO_BULLET
using namespace vpvl2;

class DefaultMotionState : public btMotionState
{
public:
    DefaultMotionState(const Transform &startTransform, const IBone *bone)
        : m_boneRef(bone),
          m_startTransform(startTransform),
          m_worldTransform(startTransform)
    {
    }
    ~DefaultMotionState() {}

    void getWorldTransform(btTransform &worldTransform) const {
        qDebug("DefaultMotionState#getWorldTransform: %s", m_boneRef->name() ? m_boneRef->name()->toByteArray() : 0);
        qDebug() << m_worldTransform.getOrigin().x()
                 << m_worldTransform.getOrigin().y()
                 << m_worldTransform.getOrigin().z();
        worldTransform = m_worldTransform;
    }
    void setWorldTransform(const btTransform &worldTransform) {
        qDebug("DefaultMotionState#setWorldTransform: %s", m_boneRef->name() ? m_boneRef->name()->toByteArray() : 0);
        qDebug() << worldTransform.getOrigin().x()
                 << worldTransform.getOrigin().y()
                 << worldTransform.getOrigin().z();
        const Vector3 &v = m_boneRef->worldTransform().getOrigin();
        qDebug() << v.x()
                 << v.y()
                 << v.z();
        m_worldTransform = worldTransform;
    }
    void resetWorldTransform(const Transform &value) {
        qDebug("resetWorldTransform: %s", m_boneRef->name() ? m_boneRef->name()->toByteArray() : 0);
        qDebug() << value.getOrigin().x()
                 << value.getOrigin().y()
                 << value.getOrigin().z();
        m_startTransform = m_worldTransform = value;
    }

protected:
    const IBone *m_boneRef;
    Transform m_startTransform;
    Transform m_worldTransform;
};

class AlignedMotionState : public DefaultMotionState
{
public:
    AlignedMotionState(const Transform &startTransform, const IBone *bone)
        : DefaultMotionState(startTransform, bone)
    {
    }
    ~AlignedMotionState() {}

    void getWorldTransform(btTransform &worldTransform) const {
        qDebug("AlignedMotionState#getWorldTransform: %s", m_boneRef->name() ? m_boneRef->name()->toByteArray() : 0);
        qDebug() << m_worldTransform.getOrigin().x()
                 << m_worldTransform.getOrigin().y()
                 << m_worldTransform.getOrigin().z();
        worldTransform = m_worldTransform;
    }
    void setWorldTransform(const btTransform &worldTransform) {
        qDebug("AlignedMotionState#setWorldTransform: %s", m_boneRef->name() ? m_boneRef->name()->toByteArray() : 0);
        qDebug() << worldTransform.getOrigin().x()
                 << worldTransform.getOrigin().y()
                 << worldTransform.getOrigin().z();
        m_worldTransform = worldTransform;
        m_worldTransform.setOrigin(m_boneRef->worldTransform().getOrigin());
    }
};

class KinematicMotionState : public DefaultMotionState
{
public:
    KinematicMotionState(const Transform &startTransform, const IBone *bone)
        : DefaultMotionState(startTransform, bone)
    {
    }
    ~KinematicMotionState() {}

    void getWorldTransform(btTransform &worldTransform) const {
        // Bone#localTransform cannot use at setKinematics because it's called after performTransformBone
        // (Bone#localTransform will be identity)
        Transform output;
        m_boneRef->getLocalTransform(output);
        worldTransform = output * m_startTransform;
    }
    void setWorldTransform(const btTransform & /* worldTransform */) {
    }
};
#endif /* VPVL2_NO_BULLET */

}

namespace vpvl2
{
namespace common
{

RigidBody::RigidBody()
    : m_body(0),
      m_ptr(0),
      m_shape(0),
      m_motionState(0),
      m_kinematicMotionState(0),
      m_worldTransform(Transform::getIdentity()),
      m_world2LocalTransform(Transform::getIdentity()),
      m_boneRef(0),
      m_name(0),
      m_englishName(0),
      m_boneIndex(-1),
      m_size(kZeroV3),
      m_position(kZeroV3),
      m_rotation(kZeroV3),
      m_mass(0),
      m_linearDamping(0),
      m_angularDamping(0),
      m_restitution(0),
      m_friction(0),
      m_index(-1),
      m_groupID(0),
      m_collisionGroupMask(0),
      m_collisionGroupID(0),
      m_shapeType(kUnknownShape),
      m_type(kStaticObject)
{
}

RigidBody::~RigidBody()
{
    delete m_body;
    m_body = 0;
    delete m_ptr;
    m_ptr = 0;
    delete m_shape;
    m_shape = 0;
    delete m_motionState;
    m_motionState = 0;
    delete m_kinematicMotionState;
    m_kinematicMotionState = 0;
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_boneRef = 0;
    m_boneIndex = -1;
    m_worldTransform.setIdentity();
    m_world2LocalTransform.setIdentity();
    m_size.setZero();
    m_position.setZero();
    m_rotation.setZero();
    m_mass = 0;
    m_linearDamping = 0;
    m_angularDamping = 0;
    m_restitution = 0;
    m_friction = 0;
    m_index = -1;
    m_groupID = 0;
    m_collisionGroupMask = 0;
    m_collisionGroupID = 0;
    m_shapeType = kUnknownShape;
    m_type = kStaticObject;
}

void RigidBody::performTransformBone()
{
#ifndef VPVL2_NO_BULLET
    if (m_type == kStaticObject || !m_boneRef || m_boneRef == NullBone::sharedReference())
        return;
    const Transform &worldTransform = m_body->getCenterOfMassTransform();
    const Transform &localTransform = worldTransform * m_world2LocalTransform;
    qDebug("%s", m_name->toByteArray());
    qDebug() << localTransform.getOrigin().x()
             << localTransform.getOrigin().y()
             << localTransform.getOrigin().z();
    qDebug() << m_world2LocalTransform.getOrigin().x()
             << m_world2LocalTransform.getOrigin().y()
             << m_world2LocalTransform.getOrigin().z();
    qDebug() << worldTransform.getOrigin().x()
             << worldTransform.getOrigin().y()
             << worldTransform.getOrigin().z();
    m_boneRef->setLocalTransform(localTransform);
#endif /* VPVL2_NO_BULLET */
}

void RigidBody::setKinematic(bool value)
{
#ifndef VPVL2_NO_BULLET
    if (m_type == kStaticObject)
        return;
    if (value) {
        m_body->setMotionState(m_kinematicMotionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    else {
        Transform transform;
        m_kinematicMotionState->getWorldTransform(transform);
        DefaultMotionState *motionState = static_cast<DefaultMotionState *>(m_motionState);
        motionState->resetWorldTransform(transform);
        m_body->setMotionState(motionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }
#else  /* VPVL2_NO_BULLET */
    (void) value;
#endif /* VPVL2_NO_BULLET */
}

const Transform RigidBody::createTransform() const
{
    Matrix3x3 basis;
#ifdef VPVL2_COORDINATE_OPENGL
    Matrix3x3 mx, my, mz;
    mx.setEulerZYX(-m_rotation[0], 0.0f, 0.0f);
    my.setEulerZYX(0.0f, -m_rotation[1], 0.0f);
    mz.setEulerZYX(0.0f, 0.0f, m_rotation[2]);
    basis = my * mz * mx;
#else  /* VPVL2_COORDINATE_OPENGL */
    basis.setEulerZYX(m_rotation[0], m_rotation[1], m_rotation[2]);
#endif /* VPVL2_COORDINATE_OPENGL */
    return Transform(basis, m_position);
}

const Transform RigidBody::createStartTransform(const Transform &transform) const
{
    return transform;
}

btCollisionShape *RigidBody::createShape() const
{
    switch (m_shapeType) {
    case kSphereShape:
        return new btSphereShape(m_size.x());
    case kBoxShape:
        return new btBoxShape(m_size);
    case kCapsureShape:
        return new btCapsuleShape(m_size.x(), m_size.y());
    case kUnknownShape:
    default:
        return 0;
    }
}

btRigidBody *RigidBody::createRigidBody(btCollisionShape *shape)
{
    Vector3 localInertia(kZeroV3);
    Scalar massValue(0);
    if (m_type != kStaticObject) {
        massValue = m_mass;
        if (shape && !btFuzzyZero(massValue))
            shape->calculateLocalInertia(massValue, localInertia);
    }
    m_worldTransform = createTransform();
    m_world2LocalTransform = m_worldTransform.inverse();
    Transform startTransform(createStartTransform(m_worldTransform));
    switch (m_type) {
    default:
    case kStaticObject:
        m_motionState = new KinematicMotionState(m_worldTransform, m_boneRef);
        m_kinematicMotionState = 0;
        break;
    case kDynamicObject:
        m_motionState = new DefaultMotionState(startTransform, m_boneRef);
        m_kinematicMotionState = new KinematicMotionState(m_worldTransform, m_boneRef);
        break;
    case kAlignedObject:
        m_motionState = new AlignedMotionState(startTransform, m_boneRef);
        m_kinematicMotionState = new KinematicMotionState(m_worldTransform, m_boneRef);
        break;
    }
    btRigidBody::btRigidBodyConstructionInfo info(massValue, m_motionState, shape, localInertia);
    info.m_linearDamping = m_linearDamping;
    info.m_angularDamping = m_angularDamping;
    info.m_restitution = m_restitution;
    info.m_friction = m_friction;
    info.m_additionalDamping = true;
    /*
    if (m_type == kDynamicObject) {
        qWarning("%s", m_name->toByteArray());
    }
    */
    btRigidBody *body = m_ptr = new btRigidBody(info);
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setUserPointer(this);
    if (m_type == kStaticObject)
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    return body;
}

void RigidBody::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void RigidBody::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void RigidBody::setBone(IBone *value)
{
    m_boneRef = value;
    m_boneIndex = value ? value->index() : -1;
}

void RigidBody::setAngularDamping(float value)
{
    m_angularDamping = value;
}

void RigidBody::setCollisionGroupID(uint16_t value)
{
    m_collisionGroupID = value;
}

void RigidBody::setCollisionMask(uint16_t value)
{
    m_collisionGroupID = value;
}

void RigidBody::setFriction(float value)
{
    m_friction = value;
}

void RigidBody::setLinearDamping(float value)
{
    m_linearDamping = value;
}

void RigidBody::setMass(float value)
{
    m_mass = value;
}

void RigidBody::setPosition(const Vector3 &value)
{
    m_position = value;
}

void RigidBody::setRestitution(float value)
{
    m_restitution = value;
}

void RigidBody::setRotation(const Vector3 &value)
{
    m_rotation = value;
}

void RigidBody::setShapeType(ShapeType value)
{
    m_shapeType = value;
}

void RigidBody::setSize(const Vector3 &value)
{
    m_size = value;
}

void RigidBody::setType(ObjectType value)
{
    m_type = value;
}

void RigidBody::setIndex(int value)
{
    m_index = value;
}

void RigidBody::build(IBone *bone, int index)
{
    m_boneRef = bone;
    m_shape = createShape();
    m_body = createRigidBody(m_shape);
    m_ptr = 0;
    m_index = index;
    if (m_type != kStaticObject)
        bone->setInverseKinematicsEnable(false);
}

} /* namespace pmx */
} /* namespace vpvl2 */
