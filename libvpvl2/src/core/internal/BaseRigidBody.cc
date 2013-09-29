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

void BaseRigidBody::DefaultMotionState::setWorldTransform(const btTransform & /* worldTransform */)
{
    /* explicit synchronization at RigidBody#syncLocalTransform */
}

void BaseRigidBody::DefaultMotionState::assignWorldTransform(const btTransform &value)
{
    m_worldTransform = value;
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

void BaseRigidBody::AlignedMotionState::getWorldTransform(btTransform &worldTransform) const
{
    worldTransform.setIdentity();
    worldTransform.setOrigin(m_boneRef->worldTransform().getOrigin());
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

BaseRigidBody::BaseRigidBody(IModel *parentModelRef, IEncoding *encodingRef)
    : m_body(0),
      m_ptr(0),
      m_shape(0),
      m_activeMotionState(0),
      m_kinematicMotionState(0),
      m_worldTransform(Transform::getIdentity()),
      m_world2LocalTransform(Transform::getIdentity()),
      m_parentModelRef(parentModelRef),
      m_encodingRef(encodingRef),
      m_boneRef(Factory::sharedNullBoneRef()),
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
    delete m_activeMotionState;
    m_activeMotionState = 0;
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

void BaseRigidBody::setActivation(bool value)
{
    const Transform &newTransform = m_boneRef->localTransform() * m_worldTransform;
    if (m_type != kStaticObject) {
        if (value) {
            m_activeMotionState->assignWorldTransform(newTransform);
            m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
            m_body->setMotionState(m_activeMotionState);
        }
        else {
            m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            m_body->setMotionState(m_kinematicMotionState);
        }
    }
    else {
        m_body->setMotionState(m_activeMotionState);
    }
    m_body->setInterpolationWorldTransform(newTransform);
}

void BaseRigidBody::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_eventRefs.remove(value);
        m_eventRefs.append(value);
    }
}

void BaseRigidBody::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_eventRefs.remove(value);
    }
}

void BaseRigidBody::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_eventRefs);
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
        m_activeMotionState = createKinematicMotionState();
        m_kinematicMotionState = 0;
        break;
    case kDynamicObject:
        m_activeMotionState = createDefaultMotionState();
        m_kinematicMotionState = createKinematicMotionState();
        break;
    case kAlignedObject:
        m_activeMotionState = createDefaultMotionState();
        m_kinematicMotionState = createKinematicMotionState();
        break;
    }
    btRigidBody::btRigidBodyConstructionInfo info(massValue, m_activeMotionState, shape, localInertia);
    info.m_linearDamping = m_linearDamping;
    info.m_angularDamping = m_angularDamping;
    info.m_restitution = m_restitution;
    info.m_friction = m_friction;
    info.m_additionalDamping = true;
    btRigidBody *body = m_ptr = new btRigidBody(info);
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setUserPointer(this);
    switch (m_type) {
    case kStaticObject:
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        break;
    default:
        break;
    }
    return body;
}

btRigidBody *BaseRigidBody::body() const VPVL2_DECL_NOEXCEPT
{
    return m_body;
}

void *BaseRigidBody::bodyPtr() const VPVL2_DECL_NOEXCEPT
{
    return m_body;
}

IModel *BaseRigidBody::parentModelRef() const VPVL2_DECL_NOEXCEPT
{
    return m_parentModelRef;
}

IBone *BaseRigidBody::boneRef() const VPVL2_DECL_NOEXCEPT
{
    return m_boneRef;
}

int BaseRigidBody::boneIndex() const VPVL2_DECL_NOEXCEPT
{
    return m_boneIndex;
}

const IString *BaseRigidBody::name(IEncoding::LanguageType type) const VPVL2_DECL_NOEXCEPT
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_name;
    case IEncoding::kEnglish:
        return m_englishName;
    default:
        return 0;
    }
}

Vector3 BaseRigidBody::size() const VPVL2_DECL_NOEXCEPT
{
    return m_size;
}

Vector3 BaseRigidBody::position() const VPVL2_DECL_NOEXCEPT
{
    return m_position;
}

Vector3 BaseRigidBody::rotation() const VPVL2_DECL_NOEXCEPT
{
    return m_rotation;
}

float32 BaseRigidBody::mass() const VPVL2_DECL_NOEXCEPT
{
    return m_mass;
}

