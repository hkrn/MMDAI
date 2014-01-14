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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/BaseJoint.h"
#include "vpvl2/internal/util.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#endif
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>
#include <BulletDynamics/ConstraintSolver/btPoint2PointConstraint.h>
#include <BulletDynamics/ConstraintSolver/btSliderConstraint.h>
#include <BulletDynamics/ConstraintSolver/btSolverConstraint.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace vpvl2
{
namespace internal
{

const Scalar BaseJoint::kDefaultDamping = 0.25;

BaseJoint::BaseJoint(IModel *modelRef)
    : m_constraint(0),
      m_ptr(0),
      m_parentModelRef(modelRef),
      m_rigidBody1Ref(0),
      m_rigidBody2Ref(0),
      m_name(0),
      m_englishName(0),
      m_position(kZeroV3),
      m_rotation(kZeroV3),
      m_positionLowerLimit(kZeroV3),
      m_rotationLowerLimit(kZeroV3),
      m_positionUpperLimit(kZeroV3),
      m_rotationUpperLimit(kZeroV3),
      m_positionStiffness(kZeroV3),
      m_rotationStiffness(kZeroV3),
      m_type(kGeneric6DofSpringConstraint),
      m_rigidBodyIndex1(-1),
      m_rigidBodyIndex2(-1),
      m_index(-1)
{
}

BaseJoint::~BaseJoint()
{
    internal::deleteObject(m_constraint);
    internal::deleteObject(m_ptr);
    internal::deleteObject(m_name);
    internal::deleteObject(m_englishName);
    m_parentModelRef = 0;
    m_rigidBody1Ref = 0;
    m_rigidBody2Ref = 0;
    m_position.setZero();
    m_rotation.setZero();
    m_positionLowerLimit.setZero();
    m_rotationLowerLimit.setZero();
    m_positionUpperLimit.setZero();
    m_rotationUpperLimit.setZero();
    m_positionStiffness.setZero();
    m_rotationStiffness.setZero();
    m_rigidBodyIndex1 = -1;
    m_rigidBodyIndex2 = -1;
    m_index = -1;
}

void BaseJoint::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_eventRefs.remove(value);
        m_eventRefs.append(value);
    }
}

void BaseJoint::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_eventRefs.remove(value);
    }
}

void BaseJoint::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_eventRefs);
}

void BaseJoint::joinWorld(btDiscreteDynamicsWorld *worldRef)
{
    worldRef->addConstraint(m_constraint);
    m_constraint->setUserConstraintPtr(this);
}

void BaseJoint::leaveWorld(btDiscreteDynamicsWorld *worldRef)
{
    worldRef->removeConstraint(m_constraint);
    m_constraint->setUserConstraintPtr(0);
}

void BaseJoint::updateTransform()
{
    switch (m_constraint->getConstraintType()) {
    case D6_CONSTRAINT_TYPE:
#if BT_BULLET_VERSION > 277
    case D6_SPRING_CONSTRAINT_TYPE:
#endif
    {
        btGeneric6DofSpringConstraint *c = static_cast<btGeneric6DofSpringConstraint *>(m_constraint);
        c->calculateTransforms();
        c->setEquilibriumPoint();
        break;
    }
    default:
        break;
    }
}

btTypedConstraint *BaseJoint::constraint() const VPVL2_DECL_NOEXCEPT
{
    return m_constraint;
}

void *BaseJoint::constraintPtr() const VPVL2_DECL_NOEXCEPT
{
    return m_constraint;
}

IModel *BaseJoint::parentModelRef() const VPVL2_DECL_NOEXCEPT
{
    return m_parentModelRef;
}

IRigidBody *BaseJoint::rigidBody1Ref() const VPVL2_DECL_NOEXCEPT
{
    return m_rigidBody1Ref;
}

IRigidBody *BaseJoint::rigidBody2Ref() const VPVL2_DECL_NOEXCEPT
{
    return m_rigidBody2Ref;
}

int BaseJoint::rigidBodyIndex1() const VPVL2_DECL_NOEXCEPT
{
    return m_rigidBodyIndex1;
}

int BaseJoint::rigidBodyIndex2() const VPVL2_DECL_NOEXCEPT
{
    return m_rigidBodyIndex2;
}

const IString *BaseJoint::name(IEncoding::LanguageType type) const VPVL2_DECL_NOEXCEPT
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

Vector3 BaseJoint::position() const VPVL2_DECL_NOEXCEPT
{
    return m_position;
}

Vector3 BaseJoint::rotation() const VPVL2_DECL_NOEXCEPT
{
    return m_rotation;
}

