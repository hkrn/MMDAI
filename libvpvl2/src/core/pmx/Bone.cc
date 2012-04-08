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

#define IK_DEBUG 0
#if IK_DEBUG
#include <QtCore>
#include <QDebug>
#endif

namespace
{

#pragma pack(push, 1)

struct BoneUnit {
    float vector3[3];
};

#pragma pack(pop)

using namespace vpvl2;
using namespace vpvl2::pmx;

class BoneOrderPredication
{
public:
    inline bool operator()(const Bone *left, const Bone *right) const {
        if (left->isTransformedAfterPhysicsSimulation() == right->isTransformedAfterPhysicsSimulation()) {
            if (left->layerIndex() == right->layerIndex())
                return left->index() < right->index();
            return left->layerIndex() < right->layerIndex();
        }
        return right->isTransformedAfterPhysicsSimulation();
    }
};

static void ClampAngle(const Scalar &min,
                       const Scalar &max,
                       const Scalar &current,
                       const Scalar &result,
                       Scalar &output)
{
    if (btFuzzyZero(min) && btFuzzyZero(max)) {
        output = 0;
    }
    else if (result < min) {
        output = current - min;
    }
    else if (result > max) {
        output = max - current;
    }
}

}

namespace vpvl2
{
namespace pmx
{

struct Bone::IKLink {
    Bone *bone;
    int boneID;
    bool hasAngleConstraint;
    Vector3 lowerLimit;
    Vector3 upperLimit;
};

Bone::Bone()
    : m_parentBone(0),
      m_targetBone(0),
      m_parentInherenceBone(0),
      m_name(0),
      m_englishName(0),
      m_rotation(Quaternion::getIdentity()),
      m_rotationInherence(Quaternion::getIdentity()),
      m_rotationMorph(Quaternion::getIdentity()),
      m_rotationIKLink(Quaternion::getIdentity()),
      m_localTransform(Transform::getIdentity()),
      m_localToOrigin(Transform::getIdentity()),
      m_origin(kZeroV3),
      m_offset(kZeroV3),
      m_position(kZeroV3),
      m_positionInherence(kZeroV3),
      m_positionMorph(kZeroV3),
      m_destinationOrigin(kZeroV3),
      m_fixedAxis(kZeroV3),
      m_axisX(kZeroV3),
      m_axisZ(kZeroV3),
      m_angleConstraint(0.0),
      m_weight(1.0),
      m_index(-1),
      m_parentBoneIndex(-1),
      m_layerIndex(0),
      m_destinationOriginBoneIndex(-1),
      m_targetBoneIndex(-1),
      m_nloop(0),
      m_parentInherenceBoneIndex(-1),
      m_globalID(0),
      m_flags(0),
      m_simulated(false)
{
}

Bone::~Bone()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_parentBone = 0;
    m_targetBone = 0;
    m_parentInherenceBone = 0;
    m_origin.setZero();
    m_offset.setZero();
    m_position.setZero();
    m_positionMorph.setZero();
    m_localTransform.setIdentity();
    m_localToOrigin.setIdentity();
    m_destinationOrigin.setZero();
    m_fixedAxis.setZero();
    m_axisX.setZero();
    m_axisZ.setZero();
    m_weight = 0;
    m_parentBoneIndex = -1;
    m_layerIndex = 0;
    m_destinationOriginBoneIndex = -1;
    m_parentInherenceBoneIndex = -1;
    m_globalID = 0;
    m_flags = 0;
}

bool Bone::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size, boneIndexSize = info.boneIndexSize;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.bonesPtr = ptr;
    /* BoneUnit + boneIndexSize + hierarcy + flags */
    size_t baseSize = sizeof(BoneUnit) + boneIndexSize + sizeof(int) + sizeof(uint16_t);
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
        if (!internal::validateSize(ptr, baseSize, rest)) {
            return false;
        }
        uint16_t flags = *reinterpret_cast<uint16_t *>(ptr - 2);
        /* bone has destination relative or absolute */
        bool isRelative = ((flags & 0x0001) == 1);
        if (isRelative) {
            if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                return false;
            }
        }
        else {
            if (!internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
                return false;
            }
        }
        /* bone is IK */
        if (flags & 0x0020) {
            /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
            size_t extraSize = boneIndexSize + sizeof(int) + sizeof(float) + sizeof(int);
            if (!internal::validateSize(ptr, extraSize, rest)) {
                return false;
            }
            int nlinks = *reinterpret_cast<int *>(ptr - sizeof(int));
            for (int i = 0; i < nlinks; i++) {
                if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                    return false;
                }
                size_t hasAngleConstraint;
                if (!internal::size8(ptr, rest, hasAngleConstraint)) {
                    return false;
                }
                if (hasAngleConstraint == 1 && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
                    return false;
                }
            }
        }
        /* bone has additional bias */
        if ((flags & 0x0100 || flags & 0x200) && !internal::validateSize(ptr, boneIndexSize + sizeof(float), rest)) {
            return false;
        }
        /* axis of bone is fixed */
        if ((flags & 0x0400) && !internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
            return false;
        }
        /* axis of bone is local */
        if ((flags & 0x0800) && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
            return false;
        }
        /* bone is transformed after external parent bone transformation */
        if ((flags & 0x2000) && !internal::validateSize(ptr, sizeof(int), rest)) {
            return false;
        }
    }
    info.bonesCount = size;
    return true;
}

