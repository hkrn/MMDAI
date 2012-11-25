/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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
#include "vpvl2/pmd2/Bone.h"

namespace
{

using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct BoneUnit {
    uint8_t name[Bone::kNameSize];
    int16_t parentBoneID;
    int16_t childBoneID;
    uint8_t type;
    int16_t targetBoneID;
    float position[3];
};

struct IKUnit
{
    int16_t rootBoneID;
    int16_t targetBoneID;
    uint8_t nlinks;
    uint16_t niterations;
    float angle;
};

#pragma pack(pop)

static const vpvl2::Vector3 kXAlignAxis(1.0f, 0.0f, 0.0f);
const float kMinDistance    = 0.0001f;
const float kMinAngle       = 0.00000001f;
const float kMinAxis        = 0.0000001f;
const float kMinRotationSum = 0.002f;
const float kMinRotation    = 0.00001f;

}

namespace vpvl2
{
namespace pmd2
{

struct Bone::IKConstraint {
    Array<Bone *> effectors;
    Bone *target;
    int niterations;
    float angle;
};

const int Bone::kNameSize;
const int Bone::kCategoryNameSize;

Bone::Bone(IModel *parentModelRef, IEncoding *encodingRef)
    : m_parentModelRef(parentModelRef),
      m_encodingRef(encodingRef),
      m_name(0),
      m_parentBoneRef(0),
      m_targetBoneRef(0),
      m_childBoneRef(0),
      m_constraint(0),
      m_fixedAxis(kZeroV3),
      m_origin(kZeroV3),
      m_offset(kZeroV3),
      m_localPosition(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_worldTransform(Transform::getIdentity()),
      m_localTransform(Transform::getIdentity()),
      m_type(kUnknown),
      m_index(-1),
      m_parentBoneIndex(0),
      m_targetBoneIndex(0),
      m_childBoneIndex(0),
      m_enableInverseKinematics(false)
{
}

Bone::~Bone()
{
    delete m_name;
    m_name = 0;
    delete m_constraint;
    m_constraint = 0;
    m_encodingRef = 0;
    m_parentBoneRef = 0;
    m_childBoneRef = 0;
    m_targetBoneRef = 0;
    m_index = -1;
    m_parentBoneIndex = 0;
    m_childBoneIndex = 0;
    m_targetBoneIndex = 0;
    m_type = kUnknown;
    m_fixedAxis.setZero();
    m_origin.setZero();
    m_offset.setZero();
    m_localPosition.setZero();
    m_worldTransform.setIdentity();
    m_localTransform.setIdentity();
    m_enableInverseKinematics = false;
}

bool Bone::preparseBones(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size16(ptr, rest, size) || size * sizeof(BoneUnit) > rest) {
        return false;
    }
    info.bonesCount = size;
    info.bonesPtr = ptr;
    internal::readBytes(size * sizeof(BoneUnit), ptr, rest);
    return true;
}

bool Bone::preparseIKConstraints(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size16(ptr, rest, size)) {
        return false;
    }
    info.IKConstraintsCount = size;
    info.IKConstraintsPtr = ptr;
    IKUnit unit;
    size_t unitSize = 0;
    for (size_t i = 0; i < size; i++) {
        if (sizeof(unit) > rest) {
            return false;
        }
        internal::getData(ptr, unit);
        unitSize = sizeof(unit) + unit.nlinks * sizeof(uint16_t);
        if (unitSize > rest) {
            return false;
        }
        internal::readBytes(unitSize, ptr, rest);
    }
    return true;
}

bool Bone::loadBones(const Array<Bone *> &bones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        bone->m_index = i;
        const int parentBoneIndex = bone->m_parentBoneIndex;
        if (parentBoneIndex >= 0) {
            if (parentBoneIndex >= nbones) {
                return false;
            }
            else {
                Bone *parent = bones[parentBoneIndex];
                bone->m_offset -= parent->m_origin;
                bone->m_parentBoneRef = parent;
            }
        }
        const int targetBoneIndex = bone->m_targetBoneIndex;
        if (targetBoneIndex >= 0) {
            if (targetBoneIndex >= nbones)
                return false;
            else
                bone->m_targetBoneRef = bones[targetBoneIndex];
        }
        const int childBoneIndex = bone->m_childBoneIndex;
        if (childBoneIndex >= 0) {
            if (childBoneIndex >= nbones)
                return false;
            else
                bone->m_childBoneRef = bones[childBoneIndex];
        }
    }
    return true;
}

void Bone::readIKConstraint(const uint8_t *data, const Array<Bone *> &bones, size_t &size)
{
    IKUnit unit;
    internal::getData(data, unit);
    int nlinks = unit.nlinks, nbones = bones.count();
    int targetIndex = unit.targetBoneID;
    int rootIndex = unit.rootBoneID;
    if (internal::checkBound(targetIndex, 0, nbones) && internal::checkBound(rootIndex, 0, nbones)) {
        uint8_t *ptr = const_cast<uint8_t *>(data + sizeof(unit));
        Array<Bone *> effectors;
        for (int i = 0; i < nlinks; i++) {
            int boneIndex = internal::readUnsignedIndex(ptr, sizeof(uint16_t));
            if (internal::checkBound(boneIndex, 0, nbones)) {
                Bone *bone = bones[boneIndex];
                effectors.add(bone);
            }
        }
        Bone *rootBone = bones[rootIndex], *targetBone = bones[targetIndex];
        IKConstraint *constraint = rootBone->m_constraint = new IKConstraint();
        constraint->effectors.copy(effectors);
        constraint->target = targetBone;
        constraint->niterations = unit.niterations;
        constraint->angle = unit.angle;
        rootBone->m_targetBoneIndex = targetBone->index();
    }
    size = sizeof(unit) + sizeof(uint16_t) * nlinks;
}

size_t Bone::estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo &info)
{
    const int nbones = bones.count();
    size_t size = 0;
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        size += bone->estimateBoneSize(info);
        size += bone->estimateIKConstraintsSize(info);
    }
    return size;
}

void Bone::readBone(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    BoneUnit unit;
    internal::getData(data, unit);
    m_name = m_encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize);
    m_childBoneIndex = unit.childBoneID;
    m_parentBoneIndex = unit.parentBoneID;
    m_targetBoneIndex = unit.targetBoneID;
    m_type = static_cast<Type>(unit.type);
    internal::setPosition(unit.position, m_origin);
    m_offset = m_origin;
    size = sizeof(unit);
}

size_t Bone::estimateBoneSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    size += sizeof(BoneUnit);
    return size;
}

size_t Bone::estimateIKConstraintsSize(const Model::DataInfo & /* info */) const
{
    size_t size = 0;
    if (m_constraint) {
        size += sizeof(IKUnit);
        size += sizeof(uint16_t) * m_constraint->effectors.count();
    }
    return size;
}

void Bone::write(uint8_t *data, const Model::DataInfo & /* info */) const
{
    BoneUnit unit;
    unit.childBoneID = m_childBoneIndex;
    unit.parentBoneID = m_parentBoneIndex;
    uint8_t *name = m_encodingRef->toByteArray(m_name, IString::kShiftJIS);
    internal::copyBytes(unit.name, name, sizeof(unit.name));
    m_encodingRef->disposeByteArray(name);
    internal::getPosition(m_origin, unit.position);
    unit.targetBoneID = m_targetBoneIndex;
    unit.type = m_type;
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&unit), sizeof(unit));
}

