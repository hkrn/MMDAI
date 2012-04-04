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

static inline void SetLowerConstraint(const Scalar &lower,
                                      const Scalar &upper,
                                      bool beforeCenter,
                                      Scalar &output)
{
    if (output < lower) {
        const Scalar &value = 2.0 * lower - output;
        output = (value <= upper && beforeCenter) ? value : lower;
    }
}

static inline void SetUpperConstraint(const Scalar &lower,
                                      const Scalar &upper,
                                      bool beforeCenter,
                                      Scalar &output)
{
    if (output > upper) {
        const Scalar &value = 2.0 * upper - output;
        output = (value >= lower && beforeCenter) ? value : upper;
    }
}

static inline void SetConstraint(const Vector3 &lower,
                                 const Vector3 &upper,
                                 bool beforeCenter,
                                 Scalar &x,
                                 Scalar &y,
                                 Scalar &z)
{
    SetLowerConstraint(lower.x(), upper.x(), beforeCenter, x);
    SetUpperConstraint(lower.x(), upper.x(), beforeCenter, x);
    SetLowerConstraint(lower.y(), upper.y(), beforeCenter, y);
    SetUpperConstraint(lower.y(), upper.y(), beforeCenter, y);
    SetLowerConstraint(lower.z(), upper.z(), beforeCenter, z);
    SetUpperConstraint(lower.z(), upper.z(), beforeCenter, z);
}

class BoneOrderPredication
{
public:
    inline bool operator()(const pmx::Bone *left, const pmx::Bone *right) const {
        if (left->isTransformedAfterPhysicsSimulation() == right->isTransformedAfterPhysicsSimulation()) {
            if (left->index() == right->index())
                return left->id() < right->id();
            return left->index() < right->index();
        }
        return right->isTransformedAfterPhysicsSimulation();
    }
};

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
      m_IKLinkTransform(Transform::getIdentity()),
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
      m_id(-1),
      m_parentBoneIndex(-1),
      m_index(0),
      m_destinationOriginBoneIndex(-1),
      m_targetBoneIndex(-1),
      m_nloop(0),
      m_parentBoneBiasIndex(-1),
      m_globalID(0),
      m_flags(0)
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
    m_index = 0;
    m_destinationOriginBoneIndex = -1;
    m_parentBoneBiasIndex = -1;
    m_globalID = 0;
    m_flags = 0;
}

