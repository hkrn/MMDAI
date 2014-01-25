/**

 Copyright (c) 2010-2014  hkrn

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

namespace
{

using namespace vpvl2;

#pragma pack(push, 1)

struct BoneUnit {
    float32 vector3[3];
};

struct IKUnit {
    int32 numIterations;
    float32 angleLimit;
    int32 numConstraints;
};

#pragma pack(pop)

using namespace vpvl2;
using namespace vpvl2::pmx;

struct IKConstraint {
    Bone *jointBoneRef;
    int jointBoneIndex;
    bool hasAngleLimit;
    Vector3 lowerLimit;
    Vector3 upperLimit;
};

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

}

namespace vpvl2
{
namespace pmx
{

struct Bone::PrivateContext {
    PrivateContext(IModel *modelRef)
        : modelRef(modelRef),
          parentBoneRef(0),
          effectorBoneRef(0),
          parentInherentBoneRef(0),
          destinationOriginBoneRef(0),
          namePtr(0),
          englishNamePtr(0),
          localRotation(Quaternion::getIdentity()),
          localInherentRotation(Quaternion::getIdentity()),
          localMorphRotation(Quaternion::getIdentity()),
          jointRotation(Quaternion::getIdentity()),
          worldTransform(Transform::getIdentity()),
          localTransform(Transform::getIdentity()),
          origin(kZeroV3),
          offsetFromParent(kZeroV3),
          localTranslation(kZeroV3),
          localInherentTranslation(kZeroV3),
          localMorphTranslation(kZeroV3),
          destinationOrigin(kZeroV3),
          fixedAxis(kZeroV3),
          axisX(kZeroV3),
          axisZ(kZeroV3),
          angleLimit(0.0),
          coefficient(1.0),
          index(-1),
          parentBoneIndex(-1),
          layerIndex(0),
          destinationOriginBoneIndex(-1),
          effectorBoneIndex(-1),
          numIteration(0),
          parentInherentBoneIndex(-1),
          globalID(0),
          flags(0),
          enableInverseKinematics(true)
    {
    }
    ~PrivateContext() {
        constraints.releaseAll();
        internal::deleteObject(namePtr);
        internal::deleteObject(englishNamePtr);
        modelRef = 0;
        parentBoneRef = 0;
        effectorBoneRef = 0;
        parentInherentBoneRef = 0;
        origin.setZero();
        offsetFromParent.setZero();
        localTranslation.setZero();
        localMorphTranslation.setZero();
        worldTransform.setIdentity();
        localTransform.setIdentity();
        destinationOrigin.setZero();
        fixedAxis.setZero();
        axisX.setZero();
        axisZ.setZero();
        coefficient = 0;
        index = -1;
        parentBoneIndex = -1;
        layerIndex = 0;
        destinationOriginBoneIndex = -1;
        parentInherentBoneIndex = -1;
        globalID = 0;
        flags = 0;
        enableInverseKinematics = false;
    }

    static void clampAngle(const Scalar &min,
                           const Scalar &max,
                           const Scalar &result,
                           Scalar &output)
    {
        if (btFuzzyZero(min) && btFuzzyZero(max)) {
            output = 0;
        }
        else if (result < min) {
            output = min;
        }
        else if (result > max) {
            output = max;
        }
    }
    static void setPositionToIKUnit(const Vector3 &inputLower,
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
    static void getPositionFromIKUnit(const float *inputLower,
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

    static Scalar clampAngle2(const Scalar &value, const Scalar &lower, const Scalar &upper, bool ikt) {
        Scalar v = value;
        if (v < lower) {
            const Scalar &tf = 2 * lower - v;
            v = (tf <= upper && ikt) ? tf : lower;
        }
        if (v > upper) {
            const Scalar &tf = 2 * lower - v;
            v = (tf <= lower && ikt) ? tf : upper;
        }
        return v;
    }
    static void setMatrix(const Scalar &x, const Scalar &y, const Scalar &z,
                          const Vector3 &lowerLimit, const Vector3 &upperLimit, bool ikt,
                          Matrix3x3 &mx, Matrix3x3 &my, Matrix3x3 &mz) {
        const Scalar &x2 = PrivateContext::clampAngle2(x, lowerLimit.x(), upperLimit.x(), ikt);
        const Scalar &y2 = PrivateContext::clampAngle2(y, lowerLimit.y(), upperLimit.y(), ikt);
        const Scalar &z2 = PrivateContext::clampAngle2(z, lowerLimit.z(), upperLimit.z(), ikt);
        mx.setRotation(Quaternion(Vector3(1, 0, 0), x2));
        my.setRotation(Quaternion(Vector3(0, 1, 0), y2));
        mz.setRotation(Quaternion(Vector3(0, 0, 1), z2));
    }

    void updateWorldTransform() {
        updateWorldTransform(localTranslation, localRotation);
    }
    void updateWorldTransform(const Vector3 &translation, const Quaternion &rotation) {
        worldTransform.setRotation(rotation);
        worldTransform.setOrigin(offsetFromParent + translation);
        if (parentBoneRef) {
            worldTransform = parentBoneRef->worldTransform() * worldTransform;
        }
    }

    IModel *modelRef;
    PointerArray<IKConstraint> constraints;
    Bone *parentBoneRef;
    Bone *effectorBoneRef;
    Bone *parentInherentBoneRef;
    Bone *destinationOriginBoneRef;
    IString *namePtr;
    IString *englishNamePtr;
    Array<PropertyEventListener *> eventRefs;
    Quaternion localRotation;
    Quaternion localInherentRotation;
    Quaternion localMorphRotation;
    Quaternion jointRotation;
    Transform worldTransform;
    Transform localTransform;
    Vector3 origin;
    Vector3 offsetFromParent;
    Vector3 localTranslation;
    Vector3 localInherentTranslation;
    Vector3 localMorphTranslation;
    Vector3 destinationOrigin;
    Vector3 fixedAxis;
    Vector3 axisX;
    Vector3 axisZ;
    float32 angleLimit;
    float32 coefficient;
    int index;
    int parentBoneIndex;
    int layerIndex;
    int destinationOriginBoneIndex;
    int effectorBoneIndex;
    int numIteration;
    int parentInherentBoneIndex;
    int globalID;
    uint16 flags;
    bool enableInverseKinematics;
};

Bone::Bone(IModel *modelRef)
    : m_context(new PrivateContext(modelRef))
{
}

Bone::~Bone()
{
    internal::deleteObject(m_context);
}

bool Bone::preparse(uint8 *&ptr, vsize &rest, Model::DataInfo &info)
{
    int32 nbones = 0, size = 0, boneIndexSize = int32(info.boneIndexSize);
    if (!internal::getTyped<int32>(ptr, rest, nbones)) {
        VPVL2_LOG(WARNING, "Invalid size of PMX bones detected: size=" << nbones << " rest=" << rest);
        return false;
    }
    info.bonesPtr = ptr;
    /* BoneUnit + boneIndexSize + hierarcy + flags */
    vsize baseSize = sizeof(BoneUnit) + boneIndexSize + sizeof(int32) + sizeof(uint16);
    for (int32 i = 0; i < nbones; i++) {
        uint8 *namePtr;
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
        uint16 flags = *reinterpret_cast<uint16 *>(ptr - 2);
        /* bone has destination relative or absolute */
        BoneUnit p;
        if (internal::hasFlagBits(flags, kHasDestinationOrigin)) {
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
        if ((internal::hasFlagBits(flags, kHasInherentRotation) ||
             internal::hasFlagBits(flags, kHasInherentTranslation)) &&
                !internal::validateSize(ptr, boneIndexSize + sizeof(float), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX inherence bone index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* axis of bone is fixed */
        if (internal::hasFlagBits(flags, kHasFixedAxis) && !internal::validateSize(ptr, sizeof(BoneUnit), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX fixed bone axis detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* axis of bone is local */
        if (internal::hasFlagBits(flags, kHasLocalAxes) && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX local bone axis detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* bone is transformed after external parent bone transformation */
        if (internal::hasFlagBits(flags, kTransformByExternalParent) && !internal::validateSize(ptr, sizeof(int), rest)) {
            VPVL2_LOG(WARNING, "Invalid size of PMX external parent index detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        /* bone is IK */
        if (internal::hasFlagBits(flags, kHasInverseKinematics)) {
            /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
            vsize extraSize = boneIndexSize + sizeof(IKUnit);
            const IKUnit &unit = *reinterpret_cast<const IKUnit *>(ptr + boneIndexSize);
            if (!internal::validateSize(ptr, extraSize, rest)) {
                VPVL2_LOG(WARNING, "Invalid size of PMX IK unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
            int nconstraints = unit.numConstraints;
            for (int j = 0; j < nconstraints; j++) {
                if (!internal::validateSize(ptr, boneIndexSize, rest)) {
                    VPVL2_LOG(WARNING, "Invalid size of PMX IK joint bone index detected: index=" << i << " joint=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
                uint8 hasAngleLimit;
                if (!internal::getTyped<uint8>(ptr, rest, hasAngleLimit)) {
                    VPVL2_LOG(WARNING, "Invalid size of PMX IK constraint detected: index=" << i << " joint=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                    return false;
                }
                if (hasAngleLimit == 1 && !internal::validateSize(ptr, sizeof(BoneUnit) * 2, rest)) {
                    VPVL2_LOG(WARNING, "Invalid size of PMX IK angle constraint detected: index=" << i << " joint=" << j << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
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
        const int targetBoneIndex = boneRef->m_context->effectorBoneIndex;
        if (targetBoneIndex >= 0) {
            if (targetBoneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX targetBoneIndex specified: index=" << i << " bone=" << targetBoneIndex);
                return false;
            }
            else {
                boneRef->m_context->effectorBoneRef = bones[targetBoneIndex];
            }
        }
        const int parentInherentBoneIndex = boneRef->m_context->parentInherentBoneIndex;
        if (parentInherentBoneIndex >= 0) {
            if (parentInherentBoneIndex >= nbones) {
                VPVL2_LOG(WARNING, "Invalid PMX parentInherentBoneIndex specified: index=" << i << " bone=" << parentInherentBoneIndex);
                return false;
            }
            else {
                boneRef->m_context->parentInherentBoneRef = bones[parentInherentBoneIndex];
            }
        }
        if (boneRef->hasInverseKinematics()) {
            const Array<IKConstraint *> &constraints = boneRef->m_context->constraints;
            const int nconstraints = constraints.count();
            for (int j = 0; j < nconstraints; j++) {
                IKConstraint *constraint = constraints[j];
                const int jointBoneIndex = constraint->jointBoneIndex;
                if (jointBoneIndex >= 0) {
                    if (jointBoneIndex >= nbones) {
                        VPVL2_LOG(WARNING, "Invalid PMX jointBoneIndex specified: index=" << i << " joint=" << j << " bone=" << jointBoneIndex);
                        return false;
                    }
                    else {
                        constraint->jointBoneRef = bones[jointBoneIndex];
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

void Bone::writeBones(const Array<Bone *> &bones, const Model::DataInfo &info, uint8 *&data)
{
    const int nbones = bones.count();
    internal::writeBytes(&nbones, sizeof(nbones), data);
    for (int i = 0; i < nbones; i++) {
        const Bone *bone = bones[i];
        bone->write(data, info);
    }
}

vsize Bone::estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo &info)
{
    const int nbones = bones.count();
    vsize size = 0;
    size += sizeof(nbones);
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        size += bone->estimateSize(info);
    }
    return size;
}

void Bone::read(const uint8 *data, const Model::DataInfo &info, vsize &size)
{
    uint8 *namePtr = 0, *ptr = const_cast<uint8 *>(data), *start = ptr;
    vsize rest = SIZE_MAX, boneIndexSize = info.boneIndexSize;
    int32 nNameSize;
    IEncoding *encoding = info.encoding;
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->namePtr);
    VPVL2_VLOG(3, "PMXBone: name=" << internal::cstr(m_context->namePtr, "(null)"));
    internal::getText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_context->englishNamePtr);
    VPVL2_VLOG(3, "PMXBone: englishName=" << internal::cstr(m_context->englishNamePtr, "(null)"));
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
    uint16 flags = m_context->flags = *reinterpret_cast<uint16 *>(ptr);
    ptr += sizeof(m_context->flags);
    /* bone has destination */
    if (internal::hasFlagBits(flags, kHasDestinationOrigin)) {
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
    if (internal::hasFlagBits(flags, kHasInherentRotation) || internal::hasFlagBits(flags, kHasInherentTranslation)) {
        m_context->parentInherentBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        internal::getData(ptr, m_context->coefficient);
        ptr += sizeof(m_context->coefficient);
        VPVL2_VLOG(3, "PMXBone: parentInherentBoneIndex=" << m_context->parentInherentBoneIndex << " weight=" << m_context->coefficient);
    }
    /* axis of bone is fixed */
    if (internal::hasFlagBits(flags, kHasFixedAxis)) {
        BoneUnit axis;
        internal::getData(ptr, axis);
        internal::setPosition(axis.vector3, m_context->fixedAxis);
        ptr += sizeof(axis);
        VPVL2_VLOG(3, "PMXBone: fixedAxis=" << m_context->fixedAxis.x() << "," << m_context->fixedAxis.y() << "," << m_context->fixedAxis.z());
    }
    /* axis of bone is local */
    if (internal::hasFlagBits(flags, kHasLocalAxes)) {
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
    if (internal::hasFlagBits(flags, kTransformByExternalParent)) {
        m_context->globalID = *reinterpret_cast<int *>(ptr);
        ptr += sizeof(m_context->globalID);
        VPVL2_VLOG(3, "PMXBone: externalBoneIndex=" << m_context->globalID);
    }
    /* bone is IK */
    if (internal::hasFlagBits(flags, kHasInverseKinematics)) {
        /* boneIndex + IK loop count + IK constraint radian per once + IK link count */
        m_context->effectorBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
        IKUnit iu;
        internal::getData(ptr, iu);
        m_context->numIteration = iu.numIterations;
        m_context->angleLimit = iu.angleLimit;
        VPVL2_VLOG(3, "PMXBone: targetBoneIndex=" << m_context->effectorBoneIndex << " nloop=" << m_context->numIteration << " angle=" << m_context->angleLimit);
        int nlinks = iu.numConstraints;
        ptr += sizeof(iu);
        for (int i = 0; i < nlinks; i++) {
            IKConstraint *constraint = m_context->constraints.append(new IKConstraint());
            constraint->jointBoneIndex = internal::readSignedIndex(ptr, boneIndexSize);
            constraint->hasAngleLimit = *reinterpret_cast<uint8 *>(ptr) == 1;
            VPVL2_VLOG(3, "PMXBone: boneIndex=" << constraint->jointBoneIndex << " hasRotationConstraint" << constraint->hasAngleLimit);
            ptr += sizeof(constraint->hasAngleLimit);
            if (constraint->hasAngleLimit) {
                BoneUnit lower, upper;
                internal::getData(ptr, lower);
                ptr += sizeof(lower);
                internal::getData(ptr, upper);
                ptr += sizeof(upper);
                PrivateContext::getPositionFromIKUnit(&lower.vector3[0], &upper.vector3[0], constraint->lowerLimit, constraint->upperLimit);
                VPVL2_VLOG(3, "PMXBone: lowerLimit=" << constraint->lowerLimit.x() << "," << constraint->lowerLimit.y() << "," << constraint->lowerLimit.z());
                VPVL2_VLOG(3, "PMXBone: upperLimit=" << constraint->upperLimit.x() << "," << constraint->upperLimit.y() << "," << constraint->upperLimit.z());
            }
        }
    }
    size = ptr - start;
}

void Bone::write(uint8 *&data, const Model::DataInfo &info) const
{
    vsize boneIndexSize = info.boneIndexSize;
    BoneUnit bu;
    internal::writeString(m_context->namePtr, info.codec, data);
    internal::writeString(m_context->englishNamePtr, info.codec, data);
    internal::getPosition(m_context->origin, &bu.vector3[0]);
    internal::writeBytes(&bu, sizeof(bu), data);
    internal::writeSignedIndex(m_context->parentBoneIndex, boneIndexSize, data);
    internal::writeBytes(&m_context->layerIndex, sizeof(m_context->layerIndex), data);
    internal::writeBytes(&m_context->flags, sizeof(m_context->flags), data);
    if (internal::hasFlagBits(m_context->flags, kHasDestinationOrigin)) {
        internal::writeSignedIndex(m_context->destinationOriginBoneIndex, boneIndexSize, data);
    }
    else {
        internal::getPosition(m_context->destinationOrigin, &bu.vector3[0]);
        internal::writeBytes(&bu, sizeof(bu), data);
    }
    if (isInherentOrientationEnabled() || isInherentTranslationEnabled()) {
        internal::writeSignedIndex(m_context->parentInherentBoneIndex, boneIndexSize, data);
        internal::writeBytes(&m_context->coefficient, sizeof(m_context->coefficient), data);
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
        internal::writeSignedIndex(m_context->effectorBoneIndex, boneIndexSize, data);
        IKUnit iku;
        iku.angleLimit = m_context->angleLimit;
        iku.numIterations = m_context->numIteration;
        const int nconstarints = iku.numConstraints = m_context->constraints.count();
        internal::writeBytes(&iku, sizeof(iku), data);
        BoneUnit lower, upper;
        for (int i = 0; i < nconstarints; i++) {
            IKConstraint *constraint = m_context->constraints[i];
            internal::writeSignedIndex(constraint->jointBoneIndex, boneIndexSize, data);
            uint8 hasAngleLimit = constraint->hasAngleLimit ? 1 : 0;
            internal::writeBytes(&hasAngleLimit, sizeof(hasAngleLimit), data);
            if (hasAngleLimit) {
                PrivateContext::setPositionToIKUnit(constraint->lowerLimit, constraint->upperLimit, &lower.vector3[0], &upper.vector3[0]);
                internal::writeBytes(&lower.vector3, sizeof(lower.vector3), data);
                internal::writeBytes(&upper.vector3, sizeof(upper.vector3), data);
            }
        }
    }
}

vsize Bone::estimateSize(const Model::DataInfo &info) const
{
    vsize size = 0, boneIndexSize = info.boneIndexSize;
    size += internal::estimateSize(m_context->namePtr, info.codec);
    size += internal::estimateSize(m_context->englishNamePtr, info.codec);
    size += sizeof(BoneUnit);
    size += boneIndexSize;
    size += sizeof(m_context->layerIndex);
    size += sizeof(m_context->flags);
    size += (internal::hasFlagBits(m_context->flags, kHasDestinationOrigin)) ? boneIndexSize : sizeof(BoneUnit);
    if (isInherentOrientationEnabled() || isInherentTranslationEnabled()) {
        size += boneIndexSize;
        size += sizeof(m_context->coefficient);
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
            size += sizeof(uint8);
            if (constraints[i]->hasAngleLimit) {
                size += sizeof(BoneUnit) * 2;
            }
        }
    }
    return size;
}

void Bone::mergeMorph(const Morph::Bone *morph, const IMorph::WeightPrecision &weight)
{
    const Scalar &w = Scalar(weight);
    m_context->localMorphTranslation = morph->position * w;
    m_context->localMorphRotation = Quaternion::getIdentity().slerp(morph->rotation, w);
}

void Bone::getLocalTransform(Transform &output) const
{
    getLocalTransform(m_context->worldTransform, output);
}

void Bone::getLocalTransform(const Transform &worldTransform, Transform &output) const
{
    output = worldTransform * Transform(Matrix3x3::getIdentity(), -m_context->origin);
}

void Bone::performTransform()
{
    Quaternion rotation(Quaternion::getIdentity());
    if (isInherentOrientationEnabled()) {
        Bone *parentBoneRef = m_context->parentInherentBoneRef;
        if (parentBoneRef) {
            if (parentBoneRef->isInherentOrientationEnabled()) {
                rotation *= parentBoneRef->m_context->localInherentRotation;
            }
            else {
                rotation *= parentBoneRef->localOrientation() * parentBoneRef->m_context->localMorphRotation;
            }
        }
        if (!btFuzzyZero(m_context->coefficient - 1.0f)) {
            rotation = Quaternion::getIdentity().slerp(rotation, m_context->coefficient);
        }
        if (parentBoneRef && parentBoneRef->hasInverseKinematics()) {
            rotation *= parentBoneRef->m_context->jointRotation;
        }
        m_context->localInherentRotation = rotation * m_context->localRotation * m_context->localMorphRotation;
        m_context->localInherentRotation.normalize();
    }
    rotation *= m_context->localRotation * m_context->localMorphRotation * m_context->jointRotation;
    rotation.normalize();
    Vector3 position(kZeroV3);
    if (isInherentTranslationEnabled()) {
        Bone *parentBone = m_context->parentInherentBoneRef;
        if (parentBone) {
            if (parentBone->isInherentTranslationEnabled()) {
                position += parentBone->m_context->localInherentTranslation;
            }
            else {
                position += parentBone->localTranslation() + parentBone->m_context->localMorphTranslation;
            }
        }
        if (!btFuzzyZero(m_context->coefficient - 1.0f)) {
            position *= m_context->coefficient;
        }
        m_context->localInherentTranslation = position;
    }
    position += m_context->localTranslation + m_context->localMorphTranslation;
    m_context->updateWorldTransform(position, rotation);
}

void Bone::solveInverseKinematics()
{
    if (!hasInverseKinematics() || !m_context->enableInverseKinematics) {
        return;
    }
    const Array<IKConstraint *> &constraints = m_context->constraints;
    const Vector3 &rootBonePosition = m_context->worldTransform.getOrigin();
    const float32 angleLimit = m_context->angleLimit;
    const int nconstraints = constraints.count();
    const int niteration = m_context->numIteration;
    const int numHalfOfIteration = niteration / 2;
    Bone *effectorBoneRef = m_context->effectorBoneRef;
    const Quaternion originalTargetRotation = effectorBoneRef->localOrientation();
    Quaternion jointRotation(Quaternion::getIdentity()), newJointLocalRotation;
    Matrix3x3 matrix, mx, my, mz, result;
    Vector3 localEffectorPosition(kZeroV3), localRootBonePosition(kZeroV3), localAxis(kZeroV3);
    for (int i = 0; i < niteration; i++) {
        const bool performConstraint = i < numHalfOfIteration;
        for (int j = 0; j < nconstraints; j++) {
            const IKConstraint *constraint = constraints[j];
            Bone *jointBoneRef = constraint->jointBoneRef;
            const Vector3 &currentEffectorPosition = effectorBoneRef->worldTransform().getOrigin();
            const Transform &jointBoneTransform = jointBoneRef->worldTransform();
            const Transform &inversedJointBoneTransform = jointBoneTransform.inverse();
            localRootBonePosition = inversedJointBoneTransform * rootBonePosition;
            localEffectorPosition = inversedJointBoneTransform * currentEffectorPosition;
            localRootBonePosition.normalize();
            localEffectorPosition.normalize();
            const Scalar &dot = localRootBonePosition.dot(localEffectorPosition);
            if (btFuzzyZero(dot)) {
                break;
            }
            localAxis = localEffectorPosition.cross(localRootBonePosition).safeNormalize();
            const Scalar &newAngleLimit = angleLimit * (j + 1) * 2;
            Scalar angle = btAcos(btClamped(dot, -1.0f, 1.0f));
            btClamp(angle, -newAngleLimit, newAngleLimit);
            jointRotation.setRotation(localAxis, angle);
            if (constraint->hasAngleLimit && performConstraint) {
                const Vector3 &lowerLimit = constraint->lowerLimit;
                const Vector3 &upperLimit = constraint->upperLimit;
                if (i == 0) {
                    //const Matrix3x3 &jointRotationMatrix = jointBoneTransform.getBasis();
                    if (btFuzzyZero(lowerLimit.y()) && btFuzzyZero(upperLimit.y())
                            && btFuzzyZero(lowerLimit.z()) && btFuzzyZero(upperLimit.z())) {
                        const Scalar &axisX = 1.0f; //btSelect(localAxis.dot(jointRotationMatrix[0]) >= 0, 1.0f, -1.0f);
                        localAxis.setValue(axisX, 0.0, 0.0);
                    }
                    else if (btFuzzyZero(lowerLimit.x()) && btFuzzyZero(upperLimit.x())
                             && btFuzzyZero(lowerLimit.z()) && btFuzzyZero(upperLimit.z())) {
                        const Scalar &axisY = 1.0f; //btSelect(localAxis.dot(jointRotationMatrix[1]) >= 0, 1.0f, -1.0f);
                        localAxis.setValue(0.0, axisY, 0.0);
                    }
                    else if (btFuzzyZero(lowerLimit.x()) && btFuzzyZero(upperLimit.x())
                             && btFuzzyZero(lowerLimit.y()) && btFuzzyZero(upperLimit.y())) {
                        const Scalar &axisZ = 1.0f; //btSelect(localAxis.dot(jointRotationMatrix[2]) >= 0, 1.0f, -1.0f);
                        localAxis.setValue(0.0, 0.0, axisZ);
                    }
                    jointRotation.setRotation(localAxis, angle);
                }
                else {
#if 1
                    (void) mx;
                    (void) my;
                    (void) mz;
                    (void) result;
                    Scalar x1, y1, z1, x2, y2, z2, x3, y3, z3;
                    matrix.setRotation(jointRotation);
                    matrix.getEulerZYX(z1, y1, x1);
                    matrix.setRotation(jointBoneRef->localOrientation());
                    matrix.getEulerZYX(z2, y2, x2);
                    x3 = x1 + x2; y3 = y1 + y2; z3 = z1 + z2;
                    PrivateContext::clampAngle(lowerLimit.x(), upperLimit.x(), x3, x1);
                    PrivateContext::clampAngle(lowerLimit.y(), upperLimit.y(), y3, y1);
                    PrivateContext::clampAngle(lowerLimit.z(), upperLimit.z(), z3, z1);
                    jointRotation.setEulerZYX(z1, y1, x1);
#else
                    const Scalar &limit = btRadians(90), &limit2 = btRadians(88);
                    Scalar x, y, z;
                    matrix.setRotation(Quaternion(-rotation.x(), -rotation.y(), rotation.z(), rotation.w()));
                    matrix.transpose();
                    if (lowerLimit.x() > -limit && upperLimit.x() < limit) {
                        x = btClamped(btAsin(matrix[2][1]), -limit2, limit);
                        y = btAtan2(matrix[2][0], matrix[2][2]);
                        z = btAtan2(matrix[0][1], matrix[1][1]);
                        PrivateContext::setMatrix(x, y, z, lowerLimit, upperLimit, ikt, mx, my, mz);
                        result = mz * mx * my;
                    }
                    else if (lowerLimit.y() > -limit && upperLimit.y() < limit) {
                        x = btClamped(btAsin(matrix[0][2]), -limit2, limit);
                        y = btAtan2(matrix[1][2], matrix[2][2]);
                        z = btAtan2(matrix[0][1], matrix[0][0]);
                        PrivateContext::setMatrix(x, y, z, lowerLimit, upperLimit, ikt, mx, my, mz);
                        result = mx * my * mz;
                        result.setIdentity();
                    }
                    else {
                        x = btClamped(btAsin(matrix[1][0]), -limit2, limit);
                        y = btAtan2(matrix[1][2], matrix[1][1]);
                        z = btAtan2(matrix[2][0], matrix[0][0]);
                        PrivateContext::setMatrix(x, y, z, lowerLimit, upperLimit, ikt, mx, my, mz);
                        result = my * mz * mx;
                    }
                    result.transpose();
                    result.getRotation(rotation);
                    rotation.setValue(-rotation.x(), -rotation.y(), rotation.z(), rotation.w());
#endif
                }
                newJointLocalRotation = jointRotation * jointBoneRef->localOrientation();
            }
            else if (i == 0) {
                newJointLocalRotation = jointRotation * jointBoneRef->localOrientation();
            }
            else {
                newJointLocalRotation = jointBoneRef->localOrientation() * jointRotation;
            }
            jointBoneRef->setLocalOrientation(newJointLocalRotation);
            jointBoneRef->m_context->jointRotation = jointRotation;
            for (int k = j; k >= 0; k--) {
                IKConstraint *constraint = constraints[k];
                Bone *jointBoneRef = constraint->jointBoneRef;
                jointBoneRef->m_context->updateWorldTransform();
            }
            effectorBoneRef->m_context->updateWorldTransform();
        }
    }
    effectorBoneRef->setLocalOrientation(originalTargetRotation);
}

void Bone::updateLocalTransform()
{
    getLocalTransform(m_context->localTransform);
}

void Bone::resetIKLink()
{
    m_context->jointRotation = Quaternion::getIdentity();
}

void Bone::addEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
        m_context->eventRefs.append(value);
    }
}

void Bone::removeEventListenerRef(PropertyEventListener *value)
{
    if (value) {
        m_context->eventRefs.remove(value);
    }
}

void Bone::getEventListenerRefs(Array<PropertyEventListener *> &value)
{
    value.copy(m_context->eventRefs);
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
        IBone *bone = constraint->jointBoneRef;
        value.append(bone);
    }
}

void Bone::setLocalTranslation(const Vector3 &value)
{
    if (m_context->localTranslation != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, localTranslationWillChange(value, this));
        m_context->localTranslation = value;
    }
}

void Bone::setLocalOrientation(const Quaternion &value)
{
    if (m_context->localRotation != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, localOrientationWillChange(value, this));
        m_context->localRotation = value;
    }
}

IModel *Bone::parentModelRef() const
{
    return m_context->modelRef;
}

IBone *Bone::parentBoneRef() const
{
    return m_context->parentBoneRef;
}

IBone *Bone::effectorBoneRef() const
{
    return m_context->effectorBoneRef;
}

IBone *Bone::parentInherentBoneRef() const
{
    return m_context->parentInherentBoneRef;
}

IBone *Bone::destinationOriginBoneRef() const
{
    return m_context->destinationOriginBoneRef;
}

const IString *Bone::name(IEncoding::LanguageType type) const
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        return m_context->namePtr;
    case IEncoding::kEnglish:
        return m_context->englishNamePtr;
    default:
        return 0;
    }
}

Quaternion Bone::localOrientation() const
{
    return m_context->localRotation;
}

Vector3 Bone::origin() const
{
    return m_context->origin;
}

Vector3 Bone::destinationOrigin() const
{
    if (const IBone *boneRef = m_context->destinationOriginBoneRef) {
        return boneRef->worldTransform().getOrigin();
    }
    else {
        return m_context->worldTransform.getOrigin() + m_context->worldTransform.getBasis() * m_context->destinationOrigin;
    }
}

Vector3 Bone::localTranslation() const
{
    return m_context->localTranslation;
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

float32 Bone::constraintAngle() const
{
    return m_context->angleLimit;
}

float32 Bone::coefficient() const
{
    return m_context->coefficient;
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
    return internal::hasFlagBits(m_context->flags, kRotatetable);
}

bool Bone::isMovable() const
{
    return internal::hasFlagBits(m_context->flags, kMovable);
}

bool Bone::isVisible() const
{
    return internal::hasFlagBits(m_context->flags, kVisible);
}

bool Bone::isInteractive() const
{
    return internal::hasFlagBits(m_context->flags, kInteractive);
}

bool Bone::hasInverseKinematics() const
{
    return internal::hasFlagBits(m_context->flags, kHasInverseKinematics);
}

bool Bone::isInherentOrientationEnabled() const
{
    return internal::hasFlagBits(m_context->flags, kHasInherentRotation);
}

bool Bone::isInherentTranslationEnabled() const
{
    return internal::hasFlagBits(m_context->flags, kHasInherentTranslation);
}

bool Bone::hasFixedAxes() const
{
    return internal::hasFlagBits(m_context->flags, kHasFixedAxis);
}

bool Bone::hasLocalAxes() const
{
    return internal::hasFlagBits(m_context->flags, kHasLocalAxes);
}

bool Bone::isTransformedAfterPhysicsSimulation() const
{
    return internal::hasFlagBits(m_context->flags, kTransformAfterPhysics);
}

bool Bone::isTransformedByExternalParent() const
{
    return internal::hasFlagBits(m_context->flags, kTransformByExternalParent);
}

bool Bone::isInverseKinematicsEnabled() const
{
    return hasInverseKinematics() && m_context->enableInverseKinematics;
}

void Bone::setLocalTransform(const Transform &value)
{
    m_context->localTransform = value;
}

void Bone::setParentBoneRef(IBone *value)
{
    if (!value || (value && value->parentModelRef() == m_context->modelRef)) {
        m_context->parentBoneRef = static_cast<Bone *>(value);
        m_context->parentBoneIndex = value ? value->index() : -1;
    }
}

void Bone::setParentInherentBoneRef(IBone *value, float32 coefficient)
{
    if (!value || (value && value->parentModelRef() == m_context->modelRef)) {
        m_context->parentInherentBoneRef = static_cast<Bone *>(value);
        m_context->parentInherentBoneIndex = value ? value->index() : -1;
        m_context->coefficient = coefficient;
    }
}

void Bone::setEffectorBoneRef(IBone *effector, int numIteration, float angleLimit)
{
    if (!effector || (effector && effector->parentModelRef() == m_context->modelRef)) {
        m_context->effectorBoneRef = static_cast<Bone *>(effector);
        m_context->effectorBoneIndex = effector ? effector->index() : -1;
        m_context->numIteration = numIteration;
        m_context->angleLimit = angleLimit;
    }
}

void Bone::setDestinationOriginBoneRef(IBone *value)
{
    if (!value || (value && value->parentModelRef() == m_context->modelRef)) {
        m_context->destinationOriginBoneRef = static_cast<Bone *>(value);
        m_context->destinationOriginBoneIndex = value ? value->index() : -1;
        internal::toggleFlag(kHasDestinationOrigin, value ? true : false, m_context->flags);
    }
}

void Bone::setName(const IString *value, IEncoding::LanguageType type)
{
    switch (type) {
    case IEncoding::kDefaultLanguage:
    case IEncoding::kJapanese:
        if (value && !value->equals(m_context->namePtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->namePtr);
        }
        break;
    case IEncoding::kEnglish:
        if (value && !value->equals(m_context->englishNamePtr)) {
            VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, nameWillChange(value, type, this));
            internal::setString(value, m_context->englishNamePtr);
        }
        break;
    default:
        break;
    }
}

void Bone::setOrigin(const Vector3 &value)
{
    m_context->origin = value;
}

void Bone::setDestinationOrigin(const Vector3 &value)
{
    m_context->destinationOrigin = value;
    internal::toggleFlag(kHasDestinationOrigin, false, m_context->flags);
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
    internal::toggleFlag(kRotatetable, value, m_context->flags);
}

void Bone::setMovable(bool value)
{
    internal::toggleFlag(kMovable, value, m_context->flags);
}

void Bone::setVisible(bool value)
{
    internal::toggleFlag(kVisible, value, m_context->flags);
}

void Bone::setInteractive(bool value)
{
    internal::toggleFlag(kInteractive, value, m_context->flags);
}

void Bone::setHasInverseKinematics(bool value)
{
    internal::toggleFlag(kHasInverseKinematics, value, m_context->flags);
}

void Bone::setInherentOrientationEnable(bool value)
{
    internal::toggleFlag(kHasInherentTranslation, value, m_context->flags);
}

void Bone::setInherentTranslationEnable(bool value)
{
    internal::toggleFlag(kHasInherentRotation, value, m_context->flags);
}

void Bone::setFixedAxisEnable(bool value)
{
    internal::toggleFlag(kHasFixedAxis, value, m_context->flags);
}

void Bone::setLocalAxesEnable(bool value)
{
    internal::toggleFlag(kHasLocalAxes, value, m_context->flags);
}

void Bone::setTransformAfterPhysicsEnable(bool value)
{
    internal::toggleFlag(kTransformAfterPhysics, value, m_context->flags);
}

void Bone::setTransformedByExternalParentEnable(bool value)
{
    internal::toggleFlag(kTransformByExternalParent, value, m_context->flags);
}

void Bone::setInverseKinematicsEnable(bool value)
{
    if (m_context->enableInverseKinematics != value) {
        VPVL2_TRIGGER_PROPERTY_EVENTS(m_context->eventRefs, inverseKinematicsEnableWillChange(value, this));
        m_context->enableInverseKinematics = value;
    }
}

} /* namespace pmx */
} /* namespace vpvl2 */