void Bone::performTransform()
{
    if (m_type == kUnderRotate && m_targetBoneRef) {
        const Quaternion &rotation = m_rotation * m_targetBoneRef->localRotation();
        m_worldTransform.setRotation(rotation);
    }
    else if (m_type == kFollowRotate && m_childBoneRef) {
        const Scalar coef(m_targetBoneIndex * 0.01f);
        const Quaternion &rotation = Quaternion::getIdentity().slerp(m_rotation, coef);
        m_worldTransform.setRotation(rotation);
    }
    else {
        m_worldTransform.setRotation(m_rotation);
    }
    m_worldTransform.setOrigin(m_offset + m_localPosition);
    if (m_parentBoneRef) {
        m_worldTransform = m_parentBoneRef->worldTransform() * m_worldTransform;
    }
    getLocalTransform(m_localTransform);
}

void Bone::solveInverseKinematics()
{
    if (!m_constraint)
        return;
    const Array<Bone *> &effectors = m_constraint->effectors;
    const int neffectors = effectors.count();
    Bone *targetBoneRef = m_constraint->target;
    const Quaternion &originTargetRotation = targetBoneRef->localRotation();
    const Vector3 &destPosition = m_worldTransform.getOrigin();
    const Scalar &angleLimit = m_constraint->angle;
    Quaternion q;
    Matrix3x3 matrix;
    Vector3 localDestination, localTarget;
    int niterations = m_constraint->niterations;
    for (int i = 0; i < niterations; i++) {
        for (int j = 0; j < neffectors; j++) {
            Bone *effector = effectors[j];
            const Vector3 &targetPosition = targetBoneRef->worldTransform().getOrigin();
            const Transform &transform = effector->worldTransform().inverse();
            localDestination = transform * destPosition;
            localTarget = transform * targetPosition;
            if (localDestination.distance2(localTarget) < kMinDistance) {
                i = niterations;
                break;
            }
            localDestination.safeNormalize();
            localTarget.safeNormalize();
            const Scalar &dot = localDestination.dot(localTarget);
            if (dot > 1.0f)
                continue;
            Scalar angle = btAcos(dot);
            if (btFabs(angle) < kMinAngle)
                continue;
            btClamp(angle, -angleLimit, angleLimit);
            Vector3 axis = localTarget.cross(localDestination);
            if (axis.length2() < kMinAxis && i > 0)
                continue;
            axis.normalize();
            q.setRotation(axis, angle);
            if (effector->isAxisXAligned()) {
                if (i == 0) {
                    q.setRotation(kXAlignAxis, btFabs(angle));
                }
                else {
                    Scalar x, y, z, cx, cy, cz;
                    matrix.setIdentity();
                    matrix.setRotation(q);
                    matrix.getEulerZYX(z, y, x);
                    matrix.setRotation(effector->localRotation());
                    matrix.getEulerZYX(cz, cy, cx);
                    if (x + cx > SIMD_PI)
                        x = SIMD_PI - cx;
                    if (kMinRotationSum > x + cx)
                        x = kMinRotationSum - cx;
                    btClamp(x, -angleLimit, angleLimit);
                    if (btFabs(x) < kMinRotation)
                        continue;
                    q.setEulerZYX(0.0f, 0.0f, x);
                }
                const Quaternion &q2 = q * effector->localRotation();
                effector->setLocalRotation(q2);
            }
            else {
                const Quaternion &q2 = effector->localRotation() * q;
                effector->setLocalRotation(q2);
            }
            for (int k = j; k >= 0; k--) {
                Bone *bone = effectors[k];
                bone->performTransform();
            }
            targetBoneRef->performTransform();
        }
    }
    targetBoneRef->setLocalRotation(originTargetRotation);
    targetBoneRef->performTransform();
}

const IString *Bone::name() const
{
    return m_name;
}

int Bone::index() const
{
    return m_index;
}

IModel *Bone::parentModelRef() const
{
    return m_parentModelRef;
}

IBone *Bone::parentBoneRef() const
{
    return m_parentBoneRef;
}

IBone *Bone::targetBoneRef() const
{
    return m_targetBoneRef;
}

Transform Bone::worldTransform() const
{
    return m_worldTransform;
}

