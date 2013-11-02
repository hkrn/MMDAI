/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/RigidBody.h"

namespace
{

using namespace vpvl2;

#pragma pack(push, 1)

struct RigidBodyUnit
{
    uint8 collisionGroupID;
    uint16 collsionMask;
    uint8 shapeType;
    float32 size[3];
    float32 position[3];
    float32 rotation[3];
    float32 mass;
    float32 linearDamping;
    float32 angularDamping;
    float32 restitution;
    float32 friction;
    uint8 type;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

RigidBody::RigidBody(IModel *modelRef, IEncoding *encodingRef)
    : internal::BaseRigidBody(modelRef, encodingRef)
{
}

RigidBody::~RigidBody()
{
}

bool RigidBody::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    int32 nbodies, size, boneIndexSize = int32(info.boneIndexSize);
    if (!internal::getTyped<int32>(ptr, rest, nbodies)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX rigid bodies detected: size=" << nbodies << " rest=" << rest);
        return false;
    }
    info.rigidBodiesPtr = ptr;
    for (int32 i = 0; i < nbodies; i++) {
        uint8 *namePtr;
        /* name in Japanese */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX rigid body name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX rigid body name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::validateSize(ptr, boneIndexSize + sizeof(RigidBodyUnit), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX base rigid body unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
    }
    info.rigidBodiesCount = nbodies;
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
                VPVL2_LOG(WARNING, "Invalid PMX bone specified: index=" << i << " bone=" << boneIndex);
                return false;
            }
            else {
                Bone *boneRef = bones[boneIndex];
                rigidBody->build(boneRef, i);
            }
        }
        else {
            const IModel *parentModelRef = rigidBody->parentModelRef();
            const IEncoding *encodingRef = rigidBody->m_encodingRef;
            IBone *boneRef = parentModelRef->findBoneRef(encodingRef->stringConstant(IEncoding::kCenter));
            rigidBody->build(boneRef, i);
        }
    }
    return true;
}

void RigidBody::writeRigidBodies(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info, uint8 *&data)
{
    const int32 nbodies = rigidBodies.count();
    internal::writeBytes(&nbodies, sizeof(nbodies), data);
    for (int32 i = 0; i < nbodies; i++) {
        const RigidBody *body = rigidBodies[i];
        body->write(data, info);
    }
}

vsize RigidBody::estimateTotalSize(const Array<RigidBody *> &rigidBodies, const Model::DataInfo &info)
{
    const int32 nbodies = rigidBodies.count();
    vsize size = 0;
    size += sizeof(nbodies);
    for (int32 i = 0; i < nbodies; i++) {
        RigidBody *body = rigidBodies[i];
        size += body->estimateSize(info);
    }
    return size;
}

void RigidBody::read(const uint8 *data, const Model::DataInfo &info, vsize &size)
{
    uint8 *namePtr, *ptr = const_cast<uint8 *>(data), *start = ptr;
    vsize rest = SIZE_MAX;
    int32 nNameSize;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    VPVL2_VLOG(3, "PMXRigidBody: name=" << internal::cstr(m_name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    VPVL2_VLOG(3, "PMXRigidBody: englishName=" << internal::cstr(m_englishName, "(null)"));
    m_boneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
    RigidBodyUnit unit;
    internal::getData(ptr, unit);
    m_collisionGroupID = btClamped(unit.collisionGroupID, uint8(0), uint8(15));
    m_groupID = uint16(0x0001 << m_collisionGroupID);
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
    VPVL2_VLOG(3, "PMXRigidBody: boneIndex=" << m_boneIndex << " type=" << m_type << " shapeType=" << m_shapeType << " mass=" << m_mass);
    VPVL2_VLOG(3, "PMXRigidBody: collisionGroupID=" << m_collisionGroupID << " groupID=" << m_groupID << " collisionGroupMask=" << m_collisionGroupMask);
    VPVL2_VLOG(3, "PMXRigidBody: linearDamping=" << m_linearDamping << " angularDamping=" << m_angularDamping << " restitution=" << m_restitution << " friction=" << m_friction);
    ptr += sizeof(unit);
    size = ptr - start;
}

void RigidBody::write(uint8 *&data, const Model::DataInfo &info) const
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
    internal::writeBytes(&rbu, sizeof(rbu), data);
}

vsize RigidBody::estimateSize(const Model::DataInfo &info) const
{
    vsize size = 0;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += info.boneIndexSize;
    size += sizeof(RigidBodyUnit);
    return size;
}

void RigidBody::reset()
{
}

void RigidBody::mergeMorph(const Morph::Impulse * /* morph */, const IMorph::WeightPrecision & /* weight */)
{
}

} /* namespace pmx */
} /* namespace vpvl2 */
