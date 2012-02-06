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

}

namespace vpvl2
{
namespace pmx
{

struct Bone::IK {
    int targetBoneID;
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
      m_position(kZeroV),
      m_offset(kZeroV),
      m_fixedAxis(kZeroV),
      m_axisX(kZeroV),
      m_axisZ(kZeroV),
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
        int parentBoneBiasID = bone->m_parentBoneBiasIndex;
        if (parentBoneBiasID >= 0) {
            if (parentBoneBiasID >= nbones)
                return false;
            else
                bone->m_parentBiasBone = bones[parentBoneBiasID];
        }
    }
    return true;
}

void Bone::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_name = new StaticString(namePtr, nNameSize);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = new StaticString(namePtr, nNameSize);
    const BoneUnit &unit = *reinterpret_cast<const BoneUnit *>(ptr);
    internal::setPosition(unit.vector3, m_position);
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
            IK *ik = new IK();
            ik->targetBoneID = internal::variantIndex(ptr, info.boneIndexSize);
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

} /* namespace pmx */
} /* namespace vpvl2 */

