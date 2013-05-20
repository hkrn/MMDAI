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
    vpvl2::float32_t vector3[3];
};
struct IKUnit {
    vpvl2::int32_t numIterations;
    vpvl2::float32_t angleConstraint;
    vpvl2::int32_t numConstraints;
};

#pragma pack(pop)

using namespace vpvl2;
using namespace vpvl2::pmx;

struct BoneOrderPredication {
    inline bool operator()(const Bone *left, const Bone *right) const {
        if (left->isTransformedAfterPhysicsSimulation() == right->isTransformedAfterPhysicsSimulation()) {
            if (left->layerIndex() == right->layerIndex())
                return left->index() < right->index();
            return left->layerIndex() < right->layerIndex();
        }
        return right->isTransformedAfterPhysicsSimulation();
    }
};

static inline void ClampAngle(const Scalar &min,
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

static inline void SetPositionToIKUnit(const Vector3 &inputLower,
                                       const Vector3 &inputUpper,
                                       float *outputLower,
                                       float *outputUpper)
{
#ifdef VPVL2_COORDINATE_OPENGL
    outputLower[0] = -inputUpper.x();
    outputLower[1] = -inputUpper.y();
    outputLower[2] = inputLower.z();
    outputUpper[0] = -inputLower.x();
    outputUpper[1] = -inputLower.y();
    outputUpper[2] = inputUpper.z();
#else
    outputLower[0] = -inputUpper.x();
    outputLower[1] = -inputUpper.y();
    outputLower[2] = inputLower.z();
    outputUpper[0] = -inputLower.x();
    outputUpper[1] = -inputLower.y();
    outputUpper[2] = inputUpper.z();
#endif
}

static inline void GetPositionFromIKUnit(const float *inputLower,
                                         const float *inputUpper,
                                         Vector3 &outputLower,
                                         Vector3 &outputUpper)
{
#ifdef VPVL2_COORDINATE_OPENGL
    outputLower.setValue(-inputUpper[0], -inputUpper[1], inputLower[2]);
    outputUpper.setValue(-inputLower[0], -inputLower[1], inputUpper[2]);
#else
    outputLower.setValue(inputLower[0], inputLower[1], inputLower[2]);
    outputUpper.setValue(inputUpper[0], inputUpper[1], inputUpper[2]);
#endif
}

struct IKConstraint {
    Bone *effectorBoneRef;
    int effectorBoneIndex;
    bool hasRotationConstraint;
    Vector3 lowerLimit;
    Vector3 upperLimit;
};

}

namespace vpvl2
{
namespace pmx
{

struct Bone::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef),
          parentBoneRef(0),
          targetBoneRef(0),
          parentInherenceBoneRef(0),
          destinationOriginBoneRef(0),
          name(0),
          englishName(0),
          localRotation(Quaternion::getIdentity()),
          rotationInherence(Quaternion::getIdentity()),
          localRotationMorph(Quaternion::getIdentity()),
          effectorRotation(Quaternion::getIdentity()),
          worldTransform(Transform::getIdentity()),
          localTransform(Transform::getIdentity()),
          origin(kZeroV3),
          offsetFromParent(kZeroV3),
          localPosition(kZeroV3),
          localPositionInherence(kZeroV3),
          localPositionMorph(kZeroV3),
          destinationOrigin(kZeroV3),
          fixedAxis(kZeroV3),
          axisX(kZeroV3),
          axisZ(kZeroV3),
          angleConstraint(0.0),
          weight(1.0),
          index(-1),
          parentBoneIndex(-1),
          layerIndex(0),
          destinationOriginBoneIndex(-1),
          targetBoneIndex(-1),
          numIteration(0),
          parentInherenceBoneIndex(-1),
          globalID(0),
          flags(0),
          enableInverseKinematics(true)
    {
    }
    ~PrivateContext() {
        constraints.releaseAll();
        delete name;
        name = 0;
        delete englishName;
        englishName = 0;
        modelRef = 0;
        parentBoneRef = 0;
        targetBoneRef = 0;
        parentInherenceBoneRef = 0;
        origin.setZero();
        offsetFromParent.setZero();
        localPosition.setZero();
        localPositionMorph.setZero();
        worldTransform.setIdentity();
        localTransform.setIdentity();
        destinationOrigin.setZero();
        fixedAxis.setZero();
        axisX.setZero();
        axisZ.setZero();
        weight = 0;
        index = -1;
        parentBoneIndex = -1;
        layerIndex = 0;
        destinationOriginBoneIndex = -1;
        parentInherenceBoneIndex = -1;
        globalID = 0;
        flags = 0;
        enableInverseKinematics = false;
    }

    IModel *modelRef;
    PointerArray<IKConstraint> constraints;
    Bone *parentBoneRef;
    Bone *targetBoneRef;
    Bone *parentInherenceBoneRef;
    IBone *destinationOriginBoneRef;
    IString *name;
    IString *englishName;
    Quaternion localRotation;
    Quaternion rotationInherence;
    Quaternion localRotationMorph;
    Quaternion effectorRotation;
    Transform worldTransform;
    Transform localTransform;
    Vector3 origin;
    Vector3 offsetFromParent;
    Vector3 localPosition;
    Vector3 localPositionInherence;
    Vector3 localPositionMorph;
    Vector3 destinationOrigin;
    Vector3 fixedAxis;
    Vector3 axisX;
    Vector3 axisZ;
    float angleConstraint;
    float weight;
    int index;
    int parentBoneIndex;
    int layerIndex;
    int destinationOriginBoneIndex;
    int targetBoneIndex;
    int numIteration;
    int parentInherenceBoneIndex;
    int globalID;
    uint16_t flags;
    bool enableInverseKinematics;
};

Bone::Bone(IModel *modelRef)
    : m_context(0)
{
    m_context = new PrivateContext(modelRef);
}

Bone::~Bone()
{
    delete m_context;
    m_context = 0;
}

