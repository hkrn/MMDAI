#include "vpvl/vpvl.h"

namespace vpvl
{

const float IK::kPi             = 3.1415926f;
const float IK::kMinDistance    = 0.0001f;
const float IK::kMinAngle       = 0.00000001f;
const float IK::kMinAxis        = 0.0000001f;
const float IK::kMinRotationSum = 0.002f;
const float IK::kMinRotation    = 0.00001f;

IK::IK()
    : m_destination(0),
      m_target(0),
      m_iteration(0),
      m_angleConstraint(0.0f)
{
}

IK::~IK()
{
    m_destination = 0;
    m_target = 0;
    m_iteration = 0;
    m_angleConstraint = 0.0f;
}

// FIXME: boundary check
size_t IK::totalSize(const char *data, size_t n)
{
    size_t size = 0;
    char *ptr = const_cast<char *>(data);
    for (size_t i = 0; i < n; i++) {
        size_t base = sizeof(int16_t) * 2;
        ptr += base;
        uint8_t nlinks = *reinterpret_cast<uint8_t *>(ptr);
        size_t rest = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(float) + nlinks * sizeof(int16_t);
        size += base + rest;
        ptr += rest;
    }
    return size;
}

size_t IK::stride(const char *data)
{
    char *ptr = const_cast<char *>(data);
    size_t base = sizeof(int16_t) * 2;
    ptr += base;
    uint8_t nlinks = *reinterpret_cast<uint8_t *>(ptr);
    return base + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(float) + nlinks * sizeof(int16_t);
}

void IK::read(const char *data, BoneList *bones)
{
    char *ptr = const_cast<char *>(data);
    int16_t destBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    int16_t targetBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    uint8_t nlinks = *reinterpret_cast<uint8_t *>(ptr);
    ptr += sizeof(uint8_t);
    uint16_t niterations = *reinterpret_cast<uint16_t *>(ptr);
    ptr += sizeof(uint16_t);
    float angleConstraint = *reinterpret_cast<float *>(ptr);
    ptr += sizeof(float);

    btAlignedObjectArray<int16_t> boneIKs;
    int nbones = bones->size();
    for (int i = 0; i < nlinks; i++) {
        int16_t boneID = *reinterpret_cast<int16_t *>(ptr);
        if (boneID >= 0 && boneID < nbones) {
            boneIKs.push_back(boneID);
            ptr += sizeof(int16_t);
        }
    }

    if (destBoneID >= 0 && destBoneID < nbones && targetBoneID >= 0 && targetBoneID < nbones) {
        nlinks = boneIKs.size();
        m_destination = bones->at(destBoneID);
        m_target = bones->at(targetBoneID);
        m_iteration = niterations;
        m_angleConstraint = angleConstraint * IK::kPi;
        m_bones.reserve(nlinks);
        for (int i = 0; i < nlinks; i++) {
            Bone *bone = bones->at(boneIKs[i]);
            m_bones.push_back(bone);
        }
    }
}

void IK::solve()
{
    const btVector3 destPosition = m_destination->currentTransform().getOrigin();
    int nbones = m_bones.size();
    for (int i = 0; i < nbones; i++)
        m_bones[i]->updateTransform();
    m_target->updateTransform();
    const btQuaternion originTargetRotation = m_target->currentRotation();
    for (int i = 0; i < m_iteration; i++) {
        for (int j = 0; j < nbones; j++) {
            Bone *bone = m_bones[j];
            const btVector3 targetPosition = m_target->currentTransform().getOrigin();
            const btTransform transform = bone->currentTransform().inverse();
            btVector3 localDestination = transform * destPosition;
            btVector3 localTarget = transform * targetPosition;
            if (localDestination.distance2(localTarget) < kMinDistance) {
                i = m_iteration;
                break;
            }
            localDestination.normalize();
            localTarget.normalize();
            const float dot = localDestination.dot(localTarget);
            if (dot > 1.0f)
                continue;
            float angle = acosf(dot);
            if (fabs(angle) < kMinAngle)
                continue;
            btClamp(angle, -m_angleConstraint, m_angleConstraint);
            btVector3 axis = localTarget.cross(localDestination);
            if (axis.length2() < kMinAxis && i > 0)
                continue;
            axis.normalize();
            btQuaternion q(axis, angle);
            if (bone->isAngleXLimited()) {
                if (i == 0) {
                    if (angle < 0.0f)
                        angle = -angle;
                    q = btQuaternion(btVector3(1.0f, 0.0f, 0.0f), angle);
                }
                else {
                    btScalar x, y, z, cx, cy, cz;
                    btMatrix3x3 matrix;
                    matrix.setRotation(q);
                    matrix.getEulerZYX(z, y, x);
                    matrix.setRotation(bone->currentRotation());
                    matrix.getEulerZYX(cz, cy, cx);
                    if (x + cx > kPi)
                        x = kPi - cx;
                    if (kMinRotationSum > x + cx)
                        x = kMinRotationSum - cx;
                    btClamp(x, -m_angleConstraint, m_angleConstraint);
                    if (fabs(x) < kMinRotation)
                        continue;
                    q.setEulerZYX(0.0f, 0.0f, x);
                }
                bone->setCurrentRotation(q * bone->currentRotation());
            }
            else {
                btQuaternion tmp = bone->currentRotation();
                tmp *= q;
                bone->setCurrentRotation(tmp);
            }
            for (int k = j; k >= 0; k--)
                m_bones[k]->updateTransform();
            m_target->updateTransform();
        }
    }
    m_target->setCurrentRotation(originTargetRotation);
    m_target->updateTransform();
}

}
