/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include <btBulletDynamicsCommon.h>
#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace vpvl
{

class AlignedMotionState : public btMotionState
{
public:
    AlignedMotionState(const btTransform &startTransform, const btTransform &boneTransform, Bone *bone)
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
        btMatrix3x3 matrix = worldTrans.getBasis();
        m_worldTransform.setOrigin(internal::kZeroV);
        m_worldTransform = m_boneTransform * m_worldTransform;
        m_worldTransform.setOrigin(m_worldTransform.getOrigin() + m_bone->currentTransform().getOrigin());
        m_worldTransform.setBasis(matrix);
    }
private:
    Bone *m_bone;
    btTransform m_boneTransform;
    btTransform m_inversedBoneTransform;
    btTransform m_worldTransform;
};

class KinematicMotionState : public btMotionState
{
public:
    KinematicMotionState(const btTransform &boneTransform, Bone *bone)
        : m_bone(bone),
          m_boneTransform(boneTransform)
    {
    }
    virtual ~KinematicMotionState()
    {
    }
    virtual void getWorldTransform(btTransform &worldTrans) const
    {
        worldTrans = m_bone->currentTransform() * m_boneTransform;
    }
    virtual void setWorldTransform(const btTransform &worldTrans)
    {
        (void) worldTrans;
    }
private:
    Bone *m_bone;
    btTransform m_boneTransform;
};

size_t RigidBody::stride()
{
    return kNameSize + (sizeof(int16_t) * 2) + (sizeof(int8_t) * 2)
            + (sizeof(float) * 14) + sizeof(uint8_t);
}

RigidBody::RigidBody()
    : m_bone(0),
      m_shape(0),
      m_body(0),
      m_motionState(0),
      m_kinematicMotionState(0),
      m_groupID(0),
      m_groupMask(0),
      m_type(0),
      m_noBone(false)
{
    memset(m_name, 0, sizeof(m_name));
    m_transform.setIdentity();
    m_invertedTransform.setIdentity();
}

RigidBody::~RigidBody()
{
    memset(m_name, 0, sizeof(m_name));
    delete m_body;
    m_body = 0;
    delete m_shape;
    m_shape = 0;
    delete m_motionState;
    m_motionState = 0;
    delete m_kinematicMotionState;
    m_kinematicMotionState = 0;
    m_groupID = 0;
    m_groupMask = 0;
    m_type = 0;
    m_noBone = false;
    m_transform.setIdentity();
    m_invertedTransform.setIdentity();
}

void RigidBody::read(const uint8_t *data, BoneList *bones)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    copyBytesSafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    uint16_t boneID = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(int16_t);
    int8_t collisionGroupID = *reinterpret_cast<int8_t *>(ptr);
    ptr += sizeof(int8_t);
    int16_t collisionMask = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    int8_t shapeType = *reinterpret_cast<int8_t *>(ptr);
    ptr += sizeof(int8_t);
    float width = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float height = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float depth = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float pos[3], rot[3];
    internal::vector3(ptr, pos);
    internal::vector3(ptr, rot);
    float mass = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float linearDamping = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float angularDamping = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float restitution = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    float friction = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);
    uint8_t type = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);

    Bone *bone = 0;
    if (boneID == 0xffff) {
        m_noBone = true;
        m_bone = bone = Bone::centerBone(bones);
    }
    else if (boneID < bones->size()) {
        m_bone = bone = bones->at(boneID);
    }

    switch (shapeType) {
    case 0:
        m_shape = new btSphereShape(width);
        break;
    case 1:
        m_shape = new btBoxShape(btVector3(width, height, depth));
        break;
    case 2:
        m_shape = new btCapsuleShape(width, height);
        break;
    }

    if (m_shape) {
        btVector3 localInertia(0.0f, 0.0f, 0.0f);
        btScalar massValue = 0.0f;
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
        mz.setEulerZYX(0.0f, 0.0f, -rot[2]);
        basis = my * mz * mx;
#else
        basis.setEulerZYX(rot[0], rot[1], rot[2]);
#endif
        m_transform.setBasis(basis);
#ifdef VPVL_COORDINATE_OPENGL
        m_transform.setOrigin(btVector3(pos[0], pos[1], -pos[2]));
#else
        m_transform.setOrigin(btVector3(pos[0], pos[1], pos[2]));
#endif
        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(bone->currentTransform().getOrigin());
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
    }
}

void RigidBody::transformBone()
{
    if (m_type == 0 || m_noBone)
        return;
    m_bone->setCurrentTransform(m_body->getCenterOfMassTransform() * m_invertedTransform);
}

void RigidBody::setKinematic(bool value)
{
    if (m_type == 0)
        return;
    if (value) {
        m_body->setMotionState(m_kinematicMotionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    else {
        btTransform transform;
        m_kinematicMotionState->getWorldTransform(transform);
        m_motionState->setWorldTransform(transform);
        m_body->setMotionState(m_motionState);
        m_body->setCollisionFlags(m_body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
    }
}

}