bool Bone::loadBones(const Array<Bone *> &bones, Array<Bone *> &bpsBones, Array<Bone *> &apsBones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        const int parentBoneID = bone->m_parentBoneIndex;
        if (parentBoneID >= 0) {
            if (parentBoneID >= nbones) {
                return false;
            }
            else {
                Bone *parent = bones[parentBoneID];
                bone->m_offset -= parent->m_origin;
                bone->m_parentBone = parent;
            }
        }
        const int destinationOriginBoneID = bone->m_destinationOriginBoneIndex;
        if (destinationOriginBoneID >= 0) {
            if (destinationOriginBoneID >= nbones)
                return false;
            else
                bone->m_destinationOriginBone = bones[destinationOriginBoneID];
        }
        const int targetBoneID = bone->m_targetBoneIndex;
        if (targetBoneID >= 0) {
            if (targetBoneID >= nbones)
                return false;
            else
                bone->m_targetBone = bones[targetBoneID];
        }
        const int parentBoneBiasID = bone->m_parentInherenceBoneIndex;
        if (parentBoneBiasID >= 0) {
            if (parentBoneBiasID >= nbones)
                return false;
            else
                bone->m_parentInherenceBone = bones[parentBoneBiasID];
        }
        if (bone->isIKEnabled()) {
            const int nIK = bone->m_IKLinks.count();
            for (int j = 0; j < nIK; j++) {
                IKLink *ik = bone->m_IKLinks[j];
                const int ikTargetBoneID = ik->boneID;
                if (ikTargetBoneID >= 0) {
                    if (ikTargetBoneID >= nbones)
                        return false;
                    else
                        ik->bone = bones[ikTargetBoneID];
                }
            }
        }
        bone->m_index = i;
    }
    Array<Bone *> ordered;
    ordered.copy(bones);
    ordered.sort(BoneOrderPredication());
    for (int i = 0; i < nbones; i++) {
        Bone *bone = ordered[i];
        if (bone->isTransformedAfterPhysicsSimulation())
            apsBones.add(bone);
        else
            bpsBones.add(bone);
    }
    return true;
}

void Bone::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX, boneIndexSize = info.boneIndexSize;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    setName(info.encoding->toString(namePtr, nNameSize, info.codec));
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    setEnglishName(info.encoding->toString(namePtr, nNameSize, info.codec));
    const BoneUnit &unit = *reinterpret_cast<const BoneUnit *>(ptr);
    internal::setPosition(unit.vector3, m_origin);
    m_offset = m_origin;
    m_localTransform.setOrigin(m_origin);
    m_localToOrigin.setOrigin(-m_origin);
    ptr += sizeof(unit);
    m_parentBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
    m_layerIndex = *reinterpret_cast<int *>(ptr);
    ptr += sizeof(m_layerIndex);
    uint16_t flags = m_flags = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(m_flags);
    /* bone has destination */
    bool isRelative = ((flags & 0x0001) == 1);
    if (isRelative) {
        m_destinationOriginBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
    }
    else {
        const BoneUnit &offset = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(offset.vector3, m_destinationOrigin);
        ptr += sizeof(offset);
    }
    /* bone is IK */
    if (flags & 0x0020) {
        /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
        m_targetBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        m_nloop = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(m_nloop);
        m_angleConstraint = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(m_angleConstraint);
        int nlinks = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(nlinks);
        for (int i = 0; i < nlinks; i++) {
            IKLink *ik = new IKLink();
            ik->boneID = internal::readSignedIndex(ptr, boneIndexSize);
            ik->hasAngleConstraint = *reinterpret_cast<uint8_t *>(ptr) == 1;
            ptr += sizeof(ik->hasAngleConstraint);
            if (ik->hasAngleConstraint) {
                const BoneUnit &lower = *reinterpret_cast<const BoneUnit *>(ptr);
                ptr += sizeof(lower);
                const BoneUnit &upper = *reinterpret_cast<const BoneUnit *>(ptr);
                ptr += sizeof(upper);
#ifdef VPVL2_COORDINATE_OPENGL
                ik->lowerLimit.setValue(-upper.vector3[0], -upper.vector3[1], lower.vector3[2]);
                ik->upperLimit.setValue(-lower.vector3[0], -lower.vector3[1], upper.vector3[2]);
#else
                ik->lowerLimit.setValue(lower.vector3[0], lower.vector3[1], lower.vector3[2]);
                ik->upperLimit.setValue(upper.vector3[0], upper.vector3[1], upper.vector3[2]);
#endif
            }
            m_IKLinks.add(ik);
        }
    }
    /* bone has additional bias */
    if ((flags & 0x0100 || flags & 0x200)) {
        m_parentInherenceBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        m_weight = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(m_weight);
    }
    /* axis of bone is fixed */
    if (flags & 0x0400) {
        const BoneUnit &axis = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(axis.vector3, m_fixedAxis);
        ptr += sizeof(axis);
    }
    /* axis of bone is local */
    if (flags & 0x0800) {
        const BoneUnit &axisX = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(axisX.vector3, m_axisX);
        ptr += sizeof(axisX);
        const BoneUnit &axisZ = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(axisZ.vector3, m_axisZ);
        ptr += sizeof(axisZ);
    }
    /* bone is transformed after external parent bone transformation */
    if (flags & 0x2000) {
        m_globalID = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(m_globalID);
    }
    size = ptr - start;
}

void Bone::write(uint8_t *data, const Model::DataInfo &info) const
{
    size_t boneIndexSize = info.boneIndexSize;
    BoneUnit bu;
    internal::writeString(m_name, data);
    internal::writeString(m_englishName, data);
    internal::getPosition(m_origin, &bu.vector3[0]);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    internal::writeSignedIndex(m_parentBoneIndex, boneIndexSize, data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_layerIndex), sizeof(m_layerIndex), data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_flags), sizeof(m_flags), data);
    if (m_flags & 0x0001) {
        internal::writeSignedIndex(m_destinationOriginBoneIndex, boneIndexSize, data);
    }
    else {
        internal::getPosition(m_destinationOrigin, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    }
    if (m_flags & 0x0020) {
        internal::writeSignedIndex(m_targetBoneIndex, boneIndexSize, data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_nloop), sizeof(m_nloop), data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_angleConstraint), sizeof(m_angleConstraint), data);
        int nlinks = m_IKLinks.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&nlinks), sizeof(nlinks), data);
        for (int i = 0; i < nlinks; i++) {
            IKLink *link = m_IKLinks[0];
            internal::writeSignedIndex(link->boneID, boneIndexSize, data);
            uint8_t hasAngleConstraint = link->hasAngleConstraint ? 1 : 0;
            internal::writeBytes(reinterpret_cast<const uint8_t *>(hasAngleConstraint), sizeof(hasAngleConstraint), data);
            if (hasAngleConstraint) {
                internal::getPosition(link->lowerLimit, &bu.vector3[0]);
                internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
                internal::getPosition(link->upperLimit, &bu.vector3[0]);
                internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
            }
        }
    }
    if (m_flags & 0x0100 || m_flags & 0x0200) {
        internal::writeSignedIndex(m_parentInherenceBoneIndex, boneIndexSize, data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_weight), sizeof(m_weight), data);
    }
    if (m_flags & 0x0400) {
        internal::getPosition(m_fixedAxis, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    }
    if (m_flags & 0x0800) {
        internal::getPosition(m_axisX, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
        internal::getPosition(m_axisZ, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    }
    if (m_flags & 0x2000) {
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_globalID), sizeof(m_globalID), data);
    }
}

