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
namespace pmx
{

RigidBody::RigidBody()
    : common::RigidBody::RigidBody()
{
}

RigidBody::~RigidBody()
{
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
            if (boneIndex >= nbones) {
                return false;
            }
            else {
                btCollisionShape *shape = rigidBody->createShape();
                Bone *bone = bones[boneIndex];
                if (rigidBody->m_type != kStaticObject)
                    bone->setSimulated(true);
                rigidBody->m_shape = shape;
                rigidBody->m_boneRef = bone;
                rigidBody->m_body = rigidBody->createRigidBody(shape);
            }
        }
        else {
            rigidBody->m_boneRef = &kNullBone;
            rigidBody->m_body = rigidBody->createRigidBody(0);
        }
        rigidBody->m_index = i;
    }
    return true;
}

size_t RigidBody::estimateTotalSize(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info)
{
    const int nbodies = rigidBodies.count();
    size_t size = 0;
    size += sizeof(nbodies);
    for (int i = 0; i < nbodies; i++) {
        RigidBody *body = rigidBodies[i];
        size += body->estimateSize(info);
    }
    return size;
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
    m_shapeType = static_cast<ShapeType>(unit.shapeType);
    internal::setPositionRaw(unit.size, m_size);
    internal::setPosition(unit.position, m_position);
    internal::setPositionRaw(unit.rotation, m_rotation);
    m_mass = unit.mass;
    m_linearDamping = unit.linearDamping;
    m_angularDamping = unit.angularDamping;
    m_restitution = unit.restitution;
    m_friction = unit.friction;
    m_type = static_cast<ObjectType>(unit.type);
    ptr += sizeof(unit);
    size = ptr - start;
}

void RigidBody::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
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
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += info.boneIndexSize;
    size += sizeof(RigidBodyUnit);
    return size;
}

} /* namespace pmx */
} /* namespace vpvl2 */

