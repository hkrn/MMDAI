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

#include "vpvl2/pmx/Bone.h"

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
struct IKUnit {
    int nloop;
    float angleConstraint;
    int neffectors;
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

struct Bone::IKEffector {
    Bone *bone;
    int boneID;
    bool hasAngleConstraint;
    Vector3 lowerLimit;
    Vector3 upperLimit;
};

Bone::Bone(IModel *modelRef)
    : m_modelRef(modelRef),
      m_parentBoneRef(0),
      m_targetBoneRef(0),
      m_parentInherenceBoneRef(0),
      m_destinationOriginBoneRef(0),
      m_name(0),
      m_englishName(0),
      m_rotation(Quaternion::getIdentity()),
      m_rotationInherence(Quaternion::getIdentity()),
      m_rotationMorph(Quaternion::getIdentity()),
      m_rotationIKLink(Quaternion::getIdentity()),
      m_worldTransform(Transform::getIdentity()),
      m_localTransform(Transform::getIdentity()),
      m_origin(kZeroV3),
      m_offset(kZeroV3),
      m_localPosition(kZeroV3),
      m_localPositionInherence(kZeroV3),
      m_localPositionMorph(kZeroV3),
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
      m_enableInverseKinematics(true)
{
}

Bone::~Bone()
{
    m_effectorRefs.releaseAll();
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_modelRef = 0;
    m_parentBoneRef = 0;
    m_targetBoneRef = 0;
    m_parentInherenceBoneRef = 0;
    m_origin.setZero();
    m_offset.setZero();
    m_localPosition.setZero();
    m_localPositionMorph.setZero();
    m_worldTransform.setIdentity();
    m_localTransform.setIdentity();
    m_destinationOrigin.setZero();
    m_fixedAxis.setZero();
    m_axisX.setZero();
    m_axisZ.setZero();
    m_weight = 0;
    m_index = -1;
    m_parentBoneIndex = -1;
    m_layerIndex = 0;
    m_destinationOriginBoneIndex = -1;
    m_parentInherenceBoneIndex = -1;
    m_globalID = 0;
    m_flags = 0;
    m_enableInverseKinematics = false;
}

bool Bone::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t nbones, boneIndexSize = info.boneIndexSize;
    if (!internal::size32(ptr, rest, nbones)) {
        VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX bones detected: size=" << nbones << " rest=" << rest);
        return false;
    }
    info.bonesPtr = ptr;
    /* BoneUnit + boneIndexSize + hierarcy + flags */
    size_t baseSize = sizeof(BoneUnit) + boneIndexSize + sizeof(int) + sizeof(uint16_t);
    for (size_t i = 0; i < nbones; i++) {
        size_t size;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX bone name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX bone name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::validateSize(ptr, baseSize, rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX bone base structure detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        uint16_t flags = *reinterpret_cast<uint16_t *>(ptr - 2);
        /* bone has destination relative or absolute */
        bool hasDestinationOriginBone = ((flags & 0x0001) == 1);
        BoneUnit p;
        if (hasDestinationOriginBone) {
            if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX destination bone index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        else {
            p = *reinterpret_cast<const BoneUnit *>(ptr);
            if (!internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
                VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX destination bone unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        /* bone has additional bias */
        if ((flags & 0x0100 || flags & 0x200) && !internal::validateSize(ptr, boneIndexSize + sizeof(float), rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX inherence bone index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* axis of bone is fixed */
        if ((flags & 0x0400) && !internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX fixed bone axis detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* axis of bone is local */
        if ((flags & 0x0800) && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX local bone axis detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* bone is transformed after external parent bone transformation */
        if ((flags & 0x2000) && !internal::validateSize(ptr, sizeof(int), rest)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX external parent index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* bone is IK */
        if (flags & 0x0020) {
            /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
            size_t extraSize = boneIndexSize + sizeof(IKUnit);
            const IKUnit &unit = *reinterpret_cast<const IKUnit *>(ptr + boneIndexSize);
            if (!internal::validateSize(ptr, extraSize, rest)) {
                VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX IK unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
            int neffectors = unit.neffectors;
            for (int j = 0; j < neffectors; j++) {
                if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                    VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX IK effector bone index detected: index=" << i << " effector=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
                size_t hasAngleConstraint;
                if (!internal::size8(ptr, rest, hasAngleConstraint)) {
                    VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX IK constraint detected: index=" << i << " effector=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
                if (hasAngleConstraint == 1 && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
                    VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX IK angle constraint detected: index=" << i << " effector=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
            }
        }
    }
    info.bonesCount = nbones;
    return true;
}

bool Bone::loadBones(const Array<Bone *> &bones, Array<Bone *> &bpsBones, Array<Bone *> &apsBones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        const int parentBoneIndex = bone->m_parentBoneIndex;
        if (parentBoneIndex >= 0) {
            if (parentBoneIndex >= nbones) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX parentBoneIndex specified: index=" << i << " bone=" << parentBoneIndex);
                return false;
            }
            else {
                Bone *parent = bones[parentBoneIndex];
                bone->m_offset -= parent->m_origin;
                bone->m_parentBoneRef = parent;
            }
        }
        const int destinationOriginBoneIndex = bone->m_destinationOriginBoneIndex;
        if (destinationOriginBoneIndex >= 0) {
            if (destinationOriginBoneIndex >= nbones) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX destinationOriginBoneIndex specified: index=" << i << " bone=" << destinationOriginBoneIndex);
                return false;
            }
            else {
                bone->m_destinationOriginBoneRef = bones[destinationOriginBoneIndex];
            }
        }
        const int targetBoneIndex = bone->m_targetBoneIndex;
        if (targetBoneIndex >= 0) {
            if (targetBoneIndex >= nbones) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX targetBoneIndex specified: index=" << i << " bone=" << targetBoneIndex);
                return false;
            }
            else {
                bone->m_targetBoneRef = bones[targetBoneIndex];
            }
        }
        const int parentInherenceBoneIndex = bone->m_parentInherenceBoneIndex;
        if (parentInherenceBoneIndex >= 0) {
            if (parentInherenceBoneIndex >= nbones) {
                VPVL2_LOG(LOG(ERROR) << "Invalid PMX parentInherenceBoneIndex specified: index=" << i << " bone=" << parentInherenceBoneIndex);
                return false;
            }
            else {
                bone->m_parentInherenceBoneRef = bones[parentInherenceBoneIndex];
            }
        }
        if (bone->hasInverseKinematics()) {
            const int neffectors = bone->m_effectorRefs.count();
            for (int j = 0; j < neffectors; j++) {
                IKEffector *ik = bone->m_effectorRefs[j];
                const int effectorBoneIndex = ik->boneID;
                if (effectorBoneIndex >= 0) {
                    if (effectorBoneIndex >= nbones) {
                        VPVL2_LOG(LOG(ERROR) << "Invalid PMX effectorBoneIndex specified: index=" << i << " effector=" << j << " bone=" << effectorBoneIndex);
                        return false;
                    }
                    else {
                        ik->bone = bones[effectorBoneIndex];
                    }
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
        if (bone->isTransformedAfterPhysicsSimulation()) {
            apsBones.append(bone);
        }
        else {
            bpsBones.append(bone);
        }
    }
    return true;
}

size_t Bone::estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo &info)
{
    const int nbones = bones.count();
    size_t size = 0;
    size += sizeof(nbones);
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        size += bone->estimateSize(info);
    }
    return size;
}

void Bone::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX, boneIndexSize = info.boneIndexSize;
    IEncoding *encoding = info.encoding;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    VPVL2_LOG(VLOG(3) << "PMXBone: name=" << reinterpret_cast<const char *>(m_name->toByteArray()));
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    VPVL2_LOG(VLOG(3) << "PMXBone: englishName=" << reinterpret_cast<const char *>(m_englishName->toByteArray()));
    const BoneUnit &unit = *reinterpret_cast<const BoneUnit *>(ptr);
    internal::setPosition(unit.vector3, m_origin);
    VPVL2_LOG(VLOG(3) << "PMXBone: origin=" << m_origin.x() << "," << m_origin.y() << "," << m_origin.z());
    m_offset = m_origin;
    m_worldTransform.setOrigin(m_origin);
    ptr += sizeof(unit);
    m_parentBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
    VPVL2_LOG(VLOG(3) << "PMXBone: parentBoneIndex=" << m_parentBoneIndex);
    m_layerIndex = *reinterpret_cast<int *>(ptr);
    ptr += sizeof(m_layerIndex);
    VPVL2_LOG(VLOG(3) << "PMXBone: layerIndex=" << m_origin.x() << "," << m_origin.y() << "," << m_origin.z());
    uint16_t flags = m_flags = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(m_flags);
    /* bone has destination */
    bool hasDestinationOriginBone = ((flags & 0x0001) == 1);
    if (hasDestinationOriginBone) {
        m_destinationOriginBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        VPVL2_LOG(VLOG(3) << "PMXBone: destinationOriginBoneIndex=" << m_destinationOriginBoneIndex);
    }
    else {
        BoneUnit offset;
        internal::getData(ptr, offset);
        internal::setPosition(offset.vector3, m_destinationOrigin);
        ptr += sizeof(offset);
        VPVL2_LOG(VLOG(3) << "PMXBone: destinationOrigin=" << m_destinationOrigin.x()
                  << "," << m_destinationOrigin.y() << "," << m_destinationOrigin.z());
    }
    /* bone has additional bias */
    if ((flags & 0x0100 || flags & 0x200)) {
        m_parentInherenceBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        internal::getData(ptr, m_weight);
        ptr += sizeof(m_weight);
        VPVL2_LOG(VLOG(3) << "PMXBone: parentInherenceBoneIndex=" << m_parentInherenceBoneIndex << " weight=" << m_weight);
    }
    /* axis of bone is fixed */
    if (flags & 0x0400) {
        BoneUnit axis;
        internal::getData(ptr, axis);
        internal::setPosition(axis.vector3, m_fixedAxis);
        ptr += sizeof(axis);
        VPVL2_LOG(VLOG(3) << "PMXBone: fixedAxis=" << m_fixedAxis.x() << "," << m_fixedAxis.y() << "," << m_fixedAxis.z());
    }
    /* axis of bone is local */
    if (flags & 0x0800) {
        BoneUnit axisX, axisZ;
        internal::getData(ptr, axisX);
        internal::setPosition(axisX.vector3, m_axisX);
        ptr += sizeof(axisX);
        VPVL2_LOG(VLOG(3) << "PMXBone: localAxisX=" << m_axisX.x() << "," << m_axisX.y() << "," << m_axisX.z());
        internal::getData(ptr, axisZ);
        internal::setPosition(axisZ.vector3, m_axisZ);
        ptr += sizeof(axisZ);
        VPVL2_LOG(VLOG(3) << "PMXBone: localAxisZ=" << m_axisZ.x() << "," << m_axisZ.y() << "," << m_axisZ.z());
    }
    /* bone is transformed after external parent bone transformation */
    if (flags & 0x2000) {
        m_globalID = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(m_globalID);
        VPVL2_LOG(VLOG(3) << "PMXBone: externalBoneIndex=" << m_globalID);
    }
    /* bone is IK */
    if (flags & 0x0020) {
        /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
        m_targetBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        IKUnit iu;
        internal::getData(ptr, iu);
        m_nloop = iu.nloop;
        m_angleConstraint = iu.angleConstraint;
        VPVL2_LOG(VLOG(3) << "PMXBone: targetBoneIndex=" << m_targetBoneIndex << " nloop=" << m_nloop << " angle=" << m_angleConstraint);
        int nlinks = iu.neffectors;
        ptr += sizeof(iu);
        for (int i = 0; i < nlinks; i++) {
            IKEffector *effector = m_effectorRefs.append(new IKEffector());
            effector->boneID = internal::readSignedIndex(ptr, boneIndexSize);
            effector->hasAngleConstraint = *reinterpret_cast<uint8_t *>(ptr) == 1;
            VPVL2_LOG(VLOG(3) << "PMXBone: boneID=" << effector->boneID << " hasAngleConstraint" << effector->hasAngleConstraint);
            ptr += sizeof(effector->hasAngleConstraint);
            if (effector->hasAngleConstraint) {
                BoneUnit lower, upper;
                internal::getData(ptr, lower);
                ptr += sizeof(lower);
                internal::getData(ptr, upper);
                ptr += sizeof(upper);
#ifdef VPVL2_COORDINATE_OPENGL
                effector->lowerLimit.setValue(-upper.vector3[0], -upper.vector3[1], lower.vector3[2]);
                effector->upperLimit.setValue(-lower.vector3[0], -lower.vector3[1], upper.vector3[2]);
#else
                ik->lowerLimit.setValue(lower.vector3[0], lower.vector3[1], lower.vector3[2]);
                ik->upperLimit.setValue(upper.vector3[0], upper.vector3[1], upper.vector3[2]);
#endif
                VPVL2_LOG(VLOG(3) << "PMXBone: lowerLimit=" << effector->lowerLimit.x() << "," << effector->lowerLimit.y() << "," << effector->lowerLimit.z());
                VPVL2_LOG(VLOG(3) << "PMXBone: upperLimit=" << effector->upperLimit.x() << "," << effector->upperLimit.y() << "," << effector->upperLimit.z());
            }
        }
    }
    size = ptr - start;
}

void Bone::write(uint8_t *data, const Model::DataInfo &info) const
{
    size_t boneIndexSize = info.boneIndexSize;
    BoneUnit bu;
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    internal::getPosition(m_origin, &bu.vector3[0]);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    internal::writeSignedIndex(m_parentBoneIndex, boneIndexSize, data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_layerIndex), sizeof(m_layerIndex), data);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_flags), sizeof(m_flags), data);
    if (internal::hasFlagBits(m_flags, 0x0001)) {
        internal::writeSignedIndex(m_destinationOriginBoneIndex, boneIndexSize, data);
    }
    else {
        internal::getPosition(m_destinationOrigin, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    }
    if (hasRotationInherence() || hasPositionInherence()) {
        internal::writeSignedIndex(m_parentInherenceBoneIndex, boneIndexSize, data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_weight), sizeof(m_weight), data);
    }
    if (hasFixedAxes()) {
        internal::getPosition(m_fixedAxis, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    }
    if (hasLocalAxes()) {
        internal::getPosition(m_axisX, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
        internal::getPosition(m_axisZ, &bu.vector3[0]);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&bu), sizeof(bu), data);
    }
    if (isTransformedByExternalParent()) {
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_globalID), sizeof(m_globalID), data);
    }
    if (hasInverseKinematics()) {
        internal::writeSignedIndex(m_targetBoneIndex, boneIndexSize, data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_nloop), sizeof(m_nloop), data);
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&m_angleConstraint), sizeof(m_angleConstraint), data);
        int nlinks = m_effectorRefs.count();
        internal::writeBytes(reinterpret_cast<const uint8_t *>(&nlinks), sizeof(nlinks), data);
        for (int i = 0; i < nlinks; i++) {
            IKEffector *link = m_effectorRefs[0];
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
}

size_t Bone::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0, boneIndexSize = info.boneIndexSize;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += sizeof(BoneUnit);
    size += boneIndexSize;
    size += sizeof(m_layerIndex);
    size += sizeof(m_flags);
    size += (internal::hasFlagBits(m_flags, 0x0001)) ? boneIndexSize : sizeof(BoneUnit);
    if (hasRotationInherence() || hasPositionInherence()) {
        size += boneIndexSize;
        size += sizeof(m_weight);
    }
    if (hasFixedAxes()) {
        size += sizeof(BoneUnit);
    }
    if (hasLocalAxes()) {
        size += sizeof(BoneUnit) * 2;
    }
    if (isTransformedByExternalParent()) {
        size += sizeof(m_globalID);
    }
    if (hasInverseKinematics()) {
        size += boneIndexSize;
        size += sizeof(IKUnit);
        int nlinks = m_effectorRefs.count();
        for (int i = 0; i < nlinks; i++) {
            size += boneIndexSize;
            size += sizeof(uint8_t);
            if (m_effectorRefs[i]->hasAngleConstraint)
                size += sizeof(BoneUnit) * 2;
        }
    }
    return size;
}

void Bone::mergeMorph(const Morph::Bone *morph, const IMorph::WeightPrecision &weight)
{
    const Scalar &w = Scalar(weight);
    m_localPositionMorph = morph->position * w;
    m_rotationMorph = Quaternion::getIdentity().slerp(morph->rotation, w);
}

void Bone::getLocalTransform(Transform &output) const
{
    getLocalTransform(m_worldTransform, output);
}

void Bone::getLocalTransform(const Transform &worldTransform, Transform &output) const
{
    output = worldTransform * Transform(Matrix3x3::getIdentity(), -m_origin);
}

void Bone::performFullTransform()
{
    Quaternion rotation = Quaternion::getIdentity();
    if (hasRotationInherence()) {
        Bone *parentBone = m_parentInherenceBoneRef;
        if (parentBone) {
            if (parentBone->hasRotationInherence())
                rotation *= parentBone->m_rotationInherence;
            else
                rotation *= parentBone->localRotation() * parentBone->m_rotationMorph;
        }
        if (!btFuzzyZero(m_weight - 1.0f))
            rotation = Quaternion::getIdentity().slerp(rotation, m_weight);
        if (parentBone && parentBone->hasInverseKinematics())
            rotation *= parentBone->m_rotationIKLink;
        m_rotationInherence = rotation * m_rotation * m_rotationMorph;
        m_rotationInherence.normalize();
    }
    rotation *= m_rotation * m_rotationMorph * m_rotationIKLink;
    rotation.normalize();
    m_worldTransform.setRotation(rotation);
    Vector3 position = kZeroV3;
    if (hasPositionInherence()) {
        Bone *parentBone = m_parentInherenceBoneRef;
        if (parentBone) {
            if (parentBone->hasPositionInherence())
                position += parentBone->m_localPositionInherence;
            else
                position += parentBone->localPosition() + parentBone->m_localPositionMorph;
        }
        if (!btFuzzyZero(m_weight - 1.0f))
            position *= m_weight;
        m_localPositionInherence = position;
    }
    position += m_localPosition + m_localPositionMorph;
    m_worldTransform.setOrigin(m_offset + position);
    if (m_parentBoneRef) {
        m_worldTransform = m_parentBoneRef->worldTransform() * m_worldTransform;
    }
    //const Quaternion &value = m_localTransform.getRotation();
    //qDebug("%s(fullTransform): %.f,%.f,%.f,.%f", m_name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::performTransform()
{
    m_worldTransform.setRotation(m_rotation);
    m_worldTransform.setOrigin(m_offset + m_localPosition);
    if (m_parentBoneRef) {
        m_worldTransform = m_parentBoneRef->worldTransform() * m_worldTransform;
    }
    //const Quaternion &value = m_localTransform.getRotation();
    //qDebug("%s(transform): %.f,%.f,%.f,.%f", m_name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::solveInverseKinematics()
{
    if (!hasInverseKinematics() || !m_enableInverseKinematics)
        return;
    const int nlinks = m_effectorRefs.count();
    const int nloops = m_nloop;
    Quaternion rotation, targetRotation = m_targetBoneRef->localRotation();
    Matrix3x3 matrix;
    for (int i = 0; i < nloops; i++) {
        for (int j = 0; j < nlinks; j++) {
            IKEffector *link = m_effectorRefs[j];
            Bone *bone = link->bone;
            const Vector3 &targetPosition = m_targetBoneRef->worldTransform().getOrigin();
            const Vector3 &destinationPosition = m_worldTransform.getOrigin();
            const Transform &transform = bone->m_worldTransform.inverse();
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
            if (btFuzzyZero(rotationAxis.length()) || btFuzzyZero(angle))
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
                IKEffector *ik = m_effectorRefs[k];
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
            m_targetBoneRef->performTransform();
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
    m_targetBoneRef->setLocalRotation(targetRotation);
}

void Bone::performUpdateLocalTransform()
{
    getLocalTransform(m_localTransform);
}

void Bone::resetIKLink()
{
    m_rotationIKLink = Quaternion::getIdentity();
}

void Bone::getEffectorBones(Array<IBone *> &value) const
{
    const int nlinks = m_effectorRefs.count();
    for (int i = 0; i < nlinks; i++) {
        IBone *bone = m_effectorRefs[i]->bone;
        value.append(bone);
    }
}

void Bone::setLocalPosition(const Vector3 &value)
{
    m_localPosition = value;
}

void Bone::setLocalRotation(const Quaternion &value)
{
    m_rotation = value;
    //qDebug("%s(rotate): %.f,%.f,%.f,.%f", m_name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

Vector3 Bone::destinationOrigin() const
{
    if (m_destinationOriginBoneRef)
        return m_destinationOriginBoneRef->worldTransform().getOrigin();
    else
        return m_worldTransform.getOrigin() + m_worldTransform.getBasis() * m_destinationOrigin;
}

Vector3 Bone::fixedAxis() const
{
    return m_fixedAxis;
}

void Bone::getLocalAxes(Matrix3x3 &value) const
{
    if (hasLocalAxes()) {
        const Vector3 &axisY = m_axisZ.cross(m_axisX);
        const Vector3 &axisZ = m_axisX.cross(axisY);
        value[0] = m_axisX;
        value[1] = axisY;
        value[2] = axisZ;
    }
    else {
        value.setIdentity();
    }
}

bool Bone::isRotateable() const
{
    return internal::hasFlagBits(m_flags, 0x0002);
}

bool Bone::isMovable() const
{
    return internal::hasFlagBits(m_flags, 0x0004);
}

bool Bone::isVisible() const
{
    return internal::hasFlagBits(m_flags, 0x0008);
}

bool Bone::isInteractive() const
{
    return internal::hasFlagBits(m_flags, 0x0010);
}

bool Bone::hasInverseKinematics() const
{
    return internal::hasFlagBits(m_flags, 0x0020);
}

bool Bone::hasRotationInherence() const
{
    return internal::hasFlagBits(m_flags, 0x0100);
}

bool Bone::hasPositionInherence() const
{
    return internal::hasFlagBits(m_flags, 0x0200);
}
bool Bone::hasFixedAxes() const
{
    return internal::hasFlagBits(m_flags, 0x0400);
}

bool Bone::hasLocalAxes() const
{
    return internal::hasFlagBits(m_flags, 0x0800);
}

bool Bone::isTransformedAfterPhysicsSimulation() const
{
    return internal::hasFlagBits(m_flags, 0x1000);
}

bool Bone::isTransformedByExternalParent() const
{
    return internal::hasFlagBits(m_flags, 0x2000);
}

bool Bone::isInverseKinematicsEnabled() const
{
    return m_enableInverseKinematics;
}

void Bone::setLocalTransform(const Transform &value)
{
    m_localTransform = value;
}

void Bone::setParentBone(Bone *value)
{
    m_parentBoneRef = value;
    m_parentBoneIndex = value ? value->index() : -1;
}

void Bone::setParentInherenceBone(Bone *value, float weight)
{
    m_parentInherenceBoneRef = value;
    m_parentInherenceBoneIndex = value ? value->index() : -1;
    m_weight = weight;
}

void Bone::setTargetBone(Bone *target, int nloop, float angleConstraint)
{
    m_targetBoneRef = target;
    m_targetBoneIndex = target ? target->index() : -1;
    m_nloop = nloop;
    m_angleConstraint = angleConstraint;
}

void Bone::setDestinationOriginBone(Bone *value)
{
    m_destinationOriginBoneRef = value;
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
    m_destinationOrigin = value;
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

void Bone::setTransformAfterPhysicsEnable(bool value)
{
    internal::toggleFlag(0x1000, value, m_flags);
}

void Bone::setTransformedByExternalParentEnable(bool value)
{
    internal::toggleFlag(0x2000, value, m_flags);
}

void Bone::setInverseKinematicsEnable(bool value)
{
    m_enableInverseKinematics = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */
