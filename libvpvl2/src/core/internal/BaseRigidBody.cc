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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/BaseRigidBody.h"
#include "vpvl2/internal/util.h"

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

namespace vpvl2
{
namespace internal
{

BaseRigidBody::DefaultMotionState::DefaultMotionState(const Transform &startTransform, const IBone *bone)
    : m_boneRef(bone),
      m_startTransform(startTransform),
      m_worldTransform(startTransform)
{
}

BaseRigidBody::DefaultMotionState::~DefaultMotionState()
{
}

void BaseRigidBody::DefaultMotionState::getWorldTransform(btTransform &worldTransform) const
{
    worldTransform = m_worldTransform;
}

void BaseRigidBody::DefaultMotionState::setWorldTransform(const btTransform &worldTransform)
{
    m_worldTransform = worldTransform;
}

void BaseRigidBody::DefaultMotionState::reset()
{
    m_worldTransform = m_startTransform;
}

const IBone *BaseRigidBody::DefaultMotionState::boneRef() const
{
    return m_boneRef;
}

void BaseRigidBody::DefaultMotionState::setBoneRef(const IBone *value)
{
    m_boneRef = value;
}

BaseRigidBody::AlignedMotionState::AlignedMotionState(const Transform &startTransform, const IBone *bone)
    : DefaultMotionState(startTransform, bone)
{
}

BaseRigidBody::AlignedMotionState::~AlignedMotionState()
{
}

void BaseRigidBody::AlignedMotionState::setWorldTransform(const btTransform &worldTransform)
{
    m_worldTransform = worldTransform;
    m_worldTransform.setOrigin(m_boneRef->worldTransform().getOrigin());
}

BaseRigidBody::KinematicMotionState::KinematicMotionState(const Transform &startTransform, const IBone *bone)
    : DefaultMotionState(startTransform, bone)
{
}

BaseRigidBody::KinematicMotionState::~KinematicMotionState()
{
}

void BaseRigidBody::KinematicMotionState::getWorldTransform(btTransform &worldTransform) const
{
    // Bone#localTransform cannot use at setKinematics because it's called after performTransformBone
    // (Bone#localTransform will be identity)
    Transform localTransform;
    m_boneRef->getLocalTransform(localTransform);
    worldTransform = localTransform * m_startTransform;
}

void BaseRigidBody::KinematicMotionState::setWorldTransform(const btTransform & /* worldTransform */)
{
}

BaseRigidBody::BaseRigidBody(IModel *parentModelRef, IEncoding *encodingRef)
    : m_body(0),
      m_ptr(0),
      m_shape(0),
      m_motionState(0),
      m_kinematicMotionState(0),
      m_worldTransform(Transform::getIdentity()),
      m_world2LocalTransform(Transform::getIdentity()),
      m_parentModelRef(parentModelRef),
      m_encodingRef(encodingRef),
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

BaseRigidBody::~BaseRigidBody()
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
    m_parentModelRef = 0;
    m_encodingRef = 0;
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

void BaseRigidBody::syncLocalTransform()
{
    if (m_type != kStaticObject && m_boneRef && m_boneRef != Factory::sharedNullBoneRef()) {
        const Transform &worldTransform = m_body->getCenterOfMassTransform();
        const Transform &localTransform = worldTransform * m_world2LocalTransform;
        m_boneRef->setLocalTransform(localTransform);
    }
}

void BaseRigidBody::joinWorld(void *value)
{
    btDiscreteDynamicsWorld *worldRef = static_cast<btDiscreteDynamicsWorld *>(value);
    worldRef->addRigidBody(m_body, m_groupID, m_collisionGroupMask);
    m_body->setUserPointer(this);
}

void BaseRigidBody::leaveWorld(void *value)
{
    btDiscreteDynamicsWorld *worldRef = static_cast<btDiscreteDynamicsWorld *>(value);
    worldRef->removeRigidBody(m_body);
    m_body->setUserPointer(0);
}

void BaseRigidBody::setKinematic(bool value, const Vector3 &basePosition)
{
    m_motionState->reset();
    if (m_type != kStaticObject) {
        if (value) {
            m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            m_body->setMotionState(m_kinematicMotionState);
        }
        else {
            m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
            m_body->setMotionState(m_motionState);
        }
        Transform transform = m_body->getCenterOfMassTransform();
        transform.getOrigin() += basePosition;
        if (m_boneIndex == -1) {
            transform.setOrigin(kZeroV3);
        }
        m_body->setCenterOfMassTransform(transform);
        m_body->setInterpolationWorldTransform(transform);
    }
    else {
        const Transform &transform = m_body->getCenterOfMassTransform();
        m_body->setMotionState(m_motionState);
        m_body->setInterpolationWorldTransform(transform);
    }
}

const Transform BaseRigidBody::createTransform() const
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

btCollisionShape *BaseRigidBody::createShape() const
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

btRigidBody *BaseRigidBody::createRigidBody(btCollisionShape *shape)
{
    Vector3 localInertia(kZeroV3);
    Scalar massValue(0);
    if (m_type != kStaticObject) {
        massValue = m_mass;
        if (shape && !btFuzzyZero(massValue)) {
            shape->calculateLocalInertia(massValue, localInertia);
        }
    }
    m_worldTransform = createTransform();
    m_world2LocalTransform = m_worldTransform.inverse();
    switch (m_type) {
    default:
    case kStaticObject:
        m_motionState = createKinematicMotionState();
        m_kinematicMotionState = 0;
        break;
    case kDynamicObject:
        m_motionState = createDefaultMotionState();
        m_kinematicMotionState = createKinematicMotionState();
        break;
    case kAlignedObject:
        m_motionState = createAlignedMotionState();
        m_kinematicMotionState = createKinematicMotionState();
        break;
    }
    btRigidBody::btRigidBodyConstructionInfo info(massValue, m_motionState, shape, localInertia);
    info.m_linearDamping = m_linearDamping;
    info.m_angularDamping = m_angularDamping;
    info.m_restitution = m_restitution;
    info.m_friction = m_friction;
    info.m_additionalDamping = true;
    btRigidBody *body = m_ptr = new btRigidBody(info);
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setUserPointer(this);
    if (m_type == kStaticObject) {
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    return body;
}

void BaseRigidBody::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void BaseRigidBody::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void BaseRigidBody::setParentModelRef(IModel *value)
{
    m_parentModelRef = value;
}

void BaseRigidBody::setBoneRef(IBone *value)
{
    if (value) {
        m_boneIndex = value->index();
        m_boneRef = value;
        value->setInverseKinematicsEnable(m_type == kStaticObject);
    }
    else {
        m_boneRef = Factory::sharedNullBoneRef();
        m_boneIndex = -1;
    }
    if (m_motionState) {
        m_motionState->setBoneRef(m_boneRef);
    }
}

void BaseRigidBody::setAngularDamping(float32 value)
{
    m_angularDamping = value;
}

void BaseRigidBody::setCollisionGroupID(uint8 value)
{
    m_collisionGroupID = value;
}

void BaseRigidBody::setCollisionMask(uint16 value)
{
    m_collisionGroupMask = value;
}

void BaseRigidBody::setFriction(float32 value)
{
    m_friction = value;
}

void BaseRigidBody::setLinearDamping(float32 value)
{
    m_linearDamping = value;
}

void BaseRigidBody::setMass(float32 value)
{
    m_mass = value;
}

void BaseRigidBody::setPosition(const Vector3 &value)
{
    m_position = value;
}

void BaseRigidBody::setRestitution(float32 value)
{
    m_restitution = value;
}

void BaseRigidBody::setRotation(const Vector3 &value)
{
    m_rotation = value;
}

void BaseRigidBody::setShapeType(ShapeType value)
{
    m_shapeType = value;
}

void BaseRigidBody::setSize(const Vector3 &value)
{
    m_size = value;
}

void BaseRigidBody::setType(ObjectType value)
{
    m_type = value;
}

void BaseRigidBody::setIndex(int value)
{
    m_index = value;
}

void BaseRigidBody::build(IBone *boneRef, int index)
{
    setBoneRef(boneRef);
    m_shape = createShape();
    m_body = createRigidBody(m_shape);
    m_ptr = 0;
    m_index = index;
}

BaseRigidBody::DefaultMotionState *BaseRigidBody::createKinematicMotionState() const
{
    return new BaseRigidBody::KinematicMotionState(m_worldTransform, m_boneRef);
}

BaseRigidBody::DefaultMotionState *BaseRigidBody::createDefaultMotionState() const
{
    return new BaseRigidBody::DefaultMotionState(m_worldTransform, m_boneRef);
}

BaseRigidBody::DefaultMotionState *BaseRigidBody::createAlignedMotionState() const
{
    return new BaseRigidBody::AlignedMotionState(m_worldTransform, m_boneRef);
}

} /* namespace pmx */
} /* namespace vpvl2 */
