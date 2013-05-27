/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2013  hkrn                                    */
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
#include "vpvl2/internal/ModelHelper.h"
#include "vpvl2/pmd2/Bone.h"

namespace
{

using namespace vpvl2;
using namespace vpvl2::pmd2;

#pragma pack(push, 1)

struct BoneUnit {
    vpvl2::uint8_t name[internal::kPMDBoneNameSize];
    vpvl2::int16_t parentBoneID;
    vpvl2::int16_t childBoneID;
    vpvl2::uint8_t type;
    vpvl2::int16_t targetBoneID;
    vpvl2::float32_t position[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmd2
{

struct Bone::PrivateContext {
    PrivateContext(IModel *parentModelRef, IEncoding *encodingRef)
        : parentModelRef(parentModelRef),
          encodingRef(encodingRef),
          namePtr(0),
          englishNamePtr(0),
          parentBoneRef(0),
          targetBoneRef(0),
          childBoneRef(0),
          fixedAxis(kZeroV3),
          origin(kZeroV3),
          offset(kZeroV3),
          localTranslation(kZeroV3),
          rotation(Quaternion::getIdentity()),
          worldTransform(Transform::getIdentity()),
          localTransform(Transform::getIdentity()),
          type(kUnknown),
          index(-1),
          parentBoneIndex(0),
          targetBoneIndex(0),
          childBoneIndex(0),
          enableInverseKinematics(true)
    {
    }
    ~PrivateContext() {
        delete namePtr;
        namePtr = 0;
        delete englishNamePtr;
        englishNamePtr = 0;
        encodingRef = 0;
        parentBoneRef = 0;
        childBoneRef = 0;
        targetBoneRef = 0;
        index = -1;
        parentBoneIndex = 0;
        childBoneIndex = 0;
        targetBoneIndex = 0;
        type = kUnknown;
        fixedAxis.setZero();
        origin.setZero();
        offset.setZero();
        localTranslation.setZero();
        worldTransform.setIdentity();
        localTransform.setIdentity();
        enableInverseKinematics = false;
    }
    IModel *parentModelRef;
    IEncoding *encodingRef;
    IString *namePtr;
    IString *englishNamePtr;
    IBone *parentBoneRef;
    IBone *targetBoneRef;
    IBone *childBoneRef;
    Vector3 fixedAxis;
    Vector3 origin;
    Vector3 offset;
    Vector3 localTranslation;
    Quaternion rotation;
    Transform worldTransform;
    Transform localTransform;
    Type type;
    int index;
    int parentBoneIndex;
    int targetBoneIndex;
    int childBoneIndex;
    bool enableInverseKinematics;
};

const int Bone::kNameSize = internal::kPMDBoneNameSize;
const int Bone::kCategoryNameSize = internal::kPMDBoneCategoryNameSize;

Bone::Bone(IModel *parentModelRef, IEncoding *encodingRef)
    : m_context(0)
{
    m_context = new PrivateContext(parentModelRef, encodingRef);
}

Bone::~Bone()
{
    delete m_context;
    m_context = 0;
}

bool Bone::preparseBones(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    uint16_t size;
    if (!internal::getTyped<uint16_t>(ptr, rest, size) || size * sizeof(BoneUnit) > rest) {
        return false;
    }
    info.bonesCount = size;
    info.bonesPtr = ptr;
    internal::drainBytes(size * sizeof(BoneUnit), ptr, rest);
    return true;
}

bool Bone::loadBones(const Array<Bone *> &bones)
{
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        const int parentBoneIndex = bone->m_context->parentBoneIndex;
        if (parentBoneIndex >= 0) {
            if (parentBoneIndex >= nbones) {
                return false;
            }
            else {
                Bone *parent = bones[parentBoneIndex];
                bone->m_context->offset -= parent->m_context->origin;
                bone->m_context->parentBoneRef = parent;
            }
        }
        const int targetBoneIndex = bone->m_context->targetBoneIndex;
        if (targetBoneIndex >= 0) {
            if (targetBoneIndex >= nbones) {
                return false;
            }
            else {
                bone->m_context->targetBoneRef = bones[targetBoneIndex];
            }
        }
        const int childBoneIndex = bone->m_context->childBoneIndex;
        if (childBoneIndex >= 0) {
            if (childBoneIndex >= nbones) {
                return false;
            }
            else {
                bone->m_context->childBoneRef = bones[childBoneIndex];
            }
        }
        bone->setIndex(i);
    }
    return true;
}

void Bone::writeBones(const Array<Bone *> &bones, const Model::DataInfo &info, uint8_t *&data)
{
    const int nbones = bones.count();
    internal::writeUnsignedIndex(nbones, sizeof(uint16_t), data);
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        bone->write(data, info);
    }
}

void Bone::writeEnglishNames(const Array<Bone *> &bones, const Model::DataInfo &info, uint8_t *&data)
{
    const IEncoding *encodingRef = info.encoding;
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones[i];
        internal::writeStringAsByteArray(bone->englishName(), IString::kShiftJIS, encodingRef, kNameSize, data);
    }
}

size_t Bone::estimateTotalSize(const Array<Bone *> &bones, const Model::DataInfo & /* info */)
{
    const int nbones = bones.count();
    size_t size = sizeof(uint16_t);
    size += sizeof(BoneUnit) * nbones;
    return size;
}

void Bone::readBone(const uint8_t *data, const Model::DataInfo & /* info */, size_t &size)
{
    BoneUnit unit;
    internal::getData(data, unit);
    internal::setStringDirect(m_context->encodingRef->toString(unit.name, IString::kShiftJIS, kNameSize), m_context->namePtr);
    m_context->childBoneIndex = unit.childBoneID;
    m_context->parentBoneIndex = unit.parentBoneID;
    m_context->targetBoneIndex = unit.targetBoneID;
    m_context->type = static_cast<Type>(unit.type);
    internal::setPosition(unit.position, m_context->origin);
    m_context->offset = m_context->origin;
    size = sizeof(unit);
}

void Bone::readEnglishName(const uint8_t *data, int index)
{
    if (data && index >= 0) {
        internal::setStringDirect(m_context->encodingRef->toString(data + kNameSize * index, IString::kShiftJIS, kNameSize), m_context->englishNamePtr);
    }
}

void Bone::write(uint8_t *&data, const Model::DataInfo & /* info */) const
{
    BoneUnit unit;
    unit.childBoneID = m_context->childBoneIndex;
    unit.parentBoneID = m_context->parentBoneIndex;
    uint8_t *namePtr = unit.name;
    internal::writeStringAsByteArray(m_context->namePtr, IString::kShiftJIS, m_context->encodingRef, sizeof(unit.name), namePtr);
    internal::getPosition(m_context->origin, unit.position);
    unit.targetBoneID = m_context->targetBoneIndex;
    unit.type = m_context->type;
    internal::writeBytes(&unit, sizeof(unit), data);
}

void Bone::performTransform()
{
    if (m_context->type == kUnderRotate && m_context->targetBoneRef) {
        const Quaternion &rotation = m_context->rotation * m_context->targetBoneRef->localRotation();
        m_context->worldTransform.setRotation(rotation);
    }
    else if (m_context->type == kFollowRotate && m_context->childBoneRef) {
        const Scalar coef(m_context->targetBoneIndex * 0.01f);
        const Quaternion &rotation = Quaternion::getIdentity().slerp(m_context->rotation, coef);
        m_context->worldTransform.setRotation(rotation);
    }
    else {
        m_context->worldTransform.setRotation(m_context->rotation);
    }
    m_context->worldTransform.setOrigin(m_context->offset + m_context->localTranslation);
    if (m_context->parentBoneRef) {
        m_context->worldTransform = m_context->parentBoneRef->worldTransform() * m_context->worldTransform;
    }
    getLocalTransform(m_context->localTransform);
}

const IString *Bone::name() const
{
    return m_context->namePtr;
}

const IString *Bone::englishName() const
{
    return m_context->englishNamePtr;
}

int Bone::index() const
{
    return m_context->index;
}

IModel *Bone::parentModelRef() const
{
    return m_context->parentModelRef;
}

IBone *Bone::parentBoneRef() const
{
    return m_context->parentBoneRef;
}

IBone *Bone::effectorBoneRef() const
{
    return m_context->targetBoneRef;
}

Transform Bone::worldTransform() const
{
    return m_context->worldTransform;
}

Transform Bone::localTransform() const
{
    return m_context->localTransform;
}

void Bone::getLocalTransform(Transform &world2LocalTransform) const
{
    getLocalTransform(m_context->worldTransform, world2LocalTransform);
}

void Bone::getLocalTransform(const Transform &worldTransform, Transform &output) const
{
    output = worldTransform * Transform(Matrix3x3::getIdentity(), -m_context->origin);
}

void Bone::setLocalTransform(const Transform &value)
{
    m_context->localTransform = value;
}

Vector3 Bone::origin() const
{
    return m_context->origin;
}

Vector3 Bone::destinationOrigin() const
{
    return m_context->parentBoneRef ? m_context->parentBoneRef->origin() : kZeroV3;
}

Vector3 Bone::localTranslation() const
{
    return m_context->localTranslation;
}

Quaternion Bone::localRotation() const
{
    return m_context->rotation;
}

void Bone::getEffectorBones(Array<IBone *> & /* value */) const
{
}

void Bone::setLocalTranslation(const Vector3 &value)
{
    m_context->localTranslation = value;
}

void Bone::setLocalRotation(const Quaternion &value)
{
    m_context->rotation = value;
}

void Bone::setTargetBoneRef(IBone *value)
{
    m_context->targetBoneRef = value;
}

bool Bone::isMovable() const
{
    switch (m_context->type) {
    case kRotateAndMove:
    case kIKRoot:
    case kIKJoint:
        return true;
        case kRotate:
    case kUnknown:
    case kUnderRotate:
    case kIKEffector:
    case kInvisible:
    case kTwist:
    case kFollowRotate:
    default:
        return false;
    }
}

bool Bone::isRotateable() const
{
    switch (m_context->type) {
    case kUnknown:
    case kIKEffector:
    case kInvisible:
    case kFollowRotate:
        return false;
    case kRotate:
    case kRotateAndMove:
    case kIKRoot:
    case kIKJoint:
    case kUnderRotate:
    case kTwist:
    default:
        return true;
    }
}

bool Bone::isVisible() const
{
    return isRotateable();
}

bool Bone::isInteractive() const
{
    return isRotateable();
}

bool Bone::hasInverseKinematics() const
{
    return m_context->type == kIKRoot;
}

bool Bone::hasFixedAxes() const
{
    return m_context->type == kTwist;
}

bool Bone::hasLocalAxes() const
{
    if (m_context->encodingRef && m_context->namePtr) {
        bool hasFinger = m_context->namePtr->contains(m_context->encodingRef->stringConstant(IEncoding::kFinger));
        bool hasArm = m_context->namePtr->endsWith(m_context->encodingRef->stringConstant(IEncoding::kArm));
        bool hasElbow = m_context->namePtr->endsWith(m_context->encodingRef->stringConstant(IEncoding::kElbow));
        bool hasWrist = m_context->namePtr->endsWith(m_context->encodingRef->stringConstant(IEncoding::kWrist));
        return hasFinger || hasArm || hasElbow || hasWrist;
    }
    return false;
}

Vector3 Bone::fixedAxis() const
{
    return m_context->fixedAxis;
}

void Bone::getLocalAxes(Matrix3x3 &value) const
{
    if (hasLocalAxes()) {
        const Vector3 &axisX = (m_context->childBoneRef->origin() - origin()).normalized();
        Vector3 tmp1 = axisX;
        if (m_context->namePtr->startsWith(m_context->encodingRef->stringConstant(IEncoding::kLeft)))
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
    if (m_context->encodingRef && m_context->namePtr) {
        bool isRightKnee = m_context->namePtr->equals(m_context->encodingRef->stringConstant(IEncoding::kRightKnee));
        bool isLeftKnee = m_context->namePtr->equals(m_context->encodingRef->stringConstant(IEncoding::kLeftKnee));
        return isRightKnee || isLeftKnee;
    }
    return false;
}

bool Bone::isInverseKinematicsEnabled() const
{
    return m_context->enableInverseKinematics;
}

void Bone::setInverseKinematicsEnable(bool value)
{
    m_context->enableInverseKinematics = value;
}

void Bone::setIndex(int value)
{
    m_context->index = value;
}

} /* namespace pmd2 */
} /* namespace vpvl2 */
