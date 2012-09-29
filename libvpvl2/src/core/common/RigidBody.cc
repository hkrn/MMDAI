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

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btCollisionShape)
BT_DECLARE_HANDLE(btMotionState)
BT_DECLARE_HANDLE(btRigidBody)
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

        void getWorldTransform(btTransform &worldTrans) const {
            worldTrans = m_worldTransform;
        }
        void setWorldTransform(const btTransform &worldTrans) {
            m_worldTransform = worldTrans * m_boneRef->localTransform();
        }
        void resetWorldTransform(const Transform &value) {
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

        void getWorldTransform(btTransform &worldTrans) const {
            worldTrans = m_worldTransform;
        }
        void setWorldTransform(const btTransform &worldTrans) {
            m_worldTransform = worldTrans;
            const Matrix3x3 &rotation = worldTrans.getBasis();
            m_worldTransform.setOrigin(kZeroV3);
            m_worldTransform = m_startTransform * m_worldTransform;
            m_worldTransform.setOrigin(m_worldTransform.getOrigin() + m_boneRef->localTransform().getOrigin());
            m_worldTransform.setBasis(rotation);
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

        void getWorldTransform(btTransform &worldTrans) const {
            // Bone#localTransform cannot use at setKinematics because it's called after performTransformBone
            // (Bone#localTransform will be identity)
            Transform output;
            m_boneRef->getLocalTransform(output);
            worldTrans = output * m_startTransform;
        }
        void setWorldTransform(const btTransform & /* worldTrans */) {
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
      m_shape(0),
      m_motionState(0),
      m_kinematicMotionState(0),
      m_worldTransform(Transform::getIdentity()),
      m_world2LocalTransform(Transform::getIdentity()),
      m_boneRef(0),
      m_name(0),
      m_englishName(0),
      m_boneIndex(0),
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
    m_boneIndex = 0;
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
    if (m_type == kStaticObject || !m_boneRef)
        return;
    const Transform &worldTransform = m_body->getCenterOfMassTransform();
    const Transform &localTransform = worldTransform * m_world2LocalTransform;
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
        Transform worldTransform;
        m_kinematicMotionState->getWorldTransform(worldTransform);
        DefaultMotionState *motionState = static_cast<DefaultMotionState *>(m_motionState);
        motionState->resetWorldTransform(worldTransform);
        m_body->setMotionState(m_motionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }
#else  /* VPVL2_NO_BULLET */
    (void) value;
#endif /* VPVL2_NO_BULLET */
}

const Transform RigidBody::createStartTransform() const
{
    Matrix3x3 basis;
#ifdef VPVL2_COORDINATE_OPENGL
    Matrix3x3 mx, my, mz;
    mx.setEulerZYX(-m_rotation[0], 0.0f, 0.0f);
    my.setEulerZYX(0.0f, -m_rotation[1], 0.0f);
    mz.setEulerZYX(0.0f, 0.0f, m_rotation[2]);
    basis = mz * my * mx;// my * mz * mx;
#else  /* VPVL2_COORDINATE_OPENGL */
    basis.setEulerZYX(m_rotation[0], m_rotation[1], m_rotation[2]);
#endif /* VPVL2_COORDINATE_OPENGL */
    return Transform(basis, m_position);
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
    default:
        return 0;
    }
    return 0;
}

btRigidBody *RigidBody::createRigidBody(btCollisionShape *shape)
{
    Vector3 localInertia = kZeroV3;
    Scalar massValue = 0.0f;
    if (m_type != kStaticObject) {
        massValue = m_mass;
        if (shape && massValue != 0.0f)
            shape->calculateLocalInertia(massValue, localInertia);
    }
    m_worldTransform = createStartTransform();
    m_world2LocalTransform = m_worldTransform.inverse();
    switch (m_type) {
    default:
    case kStaticObject:
        m_motionState = new KinematicMotionState(m_worldTransform, m_boneRef);
        m_kinematicMotionState = 0;
        break;
    case kDynamicObject:
        m_motionState = new DefaultMotionState(m_worldTransform, m_boneRef);
        m_kinematicMotionState = new KinematicMotionState(m_worldTransform, m_boneRef);
        break;
    case kAlignedObject:
        m_motionState = new AlignedMotionState(m_worldTransform, m_boneRef);
        m_kinematicMotionState = new KinematicMotionState(m_worldTransform, m_boneRef);
        break;
    }
    btRigidBody::btRigidBodyConstructionInfo info(massValue, m_motionState, shape, localInertia);
    info.m_linearDamping = m_linearDamping;
    info.m_angularDamping = m_angularDamping;
    info.m_restitution = m_restitution;
    info.m_friction = m_friction;
    info.m_additionalDamping = true;
    btRigidBody *body = new btRigidBody(info);
    if (m_type == kStaticObject)
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    body->setActivationState(DISABLE_DEACTIVATION);
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

} /* namespace pmx */
} /* namespace vpvl2 */