size_t Bone::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0, boneIndexSize = info.boneIndexSize;
    size += internal::estimateSize(m_name);
    size += internal::estimateSize(m_englishName);
    size += sizeof(BoneUnit);
    size += boneIndexSize;
    size += sizeof(m_layerIndex);
    size += sizeof(m_flags);
    size += (m_flags & 0x0001) ? boneIndexSize : sizeof(BoneUnit);
    if (m_flags & 0x0020) {
        size += boneIndexSize;
        size += sizeof(m_angleConstraint);
        size += sizeof(m_nloop);
        int nlinks = m_IKLinks.count();
        size += sizeof(nlinks);
        for (int i = 0; i < nlinks; i++) {
            size += boneIndexSize;
            size += sizeof(uint8_t);
            if (m_IKLinks[i]->hasAngleConstraint)
                size += sizeof(BoneUnit) * 2;
        }
    }
    if (m_flags & 0x0100 || m_flags & 0x0200) {
        size += boneIndexSize;
        size += sizeof(m_weight);
    }
    if (m_flags & 0x0400) {
        size += sizeof(BoneUnit);
    }
    if (m_flags & 0x0800) {
        size += sizeof(BoneUnit) * 2;
    }
    if (m_flags & 0x2000) {
        size += sizeof(m_globalID);
    }
    return size;
}

void Bone::mergeMorph(Morph::Bone *morph, float weight)
{
    m_positionMorph = morph->position * weight;
    m_rotationMorph = morph->rotation * weight;
}