Vector3 BaseJoint::positionLowerLimit() const VPVL2_DECL_NOEXCEPT
{
    return m_positionLowerLimit;
}

Vector3 BaseJoint::positionUpperLimit() const VPVL2_DECL_NOEXCEPT
{
    return m_positionUpperLimit;
}

Vector3 BaseJoint::rotationLowerLimit() const VPVL2_DECL_NOEXCEPT
{
    return m_rotationLowerLimit;
}

Vector3 BaseJoint::rotationUpperLimit() const VPVL2_DECL_NOEXCEPT
{
    return m_rotationUpperLimit;
}

Vector3 BaseJoint::positionStiffness() const VPVL2_DECL_NOEXCEPT
{
    return m_positionStiffness;
}

Vector3 BaseJoint::rotationStiffness() const VPVL2_DECL_NOEXCEPT
{
    return m_rotationStiffness;
}

BaseJoint::Type BaseJoint::type() const VPVL2_DECL_NOEXCEPT
{
    return m_type;
}

int BaseJoint::index() const VPVL2_DECL_NOEXCEPT
{
    return m_index;
}

void BaseJoint::setParentModelRef(IModel *value)
{
    m_parentModelRef = value;
}

void BaseJoint::setRigidBody1Ref(IRigidBody *value)
{
    if (value != m_rigidBody1Ref) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rigidBody1RefWillChange(value, this));
        m_rigidBody1Ref = value;
        m_rigidBodyIndex1 = value ? value->index() : -1;
    }
}

void BaseJoint::setRigidBody2Ref(IRigidBody *value)
{
    if (value != m_rigidBody2Ref) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rigidBody2RefWillChange(value, this));
        m_rigidBody2Ref = value;
        m_rigidBodyIndex2 = value ? value->index() : -1;
    }
}

void BaseJoint::setName(const IString *value, IEncoding::LanguageType type)
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

void BaseJoint::setPosition(const Vector3 &value)
{
    if (m_position != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, positionWillChange(value, this));
        m_position = value;
    }
}

void BaseJoint::setRotation(const Vector3 &value)
{
    if (m_rotation != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rotationWillChange(value, this));
        m_rotation = value;
    }
}

void BaseJoint::setPositionLowerLimit(const Vector3 &value)
{
    if (m_positionLowerLimit != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, positionLowerLimitWillChange(value, this));
        m_positionLowerLimit = value;
    }
}

void BaseJoint::setPositionUpperLimit(const Vector3 &value)
{
    if (m_positionUpperLimit != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, positionUpperLimitWillChange(value, this));
        m_positionUpperLimit = value;
    }
}

void BaseJoint::setRotationLowerLimit(const Vector3 &value)
{
    if (m_rotationLowerLimit != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rotationLowerLimitWillChange(value, this));
        m_rotationLowerLimit = value;
    }
}

void BaseJoint::setRotationUpperLimit(const Vector3 &value)
{
    if (m_rotationUpperLimit != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rotationUpperLimitWillChange(value, this));
        m_rotationUpperLimit = value;
    }
}

void BaseJoint::setPositionStiffness(const Vector3 &value)
{
    if (m_positionStiffness != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, positionStiffnessWillChange(value, this));
        m_positionStiffness = value;
    }
}

void BaseJoint::setRotationStiffness(const Vector3 &value)
{
    if (m_rotationStiffness != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, rotationStiffnessWillChange(value, this));
        m_rotationStiffness = value;
    }
}

void BaseJoint::setType(Type value)
{
    if (m_type != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_eventRefs, typeWillChange(value, this));
        m_type = value;
    }
}

void BaseJoint::setIndex(int value)
{
    m_index = value;
}

