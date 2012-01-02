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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"

namespace
{
const float kMinDistance    = 0.0001f;
const float kMinAngle       = 0.00000001f;
const float kMinAxis        = 0.0000001f;
const float kMinRotationSum = 0.002f;
const float kMinRotation    = 0.00001f;
}

namespace vpvl
{

#pragma pack(push, 1)

struct IKChunk
{
    int16_t destBoneID;
    int16_t targetBoneID;
    uint8_t nlinks;
    uint16_t niterations;
    float angleConstraint;
};

#pragma pack(pop)

size_t IK::totalSize(const uint8_t *data, size_t rest, size_t count, bool &ok)
{
    size_t size = 0;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    for (size_t i = 0; i < count; i++) {
        if (sizeof(IKChunk) > rest) {
            ok = false;
            return 0;
        }
        size_t required = stride(ptr);
        if (required > rest) {
            ok = false;
            return 0;
        }
        rest -= required;
        size += required;
        ptr += required;
    }
    ok = true;
    return size;
}

size_t IK::stride(const uint8_t *data)
{
    const IKChunk *chunk = reinterpret_cast<const IKChunk *>(data);
    return sizeof(*chunk) + chunk->nlinks * sizeof(int16_t);
}

IK::IK()
    : m_destination(0),
      m_target(0),
      m_iteration(0),
      m_angleConstraint(0.0f),
      m_rawAngleConstraint(0.0f)
{
}

IK::~IK()
{
    m_destination = 0;
    m_target = 0;
    m_iteration = 0;
    m_angleConstraint = 0.0f;
    m_rawAngleConstraint = 0.0f;
}

void IK::read(const uint8_t *data, BoneList *bones)
{
    IKChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    int16_t destBoneID = chunk.destBoneID;
    int16_t targetBoneID = chunk.targetBoneID;
    uint8_t nlinks = chunk.nlinks;
    uint16_t niterations = chunk.niterations;

#ifdef VPVL_BUILD_IOS
    float angleConstraint;
    memcpy(&angleConstraint, &chunk.angleConstraint, sizeof(angleConstraint));
#else
    float angleConstraint = chunk.angleConstraint;
#endif

    Array<int16_t> boneIKs;
    int nbones = bones->count();
    uint8_t *ptr = const_cast<uint8_t *>(data + sizeof(chunk));
    for (int i = 0; i < nlinks; i++) {
        int16_t boneID = *reinterpret_cast<int16_t *>(ptr);
        if (boneID >= 0 && boneID < nbones)
            boneIKs.add(boneID);
        ptr += sizeof(int16_t);
    }

    if (destBoneID >= 0 && destBoneID < nbones && targetBoneID >= 0 && targetBoneID < nbones) {
        BoneList boneIKs2;
        nlinks = boneIKs.count();
        boneIKs2.reserve(nlinks);
        for (int i = 0; i < nlinks; i++) {
            Bone *bone = bones->at(boneIKs[i]);
            boneIKs2.add(bone);
        }
        setAngleConstraint(angleConstraint);
        setIterationSize(niterations);
        setBones(bones->at(destBoneID), bones->at(targetBoneID), boneIKs2);
    }
}

size_t IK::estimateSize() const
{
    return sizeof(IKChunk) + m_bones.count() * sizeof(int16_t);
}

void IK::write(uint8_t *data) const
{
    IKChunk chunk;
    int nlinks = m_bones.count();
    chunk.destBoneID = m_destination->id();
    chunk.targetBoneID = m_target->id();
    chunk.nlinks = nlinks;
    chunk.niterations = m_iteration;
    chunk.angleConstraint = m_rawAngleConstraint;
    uint8_t *ptr = data;
    internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
    ptr += sizeof(chunk);
    if (nlinks > 0) {
        for (int i = 0; i < nlinks; i++) {
            int16_t boneID = m_bones.at(i)->id();
            internal::copyBytes(ptr, reinterpret_cast<const uint8_t *>(&boneID), sizeof(boneID));
            ptr += sizeof(boneID);
        }
    }
}

void IK::solve()
{
    const Vector3 destPosition = m_destination->localTransform().getOrigin();
    const Vector3 xAxis(1.0f, 0.0f, 0.0f);
    int nbones = m_bones.count();
    for (int i = nbones - 1; i >= 0; i--)
        m_bones[i]->updateTransform();
    m_target->updateTransform();
    const Quaternion originTargetRotation = m_target->rotation();
    Quaternion q;
    for (int i = 0; i < m_iteration; i++) {
        for (int j = 0; j < nbones; j++) {
            Bone *bone = m_bones[j];
            const Vector3 &targetPosition = m_target->localTransform().getOrigin();
            const Transform &transform = bone->localTransform().inverse();
            Vector3 localDestination = transform * destPosition;
            Vector3 localTarget = transform * targetPosition;
            if (localDestination.distance2(localTarget) < kMinDistance) {
                i = m_iteration;
                break;
            }
            localDestination.normalize();
            localTarget.normalize();
            const Scalar &dot = localDestination.dot(localTarget);
            if (dot > 1.0f)
                continue;
            Scalar angle = btAcos(dot);
            if (btFabs(angle) < kMinAngle)
                continue;
            btClamp(angle, -m_angleConstraint, m_angleConstraint);
            Vector3 axis = localTarget.cross(localDestination);
            if (axis.length2() < kMinAxis && i > 0)
                continue;
            axis.normalize();
            q.setRotation(axis, angle);
            if (bone->isConstraintedXCoordinateForIK()) {
                if (i == 0) {
                    q.setRotation(xAxis, btFabs(angle));
                }
                else {
                    Scalar x, y, z, cx, cy, cz;
                    btMatrix3x3 matrix;
                    matrix.setRotation(q);
                    matrix.getEulerZYX(z, y, x);
                    matrix.setRotation(bone->rotation());
                    matrix.getEulerZYX(cz, cy, cx);
                    if (x + cx > vpvl::kPI)
                        x = vpvl::kPI - cx;
                    if (kMinRotationSum > x + cx)
                        x = kMinRotationSum - cx;
                    btClamp(x, -m_angleConstraint, m_angleConstraint);
                    if (btFabs(x) < kMinRotation)
                        continue;
                    q.setEulerZYX(0.0f, 0.0f, x);
                }
                bone->setRotation(q * bone->rotation());
            }
            else {
                Quaternion tmp = bone->rotation();
                tmp *= q;
                bone->setRotation(tmp);
            }
            for (int k = j; k >= 0; k--)
                m_bones[k]->updateTransform();
            m_target->updateTransform();
        }
    }
    m_target->setRotation(originTargetRotation);
    m_target->updateTransform();
}

void IK::setBones(Bone *destination, Bone *target, const BoneList &boneIKs)
{
    m_destination = destination;
    m_target = target;
    m_bones.clear();
    m_bones.copy(boneIKs);
}

void IK::setAngleConstraint(float value)
{
    m_rawAngleConstraint = value;
    m_angleConstraint = value * vpvl::kPI;
}

void IK::setIterationSize(uint16_t value)
{
    m_iteration = value;
}

}
