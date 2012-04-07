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

#ifndef VPVL2_NO_BULLET
#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#else
BT_DECLARE_HANDLE(btGeneric6DofConstraint)
BT_DECLARE_HANDLE(btGeneric6DofSpringConstraint)
#endif

namespace
{

#pragma pack(push, 1)

    struct JointUnit
    {
        float position[3];
        float rotation[3];
        float positionLowerLimit[3];
        float positionUpperLimit[3];
        float rotationLowerLimit[3];
        float rotationUpperLimit[3];
        float positionStiffness[3];
        float rotationStiffness[3];
    };

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Joint::Joint()
    : m_constraint(0),
      m_rigidBody1(0),
      m_rigidBody2(0),
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
      m_rigidBodyIndex1(0),
      m_rigidBodyIndex2(0)
{
}

Joint::~Joint()
{
    delete m_constraint;
    m_constraint = 0;
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_rigidBody1 = 0;
    m_rigidBody2 = 0;
    m_position.setZero();
    m_rotation.setZero();
    m_positionLowerLimit.setZero();
    m_rotationLowerLimit.setZero();
    m_positionUpperLimit.setZero();
    m_rotationUpperLimit.setZero();
    m_positionStiffness.setZero();
    m_rotationStiffness.setZero();
    m_rigidBodyIndex1 = 0;
    m_rigidBodyIndex2 = 0;
}

bool Joint::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.jointsPtr = ptr;
    for (size_t i = 0; i < size; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        size_t type;
        if (!internal::size8(ptr, rest, type)) {
            return false;
        }
        switch (type) {
        case 0:
            if (!internal::validateSize(ptr, info.rigidBodyIndexSize * 2 + sizeof(JointUnit), rest)) {
                return false;
            }
            break;
        default:
            return false;
        }
    }
    info.jointsCount = size;
    return true;
}

bool Joint::loadJoints(const Array<Joint *> &joints, const Array<RigidBody *> &rigidBodies)
{
    const int njoints = joints.count();
    const int nRigidBodies = rigidBodies.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        const int rigidBodyIndex1 = joint->m_rigidBodyIndex1;
        if (rigidBodyIndex1 >= 0) {
            if (rigidBodyIndex1 >= nRigidBodies)
                return false;
            else
                joint->m_rigidBody1 = rigidBodies[rigidBodyIndex1];
        }
        const int rigidBodyIndex2 = joint->m_rigidBodyIndex2;
        if (rigidBodyIndex2 >= 0) {
            if (rigidBodyIndex2 >= nRigidBodies)
                return false;
            else
                joint->m_rigidBody2 = rigidBodies[rigidBodyIndex2];
        }
        if (joint->m_rigidBody1 && joint->m_rigidBody2)
            joint->m_constraint = joint->createConstraint();
    }
    return true;
}

void Joint::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_name = info.encoding->toString(namePtr, nNameSize, info.codec);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = info.encoding->toString(namePtr, nNameSize, info.codec);
    internal::size8(ptr, rest, nNameSize);
    switch (nNameSize) {
    case 0: {
        m_rigidBodyIndex1 = internal::readSignedIndex(ptr, info.rigidBodyIndexSize);
        m_rigidBodyIndex2 = internal::readSignedIndex(ptr, info.rigidBodyIndexSize);
        const JointUnit &unit = *reinterpret_cast<JointUnit *>(ptr);
        m_position.setValue(unit.position[0], unit.position[1], unit.position[2]);
        m_rotation.setValue(unit.rotation[0], unit.rotation[1], unit.rotation[2]);
        m_positionLowerLimit.setValue(unit.positionLowerLimit[0], unit.positionLowerLimit[1], unit.positionLowerLimit[2]);
        m_rotationLowerLimit.setValue(unit.rotationLowerLimit[0], unit.rotationLowerLimit[1], unit.rotationLowerLimit[2]);
        m_positionUpperLimit.setValue(unit.positionUpperLimit[0], unit.positionUpperLimit[1], unit.positionUpperLimit[2]);
        m_rotationUpperLimit.setValue(unit.rotationUpperLimit[0], unit.rotationUpperLimit[1], unit.rotationUpperLimit[2]);
        m_positionStiffness.setValue(unit.positionStiffness[0], unit.positionStiffness[1], unit.positionStiffness[2]);
        m_rotationStiffness.setValue(unit.rotationStiffness[0], unit.rotationStiffness[1], unit.rotationStiffness[2]);
        ptr += sizeof(unit);
        break;
    }
    default:
        assert(0);
        return;
    }
    size = ptr - start;
}

void Joint::write(uint8_t * /* data */) const
{
}

btGeneric6DofSpringConstraint *Joint::createConstraint() const
{
#ifndef VPVL2_NO_BULLET
    Transform transform = Transform::getIdentity();
    btMatrix3x3 basis;
    const Vector3 &position = m_position;
    const Vector3 &rotation = m_rotation;
#ifdef VPVL2_COORDINATE_OPENGL
    btMatrix3x3 mx, my, mz;
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
    btRigidBody *bodyA = m_rigidBody1->body(), *bodyB = m_rigidBody2->body();
    Transform transformA = bodyA->getWorldTransform().inverse() * transform,
            transformB = bodyB->getWorldTransform().inverse() * transform;
    btGeneric6DofSpringConstraint *constraint = new btGeneric6DofSpringConstraint(*bodyA, *bodyB, transformA, transformB, true);
    const Vector3 &positionLowerLimit = m_positionLowerLimit;
    const Vector3 &positionUpperLimit = m_positionUpperLimit;
    const Vector3 &rotationLowerLimit = m_positionLowerLimit;
    const Vector3 &rotationUpperLimit = m_positionUpperLimit;
#ifdef VPVL2_COORDINATE_OPENGL
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
        if (const Scalar &value = m_rotationStiffness[i]) {
            int index = i + 3;
            constraint->enableSpring(index, true);
            constraint->setStiffness(index, value);
        }
    }
    return constraint;
#else /* VPVL2_NO_BULLET */
    return 0;
#endif
}

} /* namespace pmx */
} /* namespace vpvl2 */