float32 BaseRigidBody::linearDamping() const VPVL2_DECL_NOEXCEPT
{
    return m_linearDamping;
}

float32 BaseRigidBody::angularDamping() const VPVL2_DECL_NOEXCEPT
{
    return m_angularDamping;
}

float32 BaseRigidBody::restitution() const VPVL2_DECL_NOEXCEPT
{
    return m_restitution;
}

float32 BaseRigidBody::friction() const VPVL2_DECL_NOEXCEPT
{
    return m_friction;
}

uint16 BaseRigidBody::groupID() const VPVL2_DECL_NOEXCEPT
{
    return m_groupID;
}

uint16 BaseRigidBody::collisionGroupMask() const VPVL2_DECL_NOEXCEPT
{
    return m_collisionGroupMask;
}

uint8 BaseRigidBody::collisionGroupID() const VPVL2_DECL_NOEXCEPT
{
    return m_collisionGroupID;
}

int BaseRigidBody::index() const VPVL2_DECL_NOEXCEPT
{
    return m_index;
}

void BaseRigidBody::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_name)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_name);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_englishName)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_englishName);
        }
        break;
    default:
        break;
    }
}

void BaseRigidBody::setParentModelRef(IModel *value)
{
    m_parentModelRef = value;
}

void BaseRigidBody::setBoneRef(IBone *value)
{
    if (value != m_boneRef) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, boneRefWillChange(value, this));
        if (value) {
            m_boneIndex = value->index();
            m_boneRef = value;
            value->setInverseKinematicsEnable(m_type == kStaticObject);
        }
        else {
            m_boneRef = Factory::sharedNullBoneRef();
            m_boneIndex = -1;
        }
        if (m_activeMotionState) {
            m_activeMotionState->setBoneRef(m_boneRef);
        }
    }
}

void BaseRigidBody::setAngularDamping(float32 value)
{
    if (m_angularDamping != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, angularDampingWillChange(value, this));
        m_angularDamping = value;
    }
}

void BaseRigidBody::setCollisionGroupID(uint8 value)
{
    if (m_collisionGroupID != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, collisionGroupIDWillChange(value, this));
        m_collisionGroupID = value;
    }
}

void BaseRigidBody::setCollisionMask(uint16 value)
{
    if (m_collisionGroupMask != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, collisionMaskWillChange(value, this));
        m_collisionGroupMask = value;
    }
}

void BaseRigidBody::setFriction(float32 value)
{
    if (m_friction != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, frictionWillChange(value, this));
        m_friction = value;
    }
}

void BaseRigidBody::setLinearDamping(float32 value)
{
    if (m_linearDamping != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, linearDampingWillChange(value, this));
        m_linearDamping = value;
    }
}

void BaseRigidBody::setMass(float32 value)
{
    if (m_mass != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, massWillChange(value, this));
        m_mass = value;
    }
}

void BaseRigidBody::setPosition(const Vector3 &value)
{
    if (m_position != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, positionWillChange(value, this));
        m_position = value;
    }
}

void BaseRigidBody::setRestitution(float32 value)
{
    if (m_restitution != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, restitutionWillChange(value, this));
        m_restitution = value;
    }
}

void BaseRigidBody::setRotation(const Vector3 &value)
{
    if (m_rotation != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rotationWillChange(value, this));
        m_rotation = value;
    }
}

void BaseRigidBody::setShapeType(ShapeType value)
{
    if (m_shapeType != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, shapeTypeWillChange(value, this));
        m_shapeType = value;
    }
}

void BaseRigidBody::setSize(const Vector3 &value)
{
    if (m_size != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, sizeWillChange(value, this));
        m_size = value;
    }
}

void BaseRigidBody::setType(ObjectType value)
{
    if (m_type != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, typeWillChange(value, this));
        m_type = value;
    }
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

BaseRigidBody::AlignedMotionState *BaseRigidBody::createAlignedMotionState() const
{
    return new BaseRigidBody::AlignedMotionState(m_worldTransform, m_boneRef);
}

BaseRigidBody::DefaultMotionState *BaseRigidBody::createDefaultMotionState() const
{
    return new BaseRigidBody::DefaultMotionState(m_worldTransform, m_boneRef);
}

} /* namespace pmx */
} /* namespace vpvl2 */