bool Bone::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    int32_t nbones, size, boneIndexSize = info.boneIndexSize;
    if (!internal::getTyped<int32_t>(ptr, rest, nbones)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX bones detected: size=" << nbones << " rest=" << rest);
        return false;
    }
    info.bonesPtr = ptr;
    /* BoneUnit + boneIndexSize + hierarcy + flags */
    size_t baseSize = sizeof(BoneUnit) + boneIndexSize + sizeof(int32_t) + sizeof(uint16_t);
    for (int32_t i = 0; i < nbones; i++) {
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX bone name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::getText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX bone name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        if (!internal::validateSize(ptr, baseSize, rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX bone base structure detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        uint16_t flags = *reinterpret_cast<uint16_t *>(ptr - 2);
        /* bone has destination relative or absolute */
        bool hasDestinationOriginBone = ((flags & 0x0001) == 1);
        BoneUnit p;
        if (hasDestinationOriginBone) {
            if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX destination bone index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        else {
            p = *reinterpret_cast<const BoneUnit *>(ptr);
            if (!internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX destination bone unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
        }
        /* bone has additional bias */
        if ((flags & 0x0100 || flags & 0x200) && !internal::validateSize(ptr, boneIndexSize + sizeof(float), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX inherence bone index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* axis of bone is fixed */
        if ((flags & 0x0400) && !internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX fixed bone axis detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* axis of bone is local */
        if ((flags & 0x0800) && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX local bone axis detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* bone is transformed after external parent bone transformation */
        if ((flags & 0x2000) && !internal::validateSize(ptr, sizeof(int), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX external parent index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* bone is IK */
        if (flags & 0x0020) {
            /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
            size_t extraSize = boneIndexSize + sizeof(IKUnit);
            const IKUnit &unit = *reinterpret_cast<const IKUnit *>(ptr + boneIndexSize);
            if (!internal::validateSize(ptr, extraSize, rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX IK unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
            int neffectors = unit.numConstraints;
            for (int j = 0; j < neffectors; j++) {
                if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                    VPVL2_LOG(WARNING, "Invalid size of PMX IK effector bone index detected: index=" << i << " effector=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
                uint8_t hasAngleConstraint;
                if (!internal::getTyped<uint8_t>(ptr, rest, hasAngleConstraint)) {
                    VPVL2_LOG(WARNING, "Invalid size of PMX IK constraint detected: index=" << i << " effector=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
                if (hasAngleConstraint == 1 && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
                    VPVL2_LOG(WARNING, "Invalid size of PMX IK angle constraint detected: index=" << i << " effector=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
            }
        }
    }
    info.bonesCount = nbones;
    return true;
}

bool Bone::loadBones(const Array<Bone *> &bones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *boneRef = bones[i];
        const int parentBoneIndex = boneRef->m_context->parentBoneIndex;
        if (parentBoneIndex >= 0) {
            if (parentBoneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX parentBoneIndex specified: index=" << i << " bone=" << parentBoneIndex);
                return false;
            }
            else {
                Bone *parentBoneRef = bones[parentBoneIndex];
                boneRef->m_context->offsetFromParent -= parentBoneRef->m_context->origin;
                boneRef->m_context->parentBoneRef = parentBoneRef;
            }
        }
        const int destinationOriginBoneIndex = boneRef->m_context->destinationOriginBoneIndex;
        if (destinationOriginBoneIndex >= 0) {
            if (destinationOriginBoneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX destinationOriginBoneIndex specified: index=" << i << " bone=" << destinationOriginBoneIndex);
                return false;
            }
            else {
                boneRef->m_context->destinationOriginBoneRef = bones[destinationOriginBoneIndex];
            }
        }
        const int targetBoneIndex = boneRef->m_context->targetBoneIndex;
        if (targetBoneIndex >= 0) {
            if (targetBoneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX targetBoneIndex specified: index=" << i << " bone=" << targetBoneIndex);
                return false;
            }
            else {
                boneRef->m_context->targetBoneRef = bones[targetBoneIndex];
            }
        }
        const int parentInherenceBoneIndex = boneRef->m_context->parentInherenceBoneIndex;
        if (parentInherenceBoneIndex >= 0) {
            if (parentInherenceBoneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX parentInherenceBoneIndex specified: index=" << i << " bone=" << parentInherenceBoneIndex);
                return false;
            }
            else {
                boneRef->m_context->parentInherenceBoneRef = bones[parentInherenceBoneIndex];
            }
        }
        if (boneRef->hasInverseKinematics()) {
            const Array<IKConstraint *> &constraints = boneRef->m_context->constraints;
            const int nconstraints = constraints.count();
            for (int j = 0; j < nconstraints; j++) {
                IKConstraint *constraint = constraints[j];
                const int effectorBoneIndex = constraint->effectorBoneIndex;
                if (effectorBoneIndex >= 0) {
                    if (effectorBoneIndex >= nbones) {
                        VPVL2_LOG(WARNING, "Invalid PMX effectorBoneIndex specified: index=" << i << " effector=" << j << " bone=" << effectorBoneIndex);
                        return false;
                    }
                    else {
                        constraint->effectorBoneRef = bones[effectorBoneIndex];
                    }
                }
            }
        }
        boneRef->setIndex(i);
    }
    return true;
}

void Bone::sortBones(const Array<Bone *> &bones, Array<Bone *> &bpsBones, Array<Bone *> &apsBones)
{
    Array<Bone *> orderedBonesRefs;
    orderedBonesRefs.copy(bones);
    orderedBonesRefs.sort(BoneOrderPredication());
    bpsBones.clear();
    apsBones.clear();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = orderedBonesRefs[i];
        if (bone->isTransformedAfterPhysicsSimulation()) {
            apsBones.append(bone);
        }
        else {
            bpsBones.append(bone);
        }
    }
}

void Bone::writeBones(const Array<Bone *> &bones, const Model::DataInfo &info, uint8_t *&data)
{
    const int nbones = bones.count();
    internal::writeBytes(&nbones, sizeof(nbones), data);
    for (int i = 0; i < nbones; i++) {
        const Bone *bone = bones[i];
        bone->write(data, info);
    }
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
    size_t rest = SIZE_MAX, boneIndexSize = info.boneIndexSize;
    int32_t nNameSize;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->name);
    VPVL2_VLOG(3, "PMXBone: name=" << internal::cstr(m_context->name, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->englishName);
    VPVL2_VLOG(3, "PMXBone: englishName=" << internal::cstr(m_context->englishName, "(null)"));
    const BoneUnit &unit = *reinterpret_cast<const BoneUnit *>(ptr);
    internal::setPosition(unit.vector3, m_context->origin);
    VPVL2_VLOG(3, "PMXBone: origin=" << m_context->origin.x() << "," << m_context->origin.y() << "," << m_context->origin.z());
    m_context->offsetFromParent = m_context->origin;
    m_context->worldTransform.setOrigin(m_context->origin);
    ptr += sizeof(unit);
    m_context->parentBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
    VPVL2_VLOG(3, "PMXBone: parentBoneIndex=" << m_context->parentBoneIndex);
    m_context->layerIndex = *reinterpret_cast<int *>(ptr);
    ptr += sizeof(m_context->layerIndex);
    VPVL2_VLOG(3, "PMXBone: layerIndex=" << m_context->origin.x() << "," << m_context->origin.y() << "," << m_context->origin.z());
    uint16_t flags = m_context->flags = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(m_context->flags);
    /* bone has destination */
    bool hasDestinationOriginBone = ((flags & 0x0001) == 1);
    if (hasDestinationOriginBone) {
        m_context->destinationOriginBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        VPVL2_VLOG(3, "PMXBone: destinationOriginBoneIndex=" << m_context->destinationOriginBoneIndex);
    }
    else {
        BoneUnit offset;
        internal::getData(ptr, offset);
        internal::setPosition(offset.vector3, m_context->destinationOrigin);
        ptr += sizeof(offset);
        VPVL2_VLOG(3, "PMXBone: destinationOrigin=" << m_context->destinationOrigin.x()
                  << "," << m_context->destinationOrigin.y() << "," << m_context->destinationOrigin.z());
    }
    /* bone has additional bias */
    if ((flags & 0x0100 || flags & 0x200)) {
        m_context->parentInherenceBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        internal::getData(ptr, m_context->weight);
        ptr += sizeof(m_context->weight);
        VPVL2_VLOG(3, "PMXBone: parentInherenceBoneIndex=" << m_context->parentInherenceBoneIndex << " weight=" << m_context->weight);
    }
    /* axis of bone is fixed */
    if (flags & 0x0400) {
        BoneUnit axis;
        internal::getData(ptr, axis);
        internal::setPosition(axis.vector3, m_context->fixedAxis);
        ptr += sizeof(axis);
        VPVL2_VLOG(3, "PMXBone: fixedAxis=" << m_context->fixedAxis.x() << "," << m_context->fixedAxis.y() << "," << m_context->fixedAxis.z());
    }
    /* axis of bone is local */
    if (flags & 0x0800) {
        BoneUnit axisX, axisZ;
        internal::getData(ptr, axisX);
        internal::setPosition(axisX.vector3, m_context->axisX);
        ptr += sizeof(axisX);
        VPVL2_VLOG(3, "PMXBone: localAxisX=" << m_context->axisX.x() << "," << m_context->axisX.y() << "," << m_context->axisX.z());
        internal::getData(ptr, axisZ);
        internal::setPosition(axisZ.vector3, m_context->axisZ);
        ptr += sizeof(axisZ);
        VPVL2_VLOG(3, "PMXBone: localAxisZ=" << m_context->axisZ.x() << "," << m_context->axisZ.y() << "," << m_context->axisZ.z());
    }
    /* bone is transformed after external parent bone transformation */
    if (flags & 0x2000) {
        m_context->globalID = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(m_context->globalID);
        VPVL2_VLOG(3, "PMXBone: externalBoneIndex=" << m_context->globalID);
    }
    /* bone is IK */
    if (flags & 0x0020) {
        /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
        m_context->targetBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        IKUnit iu;
        internal::getData(ptr, iu);
        m_context->numIteration = iu.numIterations;
        m_context->angleConstraint = iu.angleConstraint;
        VPVL2_VLOG(3, "PMXBone: targetBoneIndex=" << m_context->targetBoneIndex << " nloop=" << m_context->numIteration << " angle=" << m_context->angleConstraint);
        int nlinks = iu.numConstraints;
        ptr += sizeof(iu);
        for (int i = 0; i < nlinks; i++) {
            IKConstraint *constraint = m_context->constraints.append(new IKConstraint());
            constraint->effectorBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
            constraint->hasRotationConstraint = *reinterpret_cast<uint8_t *>(ptr) == 1;
            VPVL2_VLOG(3, "PMXBone: boneIndex=" << constraint->effectorBoneIndex << " hasRotationConstraint" << constraint->hasRotationConstraint);
            ptr += sizeof(constraint->hasRotationConstraint);
            if (constraint->hasRotationConstraint) {
                BoneUnit lower, upper;
                internal::getData(ptr, lower);
                ptr += sizeof(lower);
                internal::getData(ptr, upper);
                ptr += sizeof(upper);
                GetPositionFromIKUnit(&lower.vector3[0], &upper.vector3[0], constraint->lowerLimit, constraint->upperLimit);
                VPVL2_VLOG(3, "PMXBone: lowerLimit=" << constraint->lowerLimit.x() << "," << constraint->lowerLimit.y() << "," << constraint->lowerLimit.z());
                VPVL2_VLOG(3, "PMXBone: upperLimit=" << constraint->upperLimit.x() << "," << constraint->upperLimit.y() << "," << constraint->upperLimit.z());
            }
        }
    }
    size = ptr - start;
}

void Bone::write(uint8_t *&data, const Model::DataInfo &info) const
{
    size_t boneIndexSize = info.boneIndexSize;
    BoneUnit bu;
    internal::writeString(m_context->name, info.codec, data);
    internal::writeString(m_context->englishName, info.codec, data);
    internal::getPosition(m_context->origin, &bu.vector3[0]);
    internal::writeBytes(&bu, sizeof(bu), data);
    internal::writeSignedIndex(m_context->parentBoneIndex, boneIndexSize, data);
    internal::writeBytes(&m_context->layerIndex, sizeof(m_context->layerIndex), data);
    internal::writeBytes(&m_context->flags, sizeof(m_context->flags), data);
    if (internal::hasFlagBits(m_context->flags, 0x0001)) {
        internal::writeSignedIndex(m_context->destinationOriginBoneIndex, boneIndexSize, data);
    }
    else {
        internal::getPosition(m_context->destinationOrigin, &bu.vector3[0]);
        internal::writeBytes(&bu, sizeof(bu), data);
    }
    if (hasRotationInherence() || hasPositionInherence()) {
        internal::writeSignedIndex(m_context->parentInherenceBoneIndex, boneIndexSize, data);
        internal::writeBytes(&m_context->weight, sizeof(m_context->weight), data);
    }
    if (hasFixedAxes()) {
        internal::getPosition(m_context->fixedAxis, &bu.vector3[0]);
        internal::writeBytes(&bu, sizeof(bu), data);
    }
    if (hasLocalAxes()) {
        internal::getPosition(m_context->axisX, &bu.vector3[0]);
        internal::writeBytes(&bu, sizeof(bu), data);
        internal::getPosition(m_context->axisZ, &bu.vector3[0]);
        internal::writeBytes(&bu, sizeof(bu), data);
    }
    if (isTransformedByExternalParent()) {
        internal::writeBytes(&m_context->globalID, sizeof(m_context->globalID), data);
    }
    if (hasInverseKinematics()) {
        internal::writeSignedIndex(m_context->targetBoneIndex, boneIndexSize, data);
        IKUnit iku;
        iku.numIterations = m_context->numIteration;
        iku.angleConstraint = m_context->angleConstraint;
        const int nconstarints = iku.numConstraints = m_context->constraints.count();
        internal::writeBytes(&iku, sizeof(iku), data);
        BoneUnit lower, upper;
        for (int i = 0; i < nconstarints; i++) {
            IKConstraint *constraint = m_context->constraints[i];
            internal::writeSignedIndex(constraint->effectorBoneIndex, boneIndexSize, data);
            uint8_t hasAngleConstraint = constraint->hasRotationConstraint ? 1 : 0;
            internal::writeBytes(&hasAngleConstraint, sizeof(hasAngleConstraint), data);
            if (hasAngleConstraint) {
                SetPositionToIKUnit(constraint->lowerLimit, constraint->upperLimit, &lower.vector3[0], &upper.vector3[0]);
                internal::writeBytes(&lower.vector3, sizeof(lower.vector3), data);
                internal::writeBytes(&upper.vector3, sizeof(upper.vector3), data);
            }
        }
    }
}

size_t Bone::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0, boneIndexSize = info.boneIndexSize;
    size += internal::estimateSize(m_context->name, info.codec);
    size += internal::estimateSize(m_context->englishName, info.codec);
    size += sizeof(BoneUnit);
    size += boneIndexSize;
    size += sizeof(m_context->layerIndex);
    size += sizeof(m_context->flags);
    size += (internal::hasFlagBits(m_context->flags, 0x0001)) ? boneIndexSize : sizeof(BoneUnit);
    if (hasRotationInherence() || hasPositionInherence()) {
        size += boneIndexSize;
        size += sizeof(m_context->weight);
    }
    if (hasFixedAxes()) {
        size += sizeof(BoneUnit);
    }
    if (hasLocalAxes()) {
        size += sizeof(BoneUnit) * 2;
    }
    if (isTransformedByExternalParent()) {
        size += sizeof(m_context->globalID);
    }
    if (hasInverseKinematics()) {
        size += boneIndexSize;
        size += sizeof(IKUnit);
        const Array<IKConstraint *> &constraints = m_context->constraints;
        int nconstraints = constraints.count();
        for (int i = 0; i < nconstraints; i++) {
            size += boneIndexSize;
            size += sizeof(uint8_t);
            if (constraints[i]->hasRotationConstraint) {
                size += sizeof(BoneUnit) * 2;
            }
        }
    }
    return size;
}

void Bone::mergeMorph(const Morph::Bone *morph, const IMorph::WeightPrecision &weight)
{
    const Scalar &w = Scalar(weight);
    m_context->localPositionMorph = morph->position * w;
    m_context->localRotationMorph = Quaternion::getIdentity().slerp(morph->rotation, w);
}

void Bone::getLocalTransform(Transform &output) const
{
    getLocalTransform(m_context->worldTransform, output);
}

void Bone::getLocalTransform(const Transform &worldTransform, Transform &output) const
{
    output = worldTransform * Transform(Matrix3x3::getIdentity(), -m_context->origin);
}

void Bone::performFullTransform()
{
    Quaternion rotation = Quaternion::getIdentity();
    if (hasRotationInherence()) {
        Bone *parentBoneRef = m_context->parentInherenceBoneRef;
        if (parentBoneRef) {
            if (parentBoneRef->hasRotationInherence()) {
                rotation *= parentBoneRef->m_context->rotationInherence;
            }
            else {
                rotation *= parentBoneRef->localRotation() * parentBoneRef->m_context->localRotationMorph;
            }
        }
        if (!btFuzzyZero(m_context->weight - 1.0f)) {
            rotation = Quaternion::getIdentity().slerp(rotation, m_context->weight);
        }
        if (parentBoneRef && parentBoneRef->hasInverseKinematics()) {
            rotation *= parentBoneRef->m_context->effectorRotation;
        }
        m_context->rotationInherence = rotation * m_context->localRotation * m_context->localRotationMorph;
        m_context->rotationInherence.normalize();
    }
    rotation *= m_context->localRotation * m_context->localRotationMorph * m_context->effectorRotation;
    rotation.normalize();
    m_context->worldTransform.setRotation(rotation);
    Vector3 position = kZeroV3;
    if (hasPositionInherence()) {
        Bone *parentBone = m_context->parentInherenceBoneRef;
        if (parentBone) {
            if (parentBone->hasPositionInherence()) {
                position += parentBone->m_context->localPositionInherence;
            }
            else {
                position += parentBone->localTranslation() + parentBone->m_context->localPositionMorph;
            }
        }
        if (!btFuzzyZero(m_context->weight - 1.0f)) {
            position *= m_context->weight;
        }
        m_context->localPositionInherence = position;
    }
    position += m_context->localPosition + m_context->localPositionMorph;
    m_context->worldTransform.setOrigin(m_context->offsetFromParent + position);
    if (m_context->parentBoneRef) {
        m_context->worldTransform = m_context->parentBoneRef->worldTransform() * m_context->worldTransform;
    }
    //const Quaternion &value = m_context->localTransform.getRotation();
    //qDebug("%s(fullTransform): %.f,%.f,%.f,.%f", m_context->name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::performTransform()
{
    m_context->worldTransform.setRotation(m_context->localRotation);
    m_context->worldTransform.setOrigin(m_context->offsetFromParent + m_context->localPosition);
    if (IBone *parentBoneRef = m_context->parentBoneRef) {
        m_context->worldTransform = parentBoneRef->worldTransform() * m_context->worldTransform;
    }
    //const Quaternion &value = m_context->localTransform.getRotation();
    //qDebug("%s(transform): %.f,%.f,%.f,.%f", m_context->name->toByteArray(), value.w(), value.x(), value.y(), value.z());
}

void Bone::solveInverseKinematics()
{
    if (!hasInverseKinematics() || !m_context->enableInverseKinematics) {
        return;
    }
    const int nconstraints = m_context->constraints.count();
    const int niteration = m_context->numIteration;
    Quaternion rotation, targetRotation = m_context->targetBoneRef->localRotation();
    Matrix3x3 matrix;
    for (int i = 0; i < niteration; i++) {
        for (int j = 0; j < nconstraints; j++) {
            IKConstraint *constraint = m_context->constraints[j];
            Bone *effector = constraint->effectorBoneRef;
            const Vector3 &targetPosition = m_context->targetBoneRef->worldTransform().getOrigin();
            const Vector3 &rootPosition = m_context->worldTransform.getOrigin();
            const Transform &effectorTransform = effector->worldTransform().inverse();
            Vector3 v1 = effectorTransform * targetPosition;
            Vector3 v2 = effectorTransform * rootPosition;
            if (btFuzzyZero(v1.distance2(v2))) {
                i = niteration;
                break;
            }
#if IK_DEBUG
            qDebug() << "transo:" << transform.getOrigin().x() << transform.getOrigin().y() << transform.getOrigin().z();
            qDebug() << "transr:" << transform.getRotation().w() << transform.getRotation().x() << transform.getRotation().y() << transform.getRotation().z();
            qDebug() << "trot:  " << m_context->targetBone->m_context->localTransform.getRotation().w()
                     << m_context->targetBone->m_context->localTransform.getRotation().x()
                     << m_context->targetBone->m_context->localTransform.getRotation().y()
                     << m_context->targetBone->m_context->localTransform.getRotation().z();
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
            const Scalar &angle = btClamped(btAcos(v1.dot(v2)), -m_context->angleConstraint, m_context->angleConstraint);
            if (btFuzzyZero(rotationAxis.length()) || btFuzzyZero(angle))
                continue;
            rotation.setRotation(rotationAxis, angle);
            rotation.normalize();
#if IK_DEBUG
            qDebug("I:J=%d:%d", i, j);
            qDebug("target: %s", m_context->targetBone->name()->toByteArray());
            qDebug("linked: %s", bone->name()->toByteArray());
#endif
            if (constraint->hasRotationConstraint) {
                const Vector3 &lowerLimit = constraint->lowerLimit;
                const Vector3 &upperLimit = constraint->upperLimit;
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
                    matrix.setRotation(effector->m_context->localRotation);
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
                effector->setLocalRotation(rotation * effector->m_context->localRotation);
            }
            else {
#if IK_DEBUG
                qDebug() << "rot:   " << rotation.w() << rotation.x() << rotation.y() << rotation.z();
#endif
                effector->setLocalRotation(effector->localRotation() * rotation);
            }
            effector->m_context->localRotation.normalize();
            effector->m_context->effectorRotation = rotation;
            for (int k = j; k >= 0; k--) {
                IKConstraint *constraint = m_context->constraints[k];
                Bone *effectorBoneRef = constraint->effectorBoneRef;
                effectorBoneRef->performTransform();
#if IK_DEBUG
                qDebug("k=%d: %s", k, destinationBone->name()->toByteArray());
                qDebug() << destinationBone->m_context->localTransform.getOrigin().x()
                         << destinationBone->m_context->localTransform.getOrigin().y()
                         << destinationBone->m_context->localTransform.getOrigin().z();
                qDebug() << destinationBone->m_context->localTransform.getRotation().w()
                         << destinationBone->m_context->localTransform.getRotation().x()
                         << destinationBone->m_context->localTransform.getRotation().y()
                         << destinationBone->m_context->localTransform.getRotation().z();
#endif
            }
#if IK_DEBUG
            qDebug("target1: %s", m_context->targetBone->name()->toByteArray());
            qDebug() << m_context->targetBone->m_context->localTransform.getOrigin().x()
                     << m_context->targetBone->m_context->localTransform.getOrigin().y()
                     << m_context->targetBone->m_context->localTransform.getOrigin().z();
            qDebug() << m_context->targetBone->m_context->localTransform.getRotation().w()
                     << m_context->targetBone->m_context->localTransform.getRotation().x()
                     << m_context->targetBone->m_context->localTransform.getRotation().y()
                     << m_context->targetBone->m_context->localTransform.getRotation().z();
#endif
            m_context->targetBoneRef->performTransform();
#if IK_DEBUG
            qDebug("target2: %s", m_context->targetBone->name()->toByteArray());
            qDebug() << m_context->targetBone->m_context->localTransform.getOrigin().x()
                     << m_context->targetBone->m_context->localTransform.getOrigin().y()
                     << m_context->targetBone->m_context->localTransform.getOrigin().z();
            qDebug() << m_context->targetBone->m_context->localTransform.getRotation().w()
                     << m_context->targetBone->m_context->localTransform.getRotation().x()
                     << m_context->targetBone->m_context->localTransform.getRotation().y()
                     << m_context->targetBone->m_context->localTransform.getRotation().z();
#endif
        }
    }
    m_context->targetBoneRef->setLocalRotation(targetRotation);
}

void Bone::updateLocalTransform()
{
    getLocalTransform(m_context->localTransform);
}

void Bone::resetIKLink()
{
    m_context->effectorRotation = Quaternion::getIdentity();
}

Vector3 Bone::offset() const
{
    return m_context->offsetFromParent;
}

Transform Bone::worldTransform() const
{
    return m_context->worldTransform;
}

Transform Bone::localTransform() const
{
    return m_context->localTransform;
}

void Bone::getEffectorBones(Array<IBone *> &value) const
{
    const Array<IKConstraint *> &constraints = m_context->constraints;
    const int nlinks = constraints.count();
    for (int i = 0; i < nlinks; i++) {
        IKConstraint *constraint = constraints[i];
        IBone *bone = constraint->effectorBoneRef;
        value.append(bone);
    }
}

void Bone::setLocalTranslation(const Vector3 &value)
{
    m_context->localPosition = value;
}

void Bone::setLocalRotation(const Quaternion &value)
{
    m_context->localRotation = value;
}

IModel *Bone::parentModelRef() const
{
    return m_context->modelRef;
}

IBone *Bone::parentBoneRef() const
{
    return m_context->parentBoneRef;
}

IBone *Bone::targetBoneRef() const
{
    return m_context->targetBoneRef;
}

IBone *Bone::parentInherenceBoneRef() const
{
    return m_context->parentInherenceBoneRef;
}

IBone *Bone::destinationOriginBoneRef() const
{
    return m_context->destinationOriginBoneRef;
}

const IString *Bone::name() const
{
    return m_context->name;
}

const IString *Bone::englishName() const
{
    return m_context->englishName;
}

Quaternion Bone::localRotation() const
{
    return m_context->localRotation;
}

Vector3 Bone::origin() const
{
    return m_context->origin;
}

Vector3 Bone::destinationOrigin() const
{
    if (IBone *boneRef = m_context->destinationOriginBoneRef) {
        return boneRef->worldTransform().getOrigin();
    }
    else {
        return m_context->worldTransform.getOrigin() + m_context->worldTransform.getBasis() * m_context->destinationOrigin;
    }
}

Vector3 Bone::localTranslation() const
{
    return m_context->localPosition;
}

Vector3 Bone::axis() const
{
    return m_context->fixedAxis;
}

Vector3 Bone::axisX() const
{
    return m_context->axisX;
}

Vector3 Bone::axisZ() const
{
    return m_context->axisZ;
}

float32_t Bone::constraintAngle() const
{
    return m_context->angleConstraint;
}

float32_t Bone::weight() const
{
    return m_context->weight;
}

int Bone::index() const
{
    return m_context->index;
}

int Bone::layerIndex() const
{
    return m_context->layerIndex;
}

int Bone::externalIndex() const
{
    return m_context->globalID;
}

Vector3 Bone::fixedAxis() const
{
    return m_context->fixedAxis;
}

void Bone::getLocalAxes(Matrix3x3 &value) const
{
    if (hasLocalAxes()) {
        const Vector3 &axisY = m_context->axisZ.cross(m_context->axisX);
        const Vector3 &axisZ = m_context->axisX.cross(axisY);
        value[0] = m_context->axisX;
        value[1] = axisY;
        value[2] = axisZ;
    }
    else {
        value.setIdentity();
    }
}

bool Bone::isRotateable() const
{
    return internal::hasFlagBits(m_context->flags, 0x0002);
}

bool Bone::isMovable() const
{
    return internal::hasFlagBits(m_context->flags, 0x0004);
}

bool Bone::isVisible() const
{
    return internal::hasFlagBits(m_context->flags, 0x0008);
}

bool Bone::isInteractive() const
{
    return internal::hasFlagBits(m_context->flags, 0x0010);
}

bool Bone::hasInverseKinematics() const
{
    return internal::hasFlagBits(m_context->flags, 0x0020);
}

bool Bone::hasRotationInherence() const
{
    return internal::hasFlagBits(m_context->flags, 0x0100);
}

bool Bone::hasPositionInherence() const
{
    return internal::hasFlagBits(m_context->flags, 0x0200);
}
bool Bone::hasFixedAxes() const
{
    return internal::hasFlagBits(m_context->flags, 0x0400);
}

bool Bone::hasLocalAxes() const
{
    return internal::hasFlagBits(m_context->flags, 0x0800);
}

bool Bone::isTransformedAfterPhysicsSimulation() const
{
    return internal::hasFlagBits(m_context->flags, 0x1000);
}

bool Bone::isTransformedByExternalParent() const
{
    return internal::hasFlagBits(m_context->flags, 0x2000);
}

bool Bone::isInverseKinematicsEnabled() const
{
    return m_context->enableInverseKinematics;
}

void Bone::setLocalTransform(const Transform &value)
{
    m_context->localTransform = value;
}

void Bone::setParentBone(Bone *value)
{
    m_context->parentBoneRef = value;
    m_context->parentBoneIndex = value ? value->index() : -1;
}

void Bone::setParentInherenceBone(Bone *value, float weight)
{
    m_context->parentInherenceBoneRef = value;
    m_context->parentInherenceBoneIndex = value ? value->index() : -1;
    m_context->weight = weight;
}

void Bone::setTargetBone(Bone *target, int nloop, float angleConstraint)
{
    m_context->targetBoneRef = target;
    m_context->targetBoneIndex = target ? target->index() : -1;
    m_context->numIteration = nloop;
    m_context->angleConstraint = angleConstraint;
}

void Bone::setDestinationOriginBone(Bone *value)
{
    m_context->destinationOriginBoneRef = value;
    m_context->destinationOriginBoneIndex = value ? value->index() : -1;
    internal::toggleFlag(0x0001, value ? true : false, m_context->flags);
}

void Bone::setName(const IString *value)
{
    internal::setString(value, m_context->name);
}

void Bone::setEnglishName(const IString *value)
{
    internal::setString(value, m_context->englishName);
}

void Bone::setOrigin(const Vector3 &value)
{
    m_context->origin = value;
}

void Bone::setDestinationOrigin(const Vector3 &value)
{
    m_context->destinationOrigin = value;
    internal::toggleFlag(0x0001, false, m_context->flags);
}

void Bone::setFixedAxis(const Vector3 &value)
{
    m_context->fixedAxis = value;
}

void Bone::setAxisX(const Vector3 &value)
{
    m_context->axisX = value;
}

void Bone::setAxisZ(const Vector3 &value)
{
    m_context->axisZ = value;
}

void Bone::setIndex(int value)
{
    m_context->index = value;
}

void Bone::setLayerIndex(int value)
{
    m_context->layerIndex = value;
}

void Bone::setExternalIndex(int value)
{
    m_context->globalID = value;
}

void Bone::setRotateable(bool value)
{
    internal::toggleFlag(0x0002, value, m_context->flags);
}

void Bone::setMovable(bool value)
{
    internal::toggleFlag(0x0004, value, m_context->flags);
}

void Bone::setVisible(bool value)
{
    internal::toggleFlag(0x0008, value, m_context->flags);
}

void Bone::setOperatable(bool value)
{
    internal::toggleFlag(0x0010, value, m_context->flags);
}

void Bone::setIKEnable(bool value)
{
    internal::toggleFlag(0x0020, value, m_context->flags);
}

void Bone::setPositionInherenceEnable(bool value)
{
    internal::toggleFlag(0x0100, value, m_context->flags);
}

void Bone::setRotationInherenceEnable(bool value)
{
    internal::toggleFlag(0x0200, value, m_context->flags);
}

void Bone::setAxisFixedEnable(bool value)
{
    internal::toggleFlag(0x0400, value, m_context->flags);
}

void Bone::setLocalAxisEnable(bool value)
{
    internal::toggleFlag(0x0800, value, m_context->flags);
}

void Bone::setTransformAfterPhysicsEnable(bool value)
{
    internal::toggleFlag(0x1000, value, m_context->flags);
}

void Bone::setTransformedByExternalParentEnable(bool value)
{
    internal::toggleFlag(0x2000, value, m_context->flags);
}

void Bone::setInverseKinematicsEnable(bool value)
{
    m_context->enableInverseKinematics = value;
}

} /* namespace pmx */
} /* namespace vpvl2 */
