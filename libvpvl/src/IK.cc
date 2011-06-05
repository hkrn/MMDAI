/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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
size_t IK::totalSize(const uint8_t *data, size_t n)
{
    size_t size = 0;
    uint8_t *ptr = const_cast<uint8_t *>(data);
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

size_t IK::stride(const uint8_t *data)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    size_t base = sizeof(int16_t) * 2;
    ptr += base;
    uint8_t nlinks = *reinterpret_cast<uint8_t *>(ptr);
    return base + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(float) + nlinks * sizeof(int16_t);
}

void IK::read(const uint8_t *data, BoneList *bones)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
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
    const btVector3 xAxis(1.0f, 0.0f, 0.0f);
    int nbones = m_bones.size();
    for (int i = nbones - 1; i >= 0; i--)
        m_bones[i]->updateTransform();
    m_target->updateTransform();
    const btQuaternion originTargetRotation = m_target->currentRotation();
    btQuaternion q;
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
            if (fabsf(angle) < kMinAngle)
                continue;
            btClamp(angle, -m_angleConstraint, m_angleConstraint);
            btVector3 axis = localTarget.cross(localDestination);
            if (axis.length2() < kMinAxis && i > 0)
                continue;
            axis.normalize();
            q.setRotation(axis, angle);
            if (bone->isAngleXLimited()) {
                if (i == 0) {
                    q.setRotation(xAxis, fabsf(angle));
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
                    if (fabsf(x) < kMinRotation)
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
