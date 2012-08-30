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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/RigidBody.h"

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btCollisionShape)
BT_DECLARE_HANDLE(btMotionState)
BT_DECLARE_HANDLE(btRigidBody)
#endif

namespace
{

    static vpvl2::pmx::Bone kNullBone;

#pragma pack(push, 1)

    struct RigidBodyUnit
    {
        uint8_t collisionGroupID;
        uint16_t collsionMask;
        uint8_t shapeType;
        float size[3];
        float position[3];
        float rotation[3];
        float mass;
        float linearDamping;
        float angularDamping;
        float restitution;
        float friction;
        uint8_t type;
    };

#pragma pack(pop)

#ifndef VPVL2_NO_BULLET
    using namespace vpvl2;
    using namespace vpvl2::pmx;

    class AlignedMotionState : public btMotionState
    {
    public:
        AlignedMotionState(const Transform &startTransform, const Transform &boneTransform, const Bone *bone)
            : m_bone(bone),
              m_boneTransform(boneTransform),
              m_inversedBoneTransform(boneTransform.inverse()),
              m_worldTransform(startTransform)
        {
        }
        ~AlignedMotionState() {}

        void getWorldTransform(btTransform &worldTrans) const {
            worldTrans = m_worldTransform;
        }
        void setWorldTransform(const btTransform &worldTrans) {
            m_worldTransform = worldTrans;
            const Matrix3x3 &matrix = worldTrans.getBasis();
            m_worldTransform.setOrigin(kZeroV3);
            m_worldTransform = m_boneTransform * m_worldTransform;
            m_worldTransform.setOrigin(m_worldTransform.getOrigin() + m_bone->localTransform().getOrigin());
            m_worldTransform.setBasis(matrix);
        }

    private:
        const Bone *m_bone;
        const Transform m_boneTransform;
        const Transform m_inversedBoneTransform;
        Transform m_worldTransform;
    };

    class KinematicMotionState : public btMotionState
    {
    public:
        KinematicMotionState(const Transform &boneTransform, const Bone *bone)
            : m_bone(bone),
              m_boneTransform(boneTransform)
        {
        }
        ~KinematicMotionState() {}

        void getWorldTransform(btTransform &worldTrans) const {
            worldTrans = m_bone->localTransform() * m_boneTransform;
        }
        void setWorldTransform(const btTransform & /* worldTrans */) {}

    private:
        const Bone *m_bone;
        const Transform m_boneTransform;
    };
#endif /* VPVL2_NO_BULLET */

}

namespace vpvl2
{
namespace pmx
{

RigidBody::RigidBody()
    : m_body(0),
      m_shape(0),
      m_motionState(0),
      m_kinematicMotionState(0),
      m_transform(Transform::getIdentity()),
      m_invertedTransform(Transform::getIdentity()),
      m_bone(0),
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
      m_shapeType(0),
      m_type(0)
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
    m_bone = 0;
    m_boneIndex = 0;
    m_transform.setIdentity();
    m_invertedTransform.setIdentity();
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
    m_shapeType = 0;
    m_type = 0;
}

bool RigidBody::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size, boneIndexSize = info.boneIndexSize;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.rigidBodiesPtr = ptr;
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
        if (!internal::validateSize(ptr, boneIndexSize + sizeof(RigidBodyUnit), rest)) {
            return false;
        }
    }
    info.rigidBodiesCount = size;
    return true;
}

bool RigidBody::loadRigidBodies(const Array<RigidBody *> &rigidBodies, const Array<Bone *> &bones)
{
    const int nRigidBodies = rigidBodies.count();
    const int nbones = bones.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = rigidBodies[i];
        const int boneIndex = rigidBody->m_boneIndex;
        if (boneIndex >= 0) {
            if (boneIndex == 0xffff)
                rigidBody->m_bone = 0;
            else if (boneIndex >= nbones)
                return false;
            else {
                btCollisionShape *shape = rigidBody->createShape();
                Bone *bone = bones[boneIndex];
                if (rigidBody->m_type != 0)
                    bone->setSimulated(true);
                rigidBody->m_shape = shape;
                rigidBody->m_bone = bone;
                rigidBody->m_body = rigidBody->createRigidBody(shape);
            }
        }
        else {
            rigidBody->m_bone = &kNullBone;
            rigidBody->m_body = rigidBody->createRigidBody(0);
        }
        rigidBody->m_index = i;
    }
    return true;
}

void RigidBody::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    IEncoding *encoding = info.encoding;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    m_boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
    RigidBodyUnit unit;
    internal::getData(ptr, unit);
    m_collisionGroupID = unit.collisionGroupID;
    m_groupID = 0x0001 << m_collisionGroupID;
    m_collisionGroupMask = unit.collsionMask;
    m_shapeType = unit.shapeType;
    internal::setPositionRaw(unit.size, m_size);
    internal::setPosition(unit.position, m_position);
    internal::setPositionRaw(unit.rotation, m_rotation);
    m_mass = unit.mass;
    m_linearDamping = unit.linearDamping;
    m_angularDamping = unit.angularDamping;
    m_restitution = unit.restitution;
    m_friction = unit.friction;
    m_type = unit.type;
    ptr += sizeof(unit);
    size = ptr - start;
}

void RigidBody::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, data);
    internal::writeString(m_englishName, data);
    internal::writeSignedIndex(m_boneIndex, info.boneIndexSize, data);
    RigidBodyUnit rbu;
    rbu.angularDamping = m_angularDamping;
    rbu.collisionGroupID = m_collisionGroupID;
    rbu.collsionMask = m_collisionGroupMask;
    rbu.friction = m_friction;
    rbu.linearDamping = m_linearDamping;
    rbu.mass = m_mass;
    internal::getPosition(m_position, &rbu.position[0]);
    rbu.restitution = m_restitution;
    internal::getPositionRaw(m_rotation, &rbu.rotation[0]);
    rbu.shapeType = m_shapeType;
    internal::getPositionRaw(m_size, rbu.size);
    rbu.type = m_type;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&rbu), sizeof(rbu), data);
}

size_t RigidBody::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name);
    size += internal::estimateSize(m_englishName);
    size += info.boneIndexSize;
    size += sizeof(RigidBodyUnit);
    return size;
}

void RigidBody::performTransformBone()
{
#ifndef VPVL2_NO_BULLET
    if (m_type == 0 || !m_bone)
        return;
    const Transform &transform = m_body->getCenterOfMassTransform() * m_invertedTransform;
    m_bone->setLocalTransform(transform);
#endif /* VPVL2_NO_BULLET */
}

void RigidBody::setKinematic(bool value)
{
#ifndef VPVL2_NO_BULLET
    if (m_type == 0)
        return;
    if (value) {
        m_body->setMotionState(m_kinematicMotionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    else {
        Transform transform;
        m_kinematicMotionState->getWorldTransform(transform);
        m_motionState->setWorldTransform(transform);
        m_body->setMotionState(m_motionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }
#else  /* VPVL2_NO_BULLET */
    (void) value;
#endif /* VPVL2_NO_BULLET */
}

const Transform RigidBody::createStartTransform(Transform &base) const
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
    base.setBasis(basis);
    base.setOrigin(m_position);
    Transform startTransform = Transform::getIdentity();
    startTransform.setOrigin(m_bone->localTransform().getOrigin());
    startTransform *= base;
    return startTransform;
}

btCollisionShape *RigidBody::createShape() const
{
    switch (m_shapeType) {
    case 0:
        return new btSphereShape(m_size.x());
    case 1:
        return new btBoxShape(m_size);
    case 2:
        return new btCapsuleShape(m_size.x(), m_size.y());
    default:
        return 0;
    }
}

btRigidBody *RigidBody::createRigidBody(btCollisionShape *shape)
{
    Vector3 localInertia = kZeroV3;
    Scalar massValue = 0.0f;
    if (m_type != 0) {
        massValue = m_mass;
        if (shape && massValue != 0.0f)
            shape->calculateLocalInertia(massValue, localInertia);
    }
    const Transform &startTransform = createStartTransform(m_transform);
    m_invertedTransform = m_transform.inverse();
    switch (m_type) {
    case 0:
        m_motionState = new KinematicMotionState(m_transform, m_bone);
        m_kinematicMotionState = 0;
        break;
    case 1:
        m_motionState = new btDefaultMotionState(startTransform);
        m_kinematicMotionState = new KinematicMotionState(m_transform, m_bone);
        break;
    default:
        m_motionState = new AlignedMotionState(startTransform, m_transform, m_bone);
        m_kinematicMotionState = new KinematicMotionState(m_transform, m_bone);
        break;
    }
    btRigidBody::btRigidBodyConstructionInfo info(massValue, m_motionState, shape, localInertia);
    info.m_linearDamping = m_linearDamping;
    info.m_angularDamping = m_angularDamping;
    info.m_restitution = m_restitution;
    info.m_friction = m_friction;
    info.m_additionalDamping = true;
    btRigidBody *body = new btRigidBody(info);
    if (m_type == 0)
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

void RigidBody::setBone(Bone *value)
{
    m_bone = value;
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

void RigidBody::setShapeType(uint8_t value)
{
    m_shapeType = value;
}

void RigidBody::setSize(const Vector3 &value)
{
    m_size = value;
}

void RigidBody::setType(uint8_t value)
{
    m_type = value;
}

void RigidBody::setIndex(int value)
{
    m_index = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */

