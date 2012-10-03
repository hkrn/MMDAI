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
#include "vpvl2/pmd/Bone.h"
#include "vpvl2/pmd/RigidBody.h"

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btCollisionShape)
BT_DECLARE_HANDLE(btMotionState)
BT_DECLARE_HANDLE(btRigidBody)
#endif

namespace
{

using namespace vpvl2::pmd;

#pragma pack(push, 1)

struct RigidBodyUnit {
    uint8_t name[RigidBody::kNameSize];
    uint16_t boneID;
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

}

namespace vpvl2
{
namespace pmd
{

RigidBody::RigidBody(IEncoding *encodingRef)
    : common::RigidBody::RigidBody(),
      m_encodingRef(encodingRef)
{
}

RigidBody::~RigidBody()
{
    m_encodingRef = 0;
}

bool RigidBody::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size) || size * sizeof(RigidBodyUnit) > rest) {
        return false;
    }
    info.rigidBodiesCount = size;
    info.rigidBodiesPtr = ptr;
    internal::readBytes(size * sizeof(RigidBodyUnit), ptr, rest);
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
            if (boneIndex == 0xffff) {
                rigidBody->m_boneRef = &kNullBone;
                rigidBody->m_body = rigidBody->createRigidBody(0);
            }
            else if (boneIndex >= nbones) {
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
    for (int i = 0; i < nbodies; i++) {
        RigidBody *rigidBody = rigidBodies[i];
        size += rigidBody->estimateSize(info);
    }
    return size;
}

void RigidBody::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    RigidBodyUnit unit;
    internal::getData(data, unit);
    m_name = m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize);
    m_boneIndex = unit.boneID;
    m_collisionGroupID = unit.collisionGroupID;
    m_collisionGroupMask = unit.collsionMask;
    m_groupID = 0x0001 << unit.collsionMask;
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
    size = sizeof(unit);
}

size_t RigidBody::estimateSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(RigidBodyUnit);
    return size;
}

void RigidBody::write(uint8_t *data, const Model::DataInfo & /* info */) const
{
    RigidBodyUnit unit;
    unit.angularDamping = m_angularDamping;
    unit.boneID = m_boneIndex;
    unit.collisionGroupID = m_collisionGroupID;
    unit.collsionMask = m_collisionGroupMask;
    unit.friction = m_friction;
    unit.linearDamping = m_linearDamping;
    unit.mass = m_mass;
    uint8_t *name = m_encodingRef->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(unit.name, name, sizeof(unit.name));
    m_encodingRef->disposeByteArray(name);
    internal::getPosition(m_position, unit.position);
    unit.restitution = m_restitution;
    internal::getPositionRaw(m_rotation, unit.rotation);
    unit.shapeType = m_shapeType;
    internal::getPositionRaw(m_size, unit.size);
    unit.type = m_type;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&unit), sizeof(unit));
}

}
}
