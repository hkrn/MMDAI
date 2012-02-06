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

}

namespace vpvl2
{
namespace pmx
{

RigidBody::RigidBody()
    : m_bone(0),
      m_name(0),
      m_englishName(0),
      m_boneIndex(0),
      m_size(kZeroV),
      m_position(kZeroV),
      m_rotation(kZeroV),
      m_mass(0),
      m_groupID(0),
      m_groupMask(0),
      m_collisionGroupID(0),
      m_shapeType(0),
      m_type(0)
{
}

RigidBody::~RigidBody()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_bone = 0;
    m_boneIndex = 0;
    m_size.setZero();
    m_position.setZero();
    m_rotation.setZero();
    m_mass = 0;
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
            if (boneIndex >= nbones)
                return false;
            else
                rigidBody->m_bone = bones[boneIndex];
        }
    }
    return true;
}

void RigidBody::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_name = new StaticString(namePtr, nNameSize);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = new StaticString(namePtr, nNameSize);
    m_boneIndex = internal::variantIndex(ptr, info.boneIndexSize);
    const RigidBodyUnit &unit = *reinterpret_cast<RigidBodyUnit *>(ptr);
    m_collisionGroupID = unit.collisionGroupID;
    m_groupMask = unit.collsionMask;
    m_shapeType = unit.shapeType;
    m_size.setValue(unit.size[0], unit.size[1], unit.size[2]);
    internal::setPosition(unit.position, m_position);
    m_rotation.setValue(unit.rotation[0], unit.rotation[1], unit.rotation[2]);
    m_mass = unit.mass;
    m_type = unit.type;
    ptr += sizeof(unit);
    size = ptr - start;
}

void RigidBody::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */

