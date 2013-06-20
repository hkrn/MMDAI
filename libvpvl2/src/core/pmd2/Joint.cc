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
#include "vpvl2/internal/ModelHelper.h"
#include "vpvl2/pmd2/Joint.h"
#include "vpvl2/pmd2/RigidBody.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct JointUnit {
    uint8 name[internal::kPMDJointNameSize];
    int32 bodyIDA;
    int32 bodyIDB;
    float32 position[3];
    float32 rotation[3];
    float32 positionLowerLimit[3];
    float32 positionUpperLimit[3];
    float32 rotationLowerLimit[3];
    float32 rotationUpperLimit[3];
    float32 positionStiffness[3];
    float32 rotationStiffness[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

const int Joint::kNameSize = internal::kPMDJointNameSize;

Joint::Joint(Model *modelRef, IEncoding *encodingRef)
    : internal::BaseJoint(modelRef),
      m_encodingRef(encodingRef)
{
}

Joint::~Joint()
{
    m_encodingRef = 0;
}

bool Joint::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    int32 size;
    if (!internal::getTyped<int32>(ptr, rest, size) || size * sizeof(JointUnit) > rest) {
        return false;
    }
    info.jointsCount = size;
    info.jointsPtr = ptr;
    internal::drainBytes(size * sizeof(JointUnit), ptr, rest);
    return true;
}

bool Joint::loadJoints(const Array<Joint *> &joints, const Array<RigidBody *> &rigidBodies)
{
    const int njoints = joints.count();
    const int nRigidBodies = rigidBodies.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        const int rigidBodyIndex1 = joint->m_rigidBodyIndex1;
        if (rigidBodyIndex1 >= 0) {
            if (rigidBodyIndex1 >= nRigidBodies) {
                return false;
            }
            else {
                joint->m_rigidBody1Ref = rigidBodies[rigidBodyIndex1];
            }
        }
        const int rigidBodyIndex2 = joint->m_rigidBodyIndex2;
        if (rigidBodyIndex2 >= 0) {
            if (rigidBodyIndex2 >= nRigidBodies) {
                return false;
            }
            else {
                joint->m_rigidBody2Ref = rigidBodies[rigidBodyIndex2];
            }
        }
        joint->build(i);
    }
    return true;
}

void Joint::writeJoints(const Array<Joint *> &joints, const Model::DataInfo &info, uint8 *&data)
{
    const int32 njoints = joints.count();
    internal::writeBytes(&njoints, sizeof(njoints), data);
    for (int32 i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        joint->write(data, info);
    }
}

vsize Joint::estimateTotalSize(const Array<Joint *> &joints, const Model::DataInfo &info)
{
    const int32 njoints = joints.count();
    vsize size = sizeof(njoints);
    for (int32 i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        size += joint->estimateSize(info);
    }
    return size;
}

void Joint::read(const uint8 *data, const Model::DataInfo & /* info */, vsize &size)
{
    JointUnit unit;
    internal::getData(data, unit);
    internal::setStringDirect(m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize), m_name);
    m_rigidBodyIndex1 = unit.bodyIDA;
    m_rigidBodyIndex2 = unit.bodyIDB;
    internal::setPositionRaw(unit.position, m_position);
    internal::setPositionRaw(unit.rotation, m_rotation);
    internal::setPositionRaw(unit.positionLowerLimit, m_positionLowerLimit);
    internal::setPositionRaw(unit.rotationLowerLimit, m_rotationLowerLimit);
    internal::setPositionRaw(unit.positionUpperLimit, m_positionUpperLimit);
    internal::setPositionRaw(unit.rotationUpperLimit, m_rotationUpperLimit);
    internal::setPositionRaw(unit.positionStiffness, m_positionStiffness);
    internal::setPositionRaw(unit.rotationStiffness, m_rotationStiffness);
    size = sizeof(unit);
}

vsize Joint::estimateSize(const Model::DataInfo & /* info */) const
{
    vsize size = 0;
    size += sizeof(JointUnit);
    return size;
}

void Joint::write(uint8 *&data, const Model::DataInfo & /* info */) const
{
    JointUnit unit;
    unit.bodyIDA = m_rigidBodyIndex1;
    unit.bodyIDB = m_rigidBodyIndex2;
    uint8 *namePtr = unit.name;
    internal::writeStringAsByteArray(m_name, IString::kShiftJIS, m_encodingRef, sizeof(unit.name), namePtr);
    internal::getPositionRaw(m_position, unit.position);
    internal::getPositionRaw(m_rotation, unit.rotation);
    internal::getPositionRaw(m_positionLowerLimit, unit.positionLowerLimit);
    internal::getPositionRaw(m_rotationLowerLimit, unit.rotationLowerLimit);
    internal::getPositionRaw(m_positionUpperLimit, unit.positionUpperLimit);
    internal::getPositionRaw(m_rotationUpperLimit, unit.rotationUpperLimit);
    internal::getPositionRaw(m_positionStiffness, unit.positionStiffness);
    internal::getPositionRaw(m_rotationStiffness, unit.rotationStiffness);
    internal::writeBytes(&unit, sizeof(unit), data);
}

} /* namespace pmd2 */
} /* namespace vpvl2 */
