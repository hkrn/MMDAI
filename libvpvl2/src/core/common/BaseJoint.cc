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

#include "vpvl2/common/BaseJoint.h"

#ifndef VPVL2_NO_BULLET
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#else
BT_DECLARE_HANDLE(btGeneric6DofConstraint);
BT_DECLARE_HANDLE(btGeneric6DofSpringConstraint);
#endif

namespace vpvl2
{
namespace common
{

BaseJoint::BaseJoint()
    : m_constraint(0),
      m_ptr(0),
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
      m_rigidBodyIndex1(-1),
      m_rigidBodyIndex2(-1),
      m_index(-1)
{
}

BaseJoint::~BaseJoint()
{
    delete m_constraint;
    m_constraint = 0;
    delete m_ptr;
    m_ptr = 0;
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
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

void BaseJoint::setRigidBody1(BaseRigidBody *value)
{
    m_rigidBody1Ref = value;
    m_rigidBodyIndex1 = value ? value->index() : -1;
}

void BaseJoint::setRigidBody2(BaseRigidBody *value)
{
    m_rigidBody2Ref = value;
    m_rigidBodyIndex2 = value ? value->index() : -1;
}

void BaseJoint::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void BaseJoint::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void BaseJoint::setPosition(const Vector3 &value)
{
    m_position = value;
}

void BaseJoint::setRotation(const Vector3 &value)
{
    m_rotation = value;
}

void BaseJoint::setPositionLowerLimit(const Vector3 &value)
{
    m_positionLowerLimit = value;
}

void BaseJoint::setPositionUpperLimit(const Vector3 &value)
{
    m_positionUpperLimit = value;
}

void BaseJoint::setRotationLowerLimit(const Vector3 &value)
{
    m_rotationLowerLimit = value;
}

void BaseJoint::setRotationUpperLimit(const Vector3 &value)
{
    m_rotationUpperLimit = value;
}

void BaseJoint::setPositionStiffness(const Vector3 &value)
{
    m_positionStiffness = value;
}

void BaseJoint::setRotationStiffness(const Vector3 &value)
{
    m_rotationStiffness = value;
}

void BaseJoint::setIndex(int value)
{
    m_index = value;
}

btGeneric6DofSpringConstraint *BaseJoint::createConstraint()
{
#ifndef VPVL2_NO_BULLET
    Transform transform = Transform::getIdentity();
    Matrix3x3 basis;
    const Vector3 &position = m_position;
    const Vector3 &rotation = m_rotation;
#ifdef VPVL2_COORDINATE_OPENGL
    Matrix3x3 mx, my, mz;
    mx.setEulerZYX(-rotation[0], 0.0f, 0.0f);
    my.setEulerZYX(0.0f, -rotation[1], 0.0f);
    mz.setEulerZYX(0.0f, 0.0f, rotation[2]);
    basis = my * mz * mx;
    transform.setOrigin(Vector3(position[0], position[1], -position[2]));
#else  /* VPVL2_COORDINATE_OPENGL */
    basis.setEulerZYX(rotation[0], rotation[1], rotation[2]);
    transform.setOrigin(m_position);
#endif /* VPVL2_COORDINATE_OPENGL */
    transform.setBasis(basis);
    btRigidBody *bodyA = m_rigidBody1Ref->body(), *bodyB = m_rigidBody2Ref->body();
    const Transform &transformA = bodyA->getWorldTransform().inverse() * transform,
            &transformB = bodyB->getWorldTransform().inverse() * transform;
    btGeneric6DofSpringConstraint *constraint = m_ptr = new btGeneric6DofSpringConstraint(*bodyA, *bodyB, transformA, transformB, true);
#ifdef VPVL2_COORDINATE_OPENGL
    const Vector3 &positionLowerLimit = m_positionLowerLimit;
    const Vector3 &positionUpperLimit = m_positionUpperLimit;
    const Vector3 &rotationLowerLimit = m_positionLowerLimit;
    const Vector3 &rotationUpperLimit = m_positionUpperLimit;
    constraint->setLinearUpperLimit(Vector3(positionUpperLimit[0], positionUpperLimit[1], -positionLowerLimit[2]));
    constraint->setLinearLowerLimit(Vector3(positionLowerLimit[0], positionLowerLimit[1], -positionUpperLimit[2]));
    constraint->setAngularUpperLimit(Vector3(-rotationLowerLimit[0], -rotationLowerLimit[1], rotationUpperLimit[2]));
    constraint->setAngularLowerLimit(Vector3(-rotationUpperLimit[0], -rotationUpperLimit[1], rotationLowerLimit[2]));
#else  /* VPVL2_COORDINATE_OPENGL */
    constraint->setLinearUpperLimit(m_positionLowerLimit);
    constraint->setLinearLowerLimit(m_positionUpperLimit);
    constraint->setAngularUpperLimit(m_rotationLowerLimit);
    constraint->setAngularLowerLimit(m_rotationUpperLimit);
#endif /* VPVL2_COORDINATE_OPENGL */
    for (int i = 0; i < 3; i++) {
        const Scalar &value = m_positionStiffness[i];
        if (!btFuzzyZero(value)) {
            constraint->enableSpring(i, true);
            constraint->setStiffness(i, value);
        }
    }
    for (int i = 0; i < 3; i++) {
        const Scalar &value = m_rotationStiffness[i];
        int index = i + 3;
        constraint->enableSpring(index, true);
        constraint->setStiffness(index, value);
    }
    constraint->setEquilibriumPoint();
    constraint->setUserConstraintPtr(this);
    return constraint;
#else /* VPVL2_NO_BULLET */
    return 0;
#endif
}

void BaseJoint::build(int index)
{
    if (m_rigidBody1Ref && m_rigidBody2Ref) {
        m_constraint = createConstraint();
        m_ptr = 0;
    }
    m_index = index;
}

} /* namespace pmx */
} /* namespace vpvl2 */