void Bone::performFullTransform()
{
    Quaternion rotation = Quaternion::getIdentity();
    if (hasRotationInherence()) {
        if (Bone *parentBone = m_parentInherenceBone) {
            const Quaternion &parentRotation = parentBone->hasRotationInherence() ? parentBone->m_rotationInherence : parentBone->m_rotation * parentBone->m_rotationMorph;
            rotation *= parentRotation;
            if (!btFuzzyZero(m_weight - 1.0))
                rotation = Quaternion::getIdentity().slerp(rotation, m_weight);
            rotation *= parentBone->m_rotationIKLink;
            m_rotationInherence = Quaternion::getIdentity().slerp(parentRotation * parentBone->m_rotationIKLink, m_weight) * m_rotation * m_rotationMorph;
            m_rotationInherence.normalize();
        }
    }
    rotation *= m_rotation * m_rotationMorph * m_rotationIKLink;
    rotation.normalize();
    m_localTransform.setRotation(rotation);
    Vector3 position = kZeroV3;
    if (hasPositionInherence()) {
        if (Bone *parentBone = m_parentInherenceBone) {
            const Vector3 &parentPosition = parentBone->hasPositionInherence() ? parentBone->m_positionInherence : parentBone->m_position + parentBone->m_positionMorph;
            position += parentPosition;
            if (!btFuzzyZero(m_weight - 1.0))
                position *= m_weight;
            m_positionInherence = parentPosition + m_position + m_positionMorph;
        }
    }
    position += m_position + m_positionMorph;
    m_localTransform.setOrigin(m_offset + m_position + m_positionMorph);
    if (m_parentBone) {
        m_localTransform = m_parentBone->m_localTransform * m_localTransform;
    }
    //const Quaternion &value = m_localTransform.getRotation();
    //qDebug("%s(fullTransform): %.f,%.f,%.f,.%f", m_name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::performTransform()
{
    m_localTransform.setRotation(m_rotation);
    m_localTransform.setOrigin(m_offset + m_position);
    if (m_parentBone)
        m_localTransform = m_parentBone->m_localTransform * m_localTransform;
    //const Quaternion &value = m_localTransform.getRotation();
    //qDebug("%s(transform): %.f,%.f,%.f,.%f", m_name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::performInverseKinematics()
{
    if (!isIKEnabled() || m_simulated)
        return;
    const int nlinks = m_IKLinks.count();
    const int nloops = m_nloop;
    Quaternion rotation, targetRotation = m_targetBone->m_rotation;
    btMatrix3x3 matrix;
    for (int i = 0; i < nloops; i++) {
        for (int j = 0; j < nlinks; j++) {
            IKLink *link = m_IKLinks[j];
            Bone *bone = link->bone;
            const Vector3 &targetPosition = m_targetBone->m_localTransform.getOrigin();
            const Vector3 &destinationPosition = m_localTransform.getOrigin();
            const Transform &transform = bone->m_localTransform.inverse();
            Vector3 v1 = transform * targetPosition;
            Vector3 v2 = transform * destinationPosition;
            if (btFuzzyZero(v1.distance2(v2))) {
                i = nloops;
                break;
            }
#if IK_DEBUG
            qDebug() << "transo:" << transform.getOrigin().x() << transform.getOrigin().y() << transform.getOrigin().z();
            qDebug() << "transr:" << transform.getRotation().w() << transform.getRotation().x() << transform.getRotation().y() << transform.getRotation().z();
            qDebug() << "trot:  " << m_targetBone->m_localTransform.getRotation().w()
                     << m_targetBone->m_localTransform.getRotation().x()
                     << m_targetBone->m_localTransform.getRotation().y()
                     << m_targetBone->m_localTransform.getRotation().z();
            qDebug() << "target:" << targetPosition.x() << targetPosition.y() << targetPosition.z();
            qDebug() << "dest:  " << destinationPosition.x() << destinationPosition.y() << destinationPosition.z();
#endif
            v1.safeNormalize();
            v2.safeNormalize();
#if IK_DEBUG
            qDebug() << "v1:    " << v1.x() << v1.y() << v1.z();
            qDebug() << "v2:    " << v2.x() << v2.y() << v2.z();
#endif
            Vector3 rotationAxis = v1.cross(v2);
            const Scalar &angle = btClamped(btAcos(v1.dot(v2)), -m_angleConstraint, m_angleConstraint);
            if (btFuzzyZero(angle))
                continue;
            rotation.setRotation(rotationAxis, angle);
            rotation.normalize();
#if IK_DEBUG
            qDebug("I:J=%d:%d", i, j);
            qDebug("target: %s", m_targetBone->name()->toByteArray());
            qDebug("linked: %s", bone->name()->toByteArray());
#endif
            if (link->hasAngleConstraint) {
                const Vector3 &lowerLimit = link->lowerLimit;
                const Vector3 &upperLimit = link->upperLimit;
                if (i == 0) {
                    if (btFuzzyZero(lowerLimit.y()) && btFuzzyZero(upperLimit.y())
                            && btFuzzyZero(lowerLimit.z()) && btFuzzyZero(upperLimit.z())) {
                        rotationAxis.setValue(1.0, 0.0, 0.0);
                    }
                    else if (btFuzzyZero(lowerLimit.x()) && btFuzzyZero(upperLimit.x())
                             && btFuzzyZero(lowerLimit.z()) && btFuzzyZero(upperLimit.z())) {
                        rotationAxis.setValue(0.0, 1.0, 0.0);
                    }
                    else if (btFuzzyZero(lowerLimit.x()) && btFuzzyZero(upperLimit.x())
                             && btFuzzyZero(lowerLimit.y()) && btFuzzyZero(upperLimit.y())) {
                        rotationAxis.setValue(0.0, 0.0, 1.0);
                    }
                    rotation.setRotation(rotationAxis, btFabs(angle));
                }
                else {
                    Scalar x1, y1, z1, x2, y2, z2, x3, y3, z3;
                    matrix.setRotation(rotation);
                    matrix.getEulerZYX(z1, y1, x1);
                    matrix.setRotation(bone->m_rotation);
                    matrix.getEulerZYX(z2, y2, x2);
                    x3 = x1 + x2; y3 = y1 + y2; z3 = z1 + z2;
                    ClampAngle(lowerLimit.x(), upperLimit.x(), x2, x3, x1);
                    ClampAngle(lowerLimit.y(), upperLimit.y(), y2, y3, y1);
                    ClampAngle(lowerLimit.z(), upperLimit.z(), z2, z3, z1);
#if IK_DEBUG
                    if (x3 < lowerLimit.x() || x3 > upperLimit.x()) {
                        qDebug() << x3 << x1 << x2;
                    }
                    /*
                    qDebug() << "I:J" << i << j;
                    qDebug() << "IK:   " << x1 << y1 << z1;
                    qDebug() << "upper:" << upperLimit.x() << upperLimit.y() << upperLimit.z();
                    qDebug() << "lower:" << lowerLimit.x() << lowerLimit.y() << lowerLimit.z();
                    qDebug() << "rot:  " << x2 << y2 << z2;
                    */
#endif
                    rotation.setEulerZYX(z1, y1, x1);
                }
#if IK_DEBUG
                qDebug() << "rot:   " << rotation.w() << rotation.x() << rotation.y() << rotation.z();
#endif
                bone->m_rotation = rotation * bone->m_rotation;
            }
            else {
#if IK_DEBUG
                qDebug() << "rot:   " << rotation.w() << rotation.x() << rotation.y() << rotation.z();
#endif
                bone->m_rotation *= rotation;
            }
            bone->m_rotation.normalize();
            bone->m_rotationIKLink = rotation;
            for (int k = j; k >= 0; k--) {
                IKLink *ik = m_IKLinks[k];
                Bone *destinationBone = ik->bone;
                destinationBone->performTransform();
#if IK_DEBUG
                qDebug("k=%d: %s", k, destinationBone->name()->toByteArray());
                qDebug() << destinationBone->m_localTransform.getOrigin().x()
                         << destinationBone->m_localTransform.getOrigin().y()
                         << destinationBone->m_localTransform.getOrigin().z();
                qDebug() << destinationBone->m_localTransform.getRotation().w()
                         << destinationBone->m_localTransform.getRotation().x()
                         << destinationBone->m_localTransform.getRotation().y()
                         << destinationBone->m_localTransform.getRotation().z();
#endif
            }
#if IK_DEBUG
            qDebug("target1: %s", m_targetBone->name()->toByteArray());
            qDebug() << m_targetBone->m_localTransform.getOrigin().x()
                     << m_targetBone->m_localTransform.getOrigin().y()
                     << m_targetBone->m_localTransform.getOrigin().z();
            qDebug() << m_targetBone->m_localTransform.getRotation().w()
                     << m_targetBone->m_localTransform.getRotation().x()
                     << m_targetBone->m_localTransform.getRotation().y()
                     << m_targetBone->m_localTransform.getRotation().z();
#endif
            m_targetBone->performTransform();
#if IK_DEBUG
            qDebug("target2: %s", m_targetBone->name()->toByteArray());
            qDebug() << m_targetBone->m_localTransform.getOrigin().x()
                     << m_targetBone->m_localTransform.getOrigin().y()
                     << m_targetBone->m_localTransform.getOrigin().z();
            qDebug() << m_targetBone->m_localTransform.getRotation().w()
                     << m_targetBone->m_localTransform.getRotation().x()
                     << m_targetBone->m_localTransform.getRotation().y()
                     << m_targetBone->m_localTransform.getRotation().z();
#endif
        }
    }
    m_targetBone->m_rotation = targetRotation;
}

void Bone::performUpdateLocalTransform()
{
    m_localTransform *= m_localToOrigin;
}

void Bone::resetIKLink()
{
    m_rotationIKLink = Quaternion::getIdentity();
}

const Transform &Bone::localTransform() const
{
    return m_localTransform;
}

void Bone::setPosition(const Vector3 &value)
{
    m_position = value;
}

void Bone::setRotation(const Quaternion &value)
{
    m_rotation = value;
    //qDebug("%s(rotate): %.f,%.f,%.f,.%f", m_name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::setLocalTransform(const Transform &value)
{
    m_localTransform = value;
}

void Bone::setSimulated(bool value)
{
    m_simulated = value;
}

void Bone::setParentBone(Bone *value)
{
    m_parentBone = value;
    m_parentBoneIndex = value ? value->index() : -1;
}

void Bone::setParentInherenceBone(Bone *value, float weight)
{
    m_parentInherenceBone = value;
    m_parentInherenceBoneIndex = value ? value->index() : -1;
    m_weight = weight;
}

void Bone::setTargetBone(Bone *target, int nloop, float angleConstraint)
{
    m_targetBone = target;
    m_targetBoneIndex = target ? target->index() : -1;
    m_nloop = nloop;
    m_angleConstraint = angleConstraint;
}

void Bone::setDestinationOriginBone(Bone *value)
{
    m_destinationOriginBone = value;
    m_destinationOriginBoneIndex = value ? value->index() : -1;
    internal::toggleFlag(0x0001, value ? true : false, m_flags);
}

void Bone::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Bone::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void Bone::setOrigin(const Vector3 &value)
{
    m_origin = value;
}

void Bone::setDestinationOrigin(const Vector3 &value)
{
    m_offset = value;
    internal::toggleFlag(0x0001, false, m_flags);
}

void Bone::setFixedAxis(const Vector3 &value)
{
    m_fixedAxis = value;
}

void Bone::setAxisX(const Vector3 &value)
{
    m_axisX = value;
}

void Bone::setAxisZ(const Vector3 &value)
{
    m_axisZ = value;
}

void Bone::setIndex(int value)
{
    m_index = value;
}

void Bone::setLayerIndex(int value)
{
    m_layerIndex = value;
}

void Bone::setExternalIndex(int value)
{
    m_globalID = value;
}

void Bone::setRotateable(bool value)
{
    internal::toggleFlag(0x0002, value, m_flags);
}

void Bone::setMovable(bool value)
{
    internal::toggleFlag(0x0004, value, m_flags);
}

void Bone::setVisible(bool value)
{
    internal::toggleFlag(0x0008, value, m_flags);
}

void Bone::setOperatable(bool value)
{
    internal::toggleFlag(0x0010, value, m_flags);
}

void Bone::setIKEnable(bool value)
{
    internal::toggleFlag(0x0020, value, m_flags);
}

void Bone::setPositionInherenceEnable(bool value)
{
    internal::toggleFlag(0x0100, value, m_flags);
}

void Bone::setRotationInherenceEnable(bool value)
{
    internal::toggleFlag(0x0200, value, m_flags);
}

void Bone::setAxisFixedEnable(bool value)
{
    internal::toggleFlag(0x0400, value, m_flags);
}

void Bone::setLocalAxisEnable(bool value)
{
    internal::toggleFlag(0x0800, value, m_flags);
}

void Bone::setTransformedAfterPhysicsSimulationEnable(bool value)
{
    internal::toggleFlag(0x1000, value, m_flags);
}

void Bone::setTransformedByExternalParentEnable(bool value)
{
    internal::toggleFlag(0x2000, value, m_flags);
}

} /* namespace pmx */
} /* namespace vpvl2 */