bool Bone::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.bonesPtr = ptr;
    /* BoneUnit + boneIndexSize + hierarcy + flags */
    size_t baseSize = sizeof(BoneUnit) + info.boneIndexSize + sizeof(int) + sizeof(uint16_t);
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
            if (!internal::validateSize(ptr, info.boneIndexSize, rest)) {
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
            size_t extraSize = info.boneIndexSize + sizeof(int) + sizeof(float) + sizeof(int);
            if (!internal::validateSize(ptr, extraSize, rest)) {
                return false;
            }
            int nlinks = *reinterpret_cast<int *>(ptr - sizeof(int));
            for (int i = 0; i < nlinks; i++) {
                if (!internal::validateSize(ptr, info.boneIndexSize, rest)) {
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
        if ((flags & 0x0100 || flags & 0x200) && !internal::validateSize(ptr, info.boneIndexSize + sizeof(float), rest)) {
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
        const int offsetBoneID = bone->m_destinationOriginBoneIndex;
        if (offsetBoneID >= 0) {
            if (offsetBoneID >= nbones)
                return false;
            else
                bone->m_destinationOrigin = bones[offsetBoneID]->m_origin;
        }
        const int targetBoneID = bone->m_targetBoneIndex;
        if (targetBoneID >= 0) {
            if (targetBoneID >= nbones)
                return false;
            else
                bone->m_targetBone = bones[targetBoneID];
        }
        const int parentBoneBiasID = bone->m_parentBoneBiasIndex;
        if (parentBoneBiasID >= 0) {
            if (parentBoneBiasID >= nbones)
                return false;
            else
                bone->m_parentInherenceBone = bones[parentBoneBiasID];
        }
        if (bone->hasIKLinks()) {
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
        bone->m_id = i;
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
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_name = info.encoding->toString(namePtr, nNameSize, info.codec);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = info.encoding->toString(namePtr, nNameSize, info.codec);
    const BoneUnit &unit = *reinterpret_cast<const BoneUnit *>(ptr);
    internal::setPosition(unit.vector3, m_origin);
    m_offset = m_origin;
    m_localTransform.setOrigin(m_origin);
    m_localToOrigin.setOrigin(-m_origin);
    ptr += sizeof(unit);
    m_parentBoneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
    m_index = *reinterpret_cast<int *>(ptr);
    ptr += sizeof(int);
    uint16_t flags = m_flags = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(uint16_t);
    /* bone has destination */
    bool isRelative = ((flags & 0x0001) == 1);
    if (isRelative) {
        m_destinationOriginBoneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
    }
    else {
        const BoneUnit &offset = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(offset.vector3, m_destinationOrigin);
        ptr += sizeof(offset);
    }
    /* bone is IK */
    if (flags & 0x0020) {
        /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
        m_targetBoneIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
        m_nloop = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
        m_angleConstraint = ((*reinterpret_cast<float *>(ptr)) / 4.0) * kPI;
        ptr += sizeof(float);
        int nlinks = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
        for (int i = 0; i < nlinks; i++) {
            IKLink *ik = new IKLink();
            ik->boneID = internal::readSignedIndex(ptr, info.boneIndexSize);
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
        m_parentBoneBiasIndex = internal::readSignedIndex(ptr, info.boneIndexSize);
        m_weight = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
    }
    /* axis of bone is fixed */
    if ((flags & 0x0400) && !internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
        const BoneUnit &axis = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(axis.vector3, m_fixedAxis);
        ptr += sizeof(axis);
    }
    /* axis of bone is local */
    if ((flags & 0x0800) && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
        const BoneUnit &axisX = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(axisX.vector3, m_axisX);
        ptr += sizeof(axisX);
        const BoneUnit &axisZ = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(axisZ.vector3, m_axisZ);
        ptr += sizeof(axisZ);
    }
    /* bone is transformed after external parent bone transformation */
    if ((flags & 0x2000) && !internal::validateSize(ptr, sizeof(int), rest)) {
        m_globalID = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
    }
    size = ptr - start;
}

void Bone::write(uint8_t * /* data */) const
{
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
    //rotation.normalize();
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
    if (m_parentBone)
        m_localTransform = m_parentBone->m_localTransform * m_localTransform;
}

void Bone::performTransform()
{
    m_localTransform.setRotation(m_rotationIKLink);
    m_localTransform.setOrigin(m_position);
    if (m_parentBone)
        m_localTransform = m_parentBone->m_localTransform * m_localTransform;
}

void Bone::performInverseKinematics()
{
    if (!hasIKLinks())
        return;
    const int nlinks = m_IKLinks.count();
    const int nloops = m_nloop;
    Quaternion rotation;
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
                    matrix.setRotation(bone->m_rotationIKLink);
                    matrix.getEulerZYX(z2, y2, x2);
                    x3 = x1 + x2; y3 = y1 + y2; z3 = z1 + z2;

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
                    rotation.setEulerZYX(0, 0, x1);
                }
#if IK_DEBUG
                qDebug() << "rot:   " << rotation.w() << rotation.x() << rotation.y() << rotation.z();
#endif
                bone->m_rotationIKLink = rotation * bone->m_rotationIKLink;
            }
            else {
#if IK_DEBUG
                qDebug() << "rot:   " << rotation.w() << rotation.x() << rotation.y() << rotation.z();
#endif
                bone->m_rotationIKLink *= rotation;
            }
            //bone->m_rotationIKLink.normalize();
            for (int k = j; k >= 0; k--) {
                IKLink *ik = m_IKLinks[k];
                Bone *destinationBone = ik->bone;
                destinationBone->performFullTransform();
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
            m_targetBone->performFullTransform();
#if IK_DEBUG
            qDebug("target: %s", m_targetBone->name()->toByteArray());
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
}

void Bone::performUpdateLocalTransform()
{
    m_localTransform *= m_localToOrigin;
}

const Transform Bone::localTransform() const
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
}

} /* namespace pmx */
} /* namespace vpvl2 */