btTypedConstraint *BaseJoint::createConstraint()
{
    btTypedConstraint *ptr = 0;
    switch (m_type) {
    case kGeneric6DofSpringConstraint: {
        btGeneric6DofSpringConstraint *constraint = createGeneric6DofSpringConstraint();
        for (int i = 0; i < 3; i++) {
            const Scalar &value = m_positionStiffness[i];
            if (!btFuzzyZero(value)) {
                constraint->enableSpring(i, true);
                constraint->setStiffness(i, value);
                constraint->setDamping(i, kDefaultDamping);
            }
        }
        for (int i = 0; i < 3; i++) {
            const Scalar &value = m_rotationStiffness[i];
            if (!btFuzzyZero(value)) {
                int index = i + 3;
                constraint->enableSpring(index, true);
                constraint->setStiffness(index, value);
                constraint->setDamping(index, kDefaultDamping);
            }
        }
        constraint->setEquilibriumPoint();
        ptr = constraint;
        break;
    }
    case kGeneric6DofConstraint: {
        btGeneric6DofConstraint *constraint = createGeneric6DofSpringConstraint();
        ptr = constraint;
        break;
    }
    case kPoint2PointConstraint: {
        // FIXME: correct construction parameter
        btRigidBody *bodyA = static_cast<btRigidBody *>(m_rigidBody1Ref->bodyPtr()),
                *bodyB = static_cast<btRigidBody *>(m_rigidBody2Ref->bodyPtr());
        m_ptr = new btPoint2PointConstraint(*bodyA, *bodyB, kZeroV3, kZeroV3);
        btPoint2PointConstraint *constraint = static_cast<btPoint2PointConstraint *>(m_ptr);
        ptr = constraint;
        break;
    }
    case kConeTwistConstraint: {
        Transform worldTransform;
        getJointWorldTransform(worldTransform);
        btRigidBody *bodyA = static_cast<btRigidBody *>(m_rigidBody1Ref->bodyPtr()),
                *bodyB = static_cast<btRigidBody *>(m_rigidBody2Ref->bodyPtr());
        const Transform &transformA = bodyA->getCenterOfMassTransform().inverse() * worldTransform,
                &transformB = bodyB->getCenterOfMassTransform().inverse() * worldTransform;
        m_ptr = new btConeTwistConstraint(*bodyA, *bodyB, transformA, transformB);
        btConeTwistConstraint *constraint = static_cast<btConeTwistConstraint *>(m_ptr);
        constraint->setLimit(m_rotationLowerLimit.x(), m_rotationLowerLimit.y(), m_rotationLowerLimit.z(),
                             m_positionStiffness.x(), m_positionStiffness.y(), m_positionStiffness.z());
        constraint->setDamping(m_positionLowerLimit.x());
        constraint->setFixThresh(m_positionUpperLimit.x());
        bool enableMotor = btFuzzyZero(m_positionLowerLimit.z());
        constraint->enableMotor(enableMotor);
        if (enableMotor) {
            constraint->setMaxMotorImpulse(m_positionUpperLimit.z());
            Quaternion rotation;
#ifdef VPVL2_COORDINATE_OPENGL
            rotation.setEulerZYX(-m_rotationStiffness.x(), -m_rotationStiffness.y(), m_rotationStiffness.z());
#else
            rotation.setEuler(m_rotationStiffness.x(), m_rotationStiffness.y(), m_rotationStiffness.z());
#endif
            constraint->setMotorTargetInConstraintSpace(rotation);
        }
        ptr = constraint;
        break;
    }
    case kSliderConstraint: {
        Transform worldTransform;
        getJointWorldTransform(worldTransform);
        btRigidBody *bodyA = static_cast<btRigidBody *>(m_rigidBody1Ref->bodyPtr()),
                *bodyB = static_cast<btRigidBody *>(m_rigidBody2Ref->bodyPtr());
        const Transform &frameInA = bodyA->getCenterOfMassTransform().inverse() * worldTransform,
                &frameInB = bodyB->getCenterOfMassTransform().inverse() * worldTransform;
        m_ptr = new btSliderConstraint(*bodyA, *bodyB, frameInA, frameInB, true);
        btSliderConstraint *constraint = static_cast<btSliderConstraint *>(m_ptr);
        constraint->setLowerLinLimit(m_positionLowerLimit.x());
        constraint->setUpperLinLimit(m_positionUpperLimit.x());
        constraint->setLowerAngLimit(m_rotationLowerLimit.x());
        constraint->setUpperAngLimit(m_rotationUpperLimit.x());
        bool enablePoweredLinMotor = btFuzzyZero(m_positionStiffness.x());
        constraint->setPoweredLinMotor(enablePoweredLinMotor);
        if (enablePoweredLinMotor) {
            constraint->setTargetLinMotorVelocity(m_positionStiffness.y());
            constraint->setMaxLinMotorForce(m_positionStiffness.z());
        }
        bool enablePoweredAngMotor = btFuzzyZero(m_rotationStiffness.x());
        constraint->setPoweredAngMotor(enablePoweredAngMotor);
        if (enablePoweredAngMotor) {
            constraint->setTargetAngMotorVelocity(m_rotationStiffness.y());
            constraint->setMaxAngMotorForce(m_rotationStiffness.z());
        }
        ptr = constraint;
        break;
    }
    case kHingeConstraint: {
        Transform worldTransform;
        getJointWorldTransform(worldTransform);
        btRigidBody *bodyA = static_cast<btRigidBody *>(m_rigidBody1Ref->bodyPtr()),
                *bodyB = static_cast<btRigidBody *>(m_rigidBody2Ref->bodyPtr());
        const Transform &transformA = bodyA->getCenterOfMassTransform().inverse() * worldTransform,
                &transformB = bodyB->getCenterOfMassTransform().inverse() * worldTransform;
        m_ptr = new btHingeConstraint(*bodyA, *bodyB, transformA, transformB);
        btHingeConstraint *constraint = static_cast<btHingeConstraint *>(m_ptr);
        constraint->setLimit(m_rotationLowerLimit.x(), m_rotationUpperLimit.y(), m_positionStiffness.x(),
                             m_positionStiffness.y(), m_positionStiffness.z());
        bool enableMotor = btFuzzyZero(m_rotationStiffness.z());
        constraint->enableMotor(enableMotor);
        if (enableMotor) {
            constraint->enableAngularMotor(enableMotor, m_rotationStiffness.y(), m_rotationStiffness.z());
        }
        ptr = constraint;
        break;
    }
    case kMaxType:
    default:
        VPVL2_CHECK(0); /* should not be reached here */
        break;
    }
    if (ptr) {
        ptr->setUserConstraintPtr(this);
    }
    return ptr;
}

btGeneric6DofSpringConstraint *BaseJoint::createGeneric6DofSpringConstraint()
{
    btRigidBody *bodyA = static_cast<btRigidBody *>(m_rigidBody1Ref->bodyPtr()),
            *bodyB = static_cast<btRigidBody *>(m_rigidBody2Ref->bodyPtr());
    Transform transform;
    getJointWorldTransform(transform);
    const Transform &transformA = bodyA->getCenterOfMassTransform().inverse() * transform,
            &transformB = bodyB->getCenterOfMassTransform().inverse() * transform;
    internal::deleteObject(m_ptr);
    m_ptr = new btGeneric6DofSpringConstraint(*bodyA, *bodyB, transformA, transformB, true);
    btGeneric6DofSpringConstraint *constraint = static_cast<btGeneric6DofSpringConstraint *>(m_ptr);
#ifdef VPVL2_COORDINATE_OPENGL
    constraint->setLinearUpperLimit(Vector3(m_positionUpperLimit[0], m_positionUpperLimit[1], -m_positionLowerLimit[2]));
    constraint->setLinearLowerLimit(Vector3(m_positionLowerLimit[0], m_positionLowerLimit[1], -m_positionUpperLimit[2]));
    constraint->setAngularUpperLimit(Vector3(-m_rotationLowerLimit[0], -m_rotationLowerLimit[1], m_rotationUpperLimit[2]));
    constraint->setAngularLowerLimit(Vector3(-m_rotationUpperLimit[0], -m_rotationUpperLimit[1], m_rotationLowerLimit[2]));
#else  /* VPVL2_COORDINATE_OPENGL */
    constraint->setLinearUpperLimit(m_positionLowerLimit);
    constraint->setLinearLowerLimit(m_positionUpperLimit);
    constraint->setAngularUpperLimit(m_rotationLowerLimit);
    constraint->setAngularLowerLimit(m_rotationUpperLimit);
#endif /* VPVL2_COORDINATE_OPENGL */
    return constraint;
}

void BaseJoint::getJointWorldTransform(Transform &worldTransform) const
{
    Matrix3x3 basis;
#ifdef VPVL2_COORDINATE_OPENGL
    Matrix3x3 mx, my, mz;
    mx.setEulerZYX(-m_rotation[0], 0.0f, 0.0f);
    my.setEulerZYX(0.0f, -m_rotation[1], 0.0f);
    mz.setEulerZYX(0.0f, 0.0f, m_rotation[2]);
    basis = my * mz * mx;
    worldTransform.setOrigin(Vector3(m_position[0], m_position[1], -m_position[2]));
#else  /* VPVL2_COORDINATE_OPENGL */
    basis.setEulerZYX(m_rotation[0], m_rotation[1], m_rotation[2]);
    transform.setOrigin(m_position);
#endif /* VPVL2_COORDINATE_OPENGL */
    worldTransform.setBasis(basis);
}

void BaseJoint::build(int index)
{
    if (m_rigidBody1Ref && m_rigidBody2Ref) {
        internal::deleteObject(m_constraint);
        m_constraint = createConstraint();
        static_cast<btRigidBody *>(m_rigidBody1Ref->bodyPtr())->addConstraintRef(m_constraint);
        static_cast<btRigidBody *>(m_rigidBody2Ref->bodyPtr())->addConstraintRef(m_constraint);
        m_ptr = 0;
    }
    m_index = index;
}

} /* namespace internal */
} /* namespace vpvl2 */
