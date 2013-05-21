/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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
#include "vpvl2/pmd2/Bone.h"
#include "vpvl2/pmd2/RigidBody.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct RigidBodyUnit {
    vpvl2::uint8_t name[RigidBody::kNameSize];
    vpvl2::uint16_t boneID;
    vpvl2::uint8_t collisionGroupID;
    vpvl2::uint16_t collsionMask;
    vpvl2::uint8_t shapeType;
    vpvl2::float32_t size[3];
    vpvl2::float32_t position[3];
    vpvl2::float32_t rotation[3];
    vpvl2::float32_t mass;
    vpvl2::float32_t linearDamping;
    vpvl2::float32_t angularDamping;
    vpvl2::float32_t restitution;
    vpvl2::float32_t friction;
    vpvl2::uint8_t type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

const int RigidBody::kNameSize;

RigidBody::RigidBody(IEncoding *encodingRef)
    : internal::BaseRigidBody(),
      m_encodingRef(encodingRef)
{
}

RigidBody::~RigidBody()
{
    m_encodingRef = 0;
}

bool RigidBody::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int32_t size;
    if (!internal::getTyped<int32_t>(ptr, rest, size) || size * sizeof(RigidBodyUnit) > rest) {
        return false;
    }
    info.rigidBodiesCount = size;
    info.rigidBodiesPtr = ptr;
    internal::drainBytes(size * sizeof(RigidBodyUnit), ptr, rest);
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
                rigidBody->build(NullBone::sharedReference(), i);
            }
            else if (boneIndex >= nbones) {
                return false;
            }
            else {
                IBone *boneRef = bones[boneIndex];
                rigidBody->build(boneRef, i);
            }
        }
        else {
            rigidBody->build(NullBone::sharedReference(), i);
        }
    }
    return true;
}

void RigidBody::writeRigidBodies(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info, uint8_t *&data)
{
    const int32_t nbodies = rigidBodies.count();
    internal::writeBytes(&nbodies, sizeof(nbodies), data);
    for (int32_t i = 0; i < nbodies; i++) {
        RigidBody *body = rigidBodies[i];
        body->write(data, info);
    }
}

size_t RigidBody::estimateTotalSize(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info)
{
    const int32_t nbodies = rigidBodies.count();
    size_t size = sizeof(nbodies);
    for (int32_t i = 0; i < nbodies; i++) {
        RigidBody *rigidBody = rigidBodies[i];
        size += rigidBody->estimateSize(info);
    }
    return size;
}

void RigidBody::read(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    RigidBodyUnit unit;
    internal::getData(data, unit);
    internal::setStringDirect(m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize), m_name);
    m_boneIndex = unit.boneID;
    m_collisionGroupID = btClamped(unit.collisionGroupID, uint8_t(0), uint8_t(15));
    m_collisionGroupMask = unit.collsionMask;
    m_groupID = uint16_t(0x0001 << m_collisionGroupID);
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

void RigidBody::write(uint8_t *&data, const Model::DataInfo & /* info */) const
{
    RigidBodyUnit unit;
    unit.angularDamping = m_angularDamping;
    unit.boneID = m_boneIndex;
    unit.collisionGroupID = m_collisionGroupID;
    unit.collsionMask = m_collisionGroupMask;
    unit.friction = m_friction;
    unit.linearDamping = m_linearDamping;
    unit.mass = m_mass;
    uint8_t *namePtr = unit.name;
    internal::writeStringAsByteArray(m_name, IString::kShiftJIS, m_encodingRef, sizeof(unit.name), namePtr);
    internal::getPosition(m_position, unit.position);
    unit.restitution = m_restitution;
    internal::getPositionRaw(m_rotation, unit.rotation);
    unit.shapeType = m_shapeType;
    internal::getPositionRaw(m_size, unit.size);
    unit.type = m_type;
    internal::writeBytes(&unit, sizeof(unit), data);
}

const Transform RigidBody::createTransform() const
{
    const Transform &localTransform = BaseRigidBody::createTransform();
    return Transform(Matrix3x3::getIdentity(), m_boneRef->worldTransform().getOrigin()) * localTransform;
}

}
}