Transform Bone::localTransform() const
{
    return m_localTransform;
}

void Bone::getLocalTransform(Transform &world2LocalTransform) const
{
    getLocalTransform(m_worldTransform, world2LocalTransform);
}

void Bone::getLocalTransform(const Transform &worldTransform, Transform &output) const
{
    output = worldTransform * Transform(Matrix3x3::getIdentity(), -m_origin);
}

void Bone::setLocalTransform(const Transform &value)
{
    m_localTransform = value;
}

Vector3 Bone::origin() const
{
    return m_origin;
}

Vector3 Bone::destinationOrigin() const
{
    return m_parentBoneRef ? m_parentBoneRef->origin() : kZeroV3;
}

Vector3 Bone::localPosition() const
{
    return m_localPosition;
}

Quaternion Bone::localRotation() const
{
    return m_rotation;
}

void Bone::getEffectorBones(Array<IBone *> &value) const
{
    if (m_constraint) {
        const Array<Bone *> &effectors = m_constraint->effectors;
        const int nbones = effectors.count();
        for (int i = 0; i < nbones; i++) {
            IBone *bone = effectors[i];
            value.add(bone);
        }
    }
}

void Bone::setLocalPosition(const Vector3 &value)
{
    m_localPosition = value;
}

void Bone::setLocalRotation(const Quaternion &value)
{
    m_rotation = value;
}

bool Bone::isMovable() const
{
    return m_type == kRotateAndMove;
}

bool Bone::isRotateable() const
{
    return m_type == kRotate || m_type == kRotateAndMove;
}

bool Bone::isVisible() const
{
    return m_type != kInvisible;
}

bool Bone::isInteractive() const
{
    return isRotateable();
}

bool Bone::hasInverseKinematics() const
{
    return m_type == kIKDestination;
}

bool Bone::hasFixedAxes() const
{
    return m_type == kTwist;
}

bool Bone::hasLocalAxes() const
{
    if (m_encodingRef && m_name) {
        bool hasFinger = m_name->contains(m_encodingRef->stringConstant(IEncoding::kFinger));
        bool hasArm = m_name->endsWith(m_encodingRef->stringConstant(IEncoding::kArm));
        bool hasElbow = m_name->endsWith(m_encodingRef->stringConstant(IEncoding::kElbow));
        bool hasWrist = m_name->endsWith(m_encodingRef->stringConstant(IEncoding::kWrist));
        return hasFinger || hasArm || hasElbow || hasWrist;
    }
    return false;
}

Vector3 Bone::fixedAxis() const
{
    return m_fixedAxis;
}

void Bone::getLocalAxes(Matrix3x3 &value) const
{
    if (hasLocalAxes()) {
        const Vector3 &axisX = (m_childBoneRef->origin() - origin()).normalized();
        Vector3 tmp1 = axisX;
        if (m_name->startsWith(m_encodingRef->stringConstant(IEncoding::kLeft)))
            tmp1.setY(-axisX.y());
        else
            tmp1.setX(-axisX.x());
        const Vector3 &axisZ = axisX.cross(tmp1).normalized();
        Vector3 tmp2 = axisX;
        tmp2.setZ(-axisZ.z());
        const Vector3 &axisY = tmp2.cross(-axisX).normalized();
        value[0] = axisX;
        value[1] = axisY;
        value[2] = axisZ;
    }
    else {
        value.setIdentity();
    }
}

bool Bone::isAxisXAligned()
{
    if (m_encodingRef && m_name) {
        bool isRightKnee = m_name->equals(m_encodingRef->stringConstant(IEncoding::kRightKnee));
        bool isLeftKnee = m_name->equals(m_encodingRef->stringConstant(IEncoding::kLeftKnee));
        return isRightKnee || isLeftKnee;
    }
    return false;
}

void Bone::setInverseKinematicsEnable(bool value)
{
    m_enableInverseKinematics = value;
}

}
}
