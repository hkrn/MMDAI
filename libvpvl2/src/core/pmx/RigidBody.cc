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
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btCollisionShape)
BT_DECLARE_HANDLE(btMotionState)
BT_DECLARE_HANDLE(btRigidBody)
#endif

namespace
{

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
    AlignedMotionState(const Transform &startTransform, const Transform &boneTransform, Bone *bone)
        : m_bone(bone),
          m_boneTransform(boneTransform),
          m_inversedBoneTransform(boneTransform.inverse()),
          m_worldTransform(startTransform)
    {
    }
    virtual ~AlignedMotionState()
    {
    }
    virtual void getWorldTransform(btTransform &worldTrans) const
    {
        worldTrans = m_worldTransform;
    }
    virtual void setWorldTransform(const btTransform &worldTrans)
    {
        m_worldTransform = worldTrans;
        const btMatrix3x3 &matrix = worldTrans.getBasis();
        m_worldTransform.setOrigin(kZeroV3);
        m_worldTransform = m_boneTransform * m_worldTransform;
        m_worldTransform.setOrigin(m_worldTransform.getOrigin() + m_bone->localTransform().getOrigin());
        m_worldTransform.setBasis(matrix);
    }
private:
    Bone *m_bone;
    Transform m_boneTransform;
    Transform m_inversedBoneTransform;
    Transform m_worldTransform;
};

class KinematicMotionState : public btMotionState
{
public:
    KinematicMotionState(const Transform &boneTransform, Bone *bone)
        : m_bone(bone),
          m_boneTransform(boneTransform)
    {
    }
    virtual ~KinematicMotionState()
    {
    }
    virtual void getWorldTransform(btTransform &worldTrans) const
    {
        worldTrans = m_bone->localTransform() * m_boneTransform;
    }
    virtual void setWorldTransform(const btTransform &worldTrans)
    {
        (void) worldTrans;
    }
private:
    Bone *m_bone;
    Transform m_boneTransform;
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
      m_groupID(0),
      m_groupMask(0),
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
    m_groupID = 0;
    m_groupMask = 0;
    m_collisionGroupID = 0;
    m_shapeType = 0;
    m_type = 0;
}

bool RigidBody::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
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
        if (!internal::validateSize(ptr, info.boneIndexSize + sizeof(RigidBodyUnit), rest)) {
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
                rigidBody->m_bone = bones[boneIndex];
                rigidBody->m_body = rigidBody->createRigidBody(rigidBody->createShape());
            }
        }
    }
    return true;
}

void RigidBody::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_name = info.encoding->toString(namePtr, nNameSize, info.codec);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = info.encoding->toString(namePtr, nNameSize, info.codec);
    m_boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
    const RigidBodyUnit &unit = *reinterpret_cast<RigidBodyUnit *>(ptr);
    m_collisionGroupID = unit.collisionGroupID;
    m_groupMask = unit.collsionMask;
    m_shapeType = unit.shapeType;
    m_size.setValue(unit.size[0], unit.size[1], unit.size[2]);
    internal::setPosition(unit.position, m_position);
    m_rotation.setValue(unit.rotation[0], unit.rotation[1], unit.rotation[2]);
    m_mass = unit.mass;
    m_linearDamping = unit.linearDamping;
    m_angularDamping = unit.angularDamping;
    m_restitution = unit.restitution;
    m_friction = unit.friction;
    m_type = unit.type;
    ptr += sizeof(unit);
    size = ptr - start;
}

void RigidBody::write(uint8_t * /* data */) const
{
}

void RigidBody::performTransformBone()
{
#ifndef VPVL2_NO_BULLET
    if (m_type == 0 || !m_bone)
        return;
    m_bone->setLocalTransform(m_body->getCenterOfMassTransform() * m_invertedTransform);
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
    btMatrix3x3 basis;
    base.setIdentity();
#ifdef VPVL2_COORDINATE_OPENGL
    btMatrix3x3 mx, my, mz;
    mx.setEulerZYX(-m_rotation[0], 0.0f, 0.0f);
    my.setEulerZYX(0.0f, -m_rotation[1], 0.0f);
    mz.setEulerZYX(0.0f, 0.0f, m_rotation[2]);
    basis = my * mz * mx;
    base.setOrigin(Vector3(m_position[0], m_position[1], -m_position[2]));
#else  /* VPVL2_COORDINATE_OPENGL */
    basis.setEulerZYX(m_rotation[0], m_rotation[1], m_rotation[2]);
    base.setOrigin(m_position);
#endif /* VPVL2_COORDINATE_OPENGL */
    base.setBasis(basis);
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
        if (massValue != 0.0f)
            shape->calculateLocalInertia(massValue, localInertia);
    }
    const Transform &startTransform = createStartTransform(m_transform);
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
    return new btRigidBody(info);
}

} /* namespace pmx */
} /* namespace vpvl2 */

