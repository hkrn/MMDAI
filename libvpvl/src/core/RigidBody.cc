/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

#ifndef VPVL_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
VPVL_DECLARE_HANDLE(btCollisionShape)
VPVL_DECLARE_HANDLE(btMotionState)
VPVL_DECLARE_HANDLE(btRigidBody)
#endif

namespace vpvl
{

#pragma pack(push, 1)

struct RigidBodyChunk
{
    uint8_t name[RigidBody::kNameSize];
    uint16_t boneID;
    uint8_t collisionGroupID;
    uint16_t collsionMask;
    uint8_t shapeType;
    float width;
    float height;
    float depth;
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

#ifndef VPVL_NO_BULLET
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
        m_worldTransform.setOrigin(kZeroV);
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
#endif /* VPVL_NO_BULLET */

size_t RigidBody::stride()
{
    return sizeof(RigidBodyChunk);
}

RigidBody::RigidBody()
    : m_bone(0),
      m_shape(0),
      m_body(0),
      m_motionState(0),
      m_kinematicMotionState(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f),
      m_width(0.0f),
      m_height(0.0f),
      m_depth(0.0f),
      m_mass(0.0f),
      m_groupID(0),
      m_groupMask(0),
      m_collisionGroupID(0),
      m_shapeType(0),
      m_type(0),
      m_noBone(false)
{
    internal::zerofill(m_name, sizeof(m_name));
    m_transform.setIdentity();
    m_invertedTransform.setIdentity();
}

RigidBody::~RigidBody()
{
    internal::zerofill(m_name, sizeof(m_name));
    delete m_body;
    m_body = 0;
    delete m_shape;
    m_shape = 0;
    delete m_motionState;
    m_motionState = 0;
    delete m_kinematicMotionState;
    m_kinematicMotionState = 0;
    m_position.setZero();
    m_rotation.setZero();
    m_width = 0.0f;
    m_height = 0.0f;
    m_depth = 0.0f;
    m_mass = 0.0f;
    m_groupID = 0;
    m_groupMask = 0;
    m_collisionGroupID = 0;
    m_shapeType = 0;
    m_type = 0;
    m_noBone = false;
    m_transform.setIdentity();
    m_invertedTransform.setIdentity();
}

void RigidBody::read(const uint8_t *data, BoneList *bones)
{
#ifndef VPVL_NO_BULLET
    RigidBodyChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    setName(chunk.name);
    uint16_t boneID = chunk.boneID;
    uint8_t collisionGroupID = chunk.collisionGroupID;
    uint16_t collisionMask = chunk.collsionMask;
    int8_t shapeType = chunk.shapeType;

#ifdef VPVL_BUILD_IOS
    float width, height, depth, pos[3], rot[3], mass, linearDamping, angularDamping, restitution, friction;
    memcpy(&width, &chunk.width, sizeof(width));
    memcpy(&height, &chunk.height, sizeof(height));
    memcpy(&depth, &chunk.depth, sizeof(depth));
    memcpy(pos, &chunk.position, sizeof(pos));
    memcpy(rot, &chunk.rotation, sizeof(rot));
    memcpy(&mass, &chunk.mass, sizeof(mass));
    memcpy(&linearDamping, &chunk.linearDamping, sizeof(linearDamping));
    memcpy(&angularDamping, &chunk.angularDamping, sizeof(angularDamping));
    memcpy(&restitution, &chunk.restitution, sizeof(restitution));
    memcpy(&friction, &chunk.friction, sizeof(friction));
#else
    float width = chunk.width;
    float height = chunk.height;
    float depth = chunk.depth;
    float *pos = chunk.position;
    float *rot = chunk.rotation;
    float mass = chunk.mass;
    float linearDamping = chunk.linearDamping;
    float angularDamping = chunk.angularDamping;
    float restitution = chunk.restitution;
    float friction = chunk.friction;
#endif
    uint8_t type = chunk.type;

    Bone *bone = 0;
    if (boneID == 0xffff) {
        m_noBone = true;
        m_bone = bone = Bone::centerBone(bones);
    }
    else if (boneID < bones->count()) {
        m_bone = bone = bones->at(boneID);
        if (type != 0)
            bone->setSimulated(true);
    }

    switch (shapeType) {
    case 0:
        m_shape = new btSphereShape(width);
        break;
    case 1:
        m_shape = new btBoxShape(Vector3(width, height, depth));
        break;
    case 2:
        m_shape = new btCapsuleShape(width, height);
        break;
    }

    if (m_shape) {
        Vector3 localInertia(0.0f, 0.0f, 0.0f);
        Scalar massValue = 0.0f;
        if (type != 0) {
            massValue = mass;
            if (massValue != 0.0f)
                m_shape->calculateLocalInertia(massValue, localInertia);
        }
        m_transform.setIdentity();
        btMatrix3x3 basis;
#ifdef VPVL_COORDINATE_OPENGL
        btMatrix3x3 mx, my, mz;
        mx.setEulerZYX(-rot[0], 0.0f, 0.0f);
        my.setEulerZYX(0.0f, -rot[1], 0.0f);
        mz.setEulerZYX(0.0f, 0.0f, rot[2]);
        basis = my * mz * mx;
#else  /* VPVL_COORDINATE_OPENGL */
        basis.setEulerZYX(rot[0], rot[1], rot[2]);
#endif /* VPVL_COORDINATE_OPENGL */
        m_transform.setBasis(basis);
#ifdef VPVL_COORDINATE_OPENGL
        m_transform.setOrigin(Vector3(pos[0], pos[1], -pos[2]));
#else  /* VPVL_COORDINATE_OPENGL */
        m_transform.setOrigin(Vector3(pos[0], pos[1], pos[2]));
#endif /* VPVL_COORDINATE_OPENGL */
        Transform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(bone->localTransform().getOrigin());
        startTransform *= m_transform;

        switch (type) {
        case 0:
            m_motionState = new KinematicMotionState(m_transform, bone);
            m_kinematicMotionState = 0;
            break;
        case 1:
            m_motionState = new btDefaultMotionState(startTransform);
            m_kinematicMotionState = new KinematicMotionState(m_transform, bone);
            break;
        default:
            m_motionState = new AlignedMotionState(startTransform, m_transform, bone);
            m_kinematicMotionState = new KinematicMotionState(m_transform, bone);
            break;
        }

        btRigidBody::btRigidBodyConstructionInfo info(massValue, m_motionState, m_shape, localInertia);
        info.m_linearDamping = linearDamping;
        info.m_angularDamping = angularDamping;
        info.m_restitution = restitution;
        info.m_friction = friction;
        info.m_additionalDamping = true;
        m_body = new btRigidBody(info);
        if (type == 0)
            m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        m_body->setActivationState(DISABLE_DEACTIVATION);
        m_groupID = 0x0001 << collisionGroupID;
        m_groupMask = collisionMask;
        m_type = type;
        m_invertedTransform = m_transform.inverse();
        m_position.setValue(pos[0], pos[1], pos[2]);
        m_rotation.setValue(rot[0], rot[1], rot[2]);
        m_width = width;
        m_height = height;
        m_depth = depth;
        m_mass = mass;
        m_collisionGroupID = collisionGroupID;
        m_shapeType = shapeType;
    }
#else  /* VPVL_NO_BULLET */
    (void) data;
    (void) bones;
#endif /* VPVL_NO_BULLET */
}

void RigidBody::write(uint8_t *data) const
{
    RigidBodyChunk chunk;
    internal::copyBytes(chunk.name, m_name, sizeof(chunk.name));
    chunk.boneID = m_noBone ? 0xffff : m_bone->id();
    chunk.collisionGroupID = m_collisionGroupID;
    chunk.collsionMask = m_groupMask;
    chunk.shapeType = m_shapeType;
    chunk.width = m_width;
    chunk.height = m_height;
    chunk.depth = m_depth;
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.position[2] = m_position.z();
    chunk.rotation[0] = m_rotation.x();
    chunk.rotation[1] = m_rotation.y();
    chunk.rotation[2] = m_rotation.z();
    chunk.mass = m_mass;
    chunk.linearDamping = m_body->getLinearDamping();
    chunk.angularDamping = m_body->getAngularDamping();
    chunk.restitution = m_body->getRestitution();
    chunk.friction = m_body->getFriction();
    chunk.type = m_type;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

void RigidBody::transformBone()
{
#ifndef VPVL_NO_BULLET
    if (m_type == 0 || m_noBone)
        return;
    m_bone->setLocalTransform(m_body->getCenterOfMassTransform() * m_invertedTransform);
#endif /* VPVL_NO_BULLET */
}

void RigidBody::setKinematic(bool value)
{
#ifndef VPVL_NO_BULLET
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
#else  /* VPVL_NO_BULLET */
    (void) value;
#endif /* VPVL_NO_BULLET */
}

}
