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

}

namespace vpvl2
{
namespace pmx
{

struct Bone::IKLink {
    Bone *destinationBone;
    int destinationBoneID;
    bool hasAngleConstraint;
    Vector3 lowerLimit;
    Vector3 upperLimit;
};

Bone::Bone()
    : m_parentBone(0),
      m_offsetBone(0),
      m_targetBone(0),
      m_parentBiasBone(0),
      m_name(0),
      m_englishName(0),
      m_rotation(Quaternion::getIdentity()),
      m_rotationExtra(Quaternion::getIdentity()),
      m_rotationMorph(Quaternion::getIdentity()),
      m_rotationIKLink(Quaternion::getIdentity()),
      m_localTransform(Transform::getIdentity()),
      m_IKLinkTransform(Transform::getIdentity()),
      m_origin(kZeroV3),
      m_position(kZeroV3),
      m_positionExtra(kZeroV3),
      m_positionMorph(kZeroV3),
      m_offset(kZeroV3),
      m_fixedAxis(kZeroV3),
      m_axisX(kZeroV3),
      m_axisZ(kZeroV3),
      m_bias(0),
      m_parentBoneIndex(-1),
      m_priority(0),
      m_offsetBoneIndex(-1),
      m_nlinks(0),
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
    m_offsetBone = 0;
    m_targetBone = 0;
    m_parentBiasBone = 0;
    m_position.setZero();
    m_positionMorph.setZero();
    m_localTransform.setIdentity();
    m_offset.setZero();
    m_fixedAxis.setZero();
    m_axisX.setZero();
    m_axisZ.setZero();
    m_bias = 0;
    m_parentBoneIndex = -1;
    m_priority = 0;
    m_offsetBoneIndex = -1;
    m_nlinks = 0;
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
        bool isRelative = flags & 0x0001 == 1;
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

bool Bone::loadBones(const Array<Bone *> &bones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        const int parentBoneID = bone->m_parentBoneIndex;
        if (parentBoneID >= 0) {
            if (parentBoneID >= nbones)
                return false;
            else
                bone->m_parentBone = bones[parentBoneID];
        }
        const int offsetBoneID = bone->m_offsetBoneIndex;
        if (offsetBoneID >= 0) {
            if (offsetBoneID >= nbones)
                return false;
            else
                bone->m_offsetBone = bones[offsetBoneID];
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
                bone->m_parentBiasBone = bones[parentBoneBiasID];
        }
        if (bone->hasIKLinks()) {
            const int nIK = bone->m_IKLinks.count();
            for (int j = 0; j < nIK; j++) {
                IKLink *ik = bone->m_IKLinks[j];
                const int ikTargetBoneID = ik->destinationBoneID;
                if (ikTargetBoneID >= 0) {
                    if (ikTargetBoneID >= nbones)
                        return false;
                    else
                        ik->destinationBone = bones[ikTargetBoneID];
                }
            }
        }
    }
    return true;
}

void Bone::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    StaticString::Encoding encoding = info.encoding;
    m_name = new StaticString(namePtr, nNameSize, encoding);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = new StaticString(namePtr, nNameSize, encoding);
    const BoneUnit &unit = *reinterpret_cast<const BoneUnit *>(ptr);
    internal::setPosition(unit.vector3, m_origin);
    m_localTransform.setOrigin(m_origin);
    ptr += sizeof(unit);
    m_parentBoneIndex = internal::variantIndex(ptr, info.boneIndexSize);
    m_priority = *reinterpret_cast<int *>(ptr);
    ptr += sizeof(int);
    uint16_t flags = m_flags = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(uint16_t);
    /* bone has destination */
    bool isRelative = flags & 0x0001 == 1;
    if (isRelative) {
        m_offsetBoneIndex = internal::variantIndex(ptr, info.boneIndexSize);
    }
    else {
        const BoneUnit &offset = *reinterpret_cast<const BoneUnit *>(ptr);
        internal::setPosition(offset.vector3, m_offset);
        ptr += sizeof(offset);
    }
    /* bone is IK */
    if (flags & 0x0020) {
        /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
        m_targetBoneIndex = internal::variantIndex(ptr, info.boneIndexSize);
        m_nloop = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
        m_constraintAngle = *reinterpret_cast<float *>(ptr);
        ptr += sizeof(float);
        m_nlinks = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(int);
        for (int i = 0; i < m_nlinks; i++) {
            IKLink *ik = new IKLink();
            ik->destinationBoneID = internal::variantIndex(ptr, info.boneIndexSize);
            ik->hasAngleConstraint = *reinterpret_cast<uint8_t *>(ptr) == 1;
            ptr += sizeof(ik->hasAngleConstraint);
            if (ik->hasAngleConstraint) {
                const BoneUnit &lower = *reinterpret_cast<const BoneUnit *>(ptr);
                ik->lowerLimit.setValue(lower.vector3[0], lower.vector3[1], lower.vector3[2]);
                ptr += sizeof(lower);
                const BoneUnit &upper = *reinterpret_cast<const BoneUnit *>(ptr);
                ik->upperLimit.setValue(upper.vector3[0], upper.vector3[1], upper.vector3[2]);
                ptr += sizeof(upper);
            }
        }
    }
    /* bone has additional bias */
    if ((flags & 0x0100 || flags & 0x200)) {
        m_parentBoneBiasIndex = internal::variantIndex(ptr, info.boneIndexSize);
        m_bias = *reinterpret_cast<float *>(ptr);
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

void Bone::write(uint8_t *data) const
{
}

void Bone::mergeMorph(Morph::Bone *morph, float weight)
{
    m_positionMorph = morph->position * weight;
    m_rotationMorph = morph->rotation * weight;
}

void Bone::performTransform()
{
    Quaternion rotation = Quaternion::getIdentity();
    if (hasRotationBias()) {
        Quaternion internal = Quaternion::getIdentity();
        if (m_parentBone) {
            if (m_parentBone->hasRotationBias())
                internal *= m_parentBone->m_rotationExtra;
            else
                internal *= m_parentBone->m_rotation * m_parentBone->m_rotationMorph;
        }
        if (m_bias != 1.0)
            rotation = rotation.slerp(internal, m_bias);
        if (m_parentBone->hasIKLinks())
            rotation *= m_parentBone->m_rotationIKLink;
        m_rotationExtra = rotation.slerp((internal * m_parentBone->m_rotationIKLink), m_bias) * m_rotation * m_rotationMorph;
    }
    rotation *= m_rotation * m_rotationMorph;
    if (hasIKLinks())
        rotation *= m_rotationIKLink;
    m_localTransform.setRotation(rotation);
    Vector3 position = kZeroV3;
    if (hasPositionBias()) {
        if (m_parentBone) {
            if (m_parentBone->hasPositionBias())
                position += m_parentBone->m_positionExtra + m_parentBone->m_positionMorph;
            else
                position += m_parentBone->m_position + m_parentBone->m_positionMorph;
        }
        if (m_bias != 1.0)
            position *= m_bias;
        m_positionExtra = position + m_position + m_positionMorph;
    }
    position += m_position + m_positionMorph;
    m_localTransform.setOrigin(position + offset());
    if (m_parentBone)
        m_localTransform = m_parentBone->m_localTransform * m_localTransform;
}

void Bone::performInverseKinematics()
{
    if (!hasIKLinks())
        return;
    /* source based on original MMD's IK transformation */
    static const Vector3 kUnitX(1, 0, 0), kUnitY(0, 1, 0), kUnitZ(0, 0, 1);
    static const Scalar &kHalfRadian = btRadians(45.0), &kLimitRadian = btRadians(88.0f);
    const int nlinks = m_IKLinks.count();
    const int nloops = m_nloop;
    Quaternion rotation, rx, ry, rz;
    for (int i = 0; i < nloops; i++) {
        int t = m_nloop / 2;
        for (int j = 0; j < nlinks; j++) {
            IKLink *link = m_IKLinks[i];
            Bone *destinationBone = link->destinationBone;
            const Vector3 &targetPosition = (m_targetBone->m_localTransform * m_targetBone->m_position);
            const Vector3 &destinationPosition = (destinationBone->m_localTransform * destinationBone->m_position);
            const Vector3 &v1 = (destinationPosition - targetPosition).normalized();
            const Vector3 &v2 = (destinationPosition - m_position).normalized();
            const Vector3 &v3 = v1 - v2;
            if (btFuzzyZero(v3.dot(v3)))
                break;
            Vector3 v4 = v1.cross(v2);
            const Vector3 &lowerLimit = link->lowerLimit;
            const Vector3 &upperLimit = link->upperLimit;
            if (link->hasAngleConstraint && i < t) {
                // limit x axis
                if (lowerLimit.y() == 0 && upperLimit.y() == 0 && lowerLimit.z() == 0 && upperLimit.z() == 0) {
                    const Scalar &vx = m_targetBone->m_localTransform.getBasis().tdotx(v4);
                    v4.setZero();
                    v4.setX(vx >= 0.0 ? 1.0 : -1.0);
                }
                // limit y axis
                else if (lowerLimit.x() == 0 && upperLimit.x() == 0 && lowerLimit.z() == 0 && upperLimit.z() == 0) {
                    const Scalar &vy = m_targetBone->m_localTransform.getBasis().tdoty(v4);
                    v4.setZero();
                    v4.setY(vy >= 0.0 ? 1.0 : -1.0);
                }
                // limit z axis
                else if (lowerLimit.x() == 0 && upperLimit.x() == 0 && lowerLimit.y() == 0 && upperLimit.y() == 0) {
                    const Scalar &vz = m_targetBone->m_localTransform.getBasis().tdotz(v4);
                    v4.setZero();
                    v4.setZ(vz >= 0.0 ? 1.0 : -1.0);
                }
            }
            else {
                v4 = (m_targetBone->m_localTransform.getBasis() * v4).normalized();
            }
            const Scalar &nais = btClamped(v1.dot(v2), -1.0f, 1.0f);
            const Scalar &constraintAngleRadian = m_constraintAngle * (j + 1) * 2;
            const Scalar &radianIK = btMin(btAcos(nais), constraintAngleRadian);
            const Scalar &sinIK = btSin(radianIK);
            rotation.setValue(v4.x() * sinIK, v4.y() * sinIK, v4.z() * sinIK, btCos(radianIK));
            m_targetBone->m_rotationIKLink *= rotation;
            if (j == 0)
                m_targetBone->m_rotationIKLink = m_targetBone->m_rotation * m_targetBone->m_rotationIKLink;
            btMatrix3x3 matrix(m_targetBone->m_rotationIKLink);
            if (link->hasAngleConstraint) {
                Scalar x, y, z;
                if (lowerLimit.x() > -kHalfRadian && upperLimit.x() < kHalfRadian) {
                    // x axis rotation
                    const Vector3 &m3 = matrix.getRow(2);
                    x = btAsin(-m3.y());
                    if (btFabs(x) > kLimitRadian)
                        x = x < 0 ? -kLimitRadian : kLimitRadian;
                    const Scalar &cosX = btCos(x);
                    y = btAtan2(m3.x() / cosX, m3.z() / cosX);
                    const Vector3 &m1 = matrix.getRow(0);
                    z = btAtan2(m1.x() / cosX, m1.z() / cosX);
                    SetConstraint(lowerLimit, upperLimit, i < t, x, y, z);
                    rx.setRotation(kUnitX, x);
                    ry.setRotation(kUnitY, y);
                    rz.setRotation(kUnitZ, z);
                    matrix.setRotation(rz * rx * ry);
                }
                else if (lowerLimit.y() > -kHalfRadian && upperLimit.y() < kHalfRadian) {
                    // y axis rotation
                    const Vector3 &m1 = matrix.getRow(0);
                    y = btAsin(-m1.z());
                    if (btFabs(y) > kLimitRadian)
                        y = y < 0 ? -kLimitRadian : kLimitRadian;
                    const Scalar &cosY = btCos(y);
                    z = btAtan2(m1.y() / cosY, m1.x() / cosY);
                    x = btAtan2(matrix.getRow(1).z() / cosY, matrix.getRow(2).z() / cosY);
                    SetConstraint(lowerLimit, upperLimit, i < t, x, y, z);
                    rx.setRotation(kUnitX, x);
                    ry.setRotation(kUnitY, y);
                    rz.setRotation(kUnitZ, z);
                    matrix.setRotation(rx * ry * rz);
                }
                else {
                    // z axis rotation
                    const Vector3 &m2 = matrix.getRow(1);
                    z = btAsin(-m2.x());
                    if (btFabs(z) > kLimitRadian)
                        z = z < 0 ? -kLimitRadian : kLimitRadian;
                    const Scalar &cosZ = btCos(y);
                    x = btAtan2(m2.z() / cosZ, m2.y() / cosZ);
                    y = btAtan2(matrix.getRow(2).x() / cosZ, matrix.getRow(0).x() / cosZ);
                    SetConstraint(lowerLimit, upperLimit, i < t, x, y, z);
                    rx.setRotation(kUnitX, x);
                    ry.setRotation(kUnitY, y);
                    rz.setRotation(kUnitZ, z);
                    matrix.setRotation(ry * rz * rx);
                }
            }
            Transform matrixLocal(matrix), temp(matrix);
            temp.setOrigin(-m_targetBone->m_position);
            matrixLocal = temp * matrixLocal;
            temp.setOrigin(m_position);
            matrixLocal *= temp;
            temp.setOrigin(m_targetBone->m_position);
            matrixLocal *= temp;
            m_targetBone->m_IKLinkTransform = matrixLocal;
            for (int k = j; k >= 0; k--) {
                IKLink *ik = m_IKLinks[k];
                Bone *destinationBone = ik->destinationBone;
                destinationBone->m_localTransform = destinationBone->m_IKLinkTransform * destinationBone->m_parentBone->m_localTransform;
            }
            m_targetBone->m_localTransform = m_targetBone->m_IKLinkTransform * m_targetBone->m_parentBone->m_localTransform;
        }
    }
}

const Vector3 &Bone::offset() const
{
    return m_offsetBone ? m_offsetBone->m_offset : m_offset;
}

const Transform Bone::localTransform() const
{
    return m_localTransform;// * m_localToOrigin;
}

} /* namespace pmx */
} /* namespace vpvl2 */
