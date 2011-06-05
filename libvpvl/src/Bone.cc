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
#include "vpvl/internal/util.h"

namespace vpvl
{

Bone::Bone()
    : m_type(kUnknown),
      m_originPosition(0.0f, 0.0f, 0.0f),
      m_currentPosition(0.0f, 0.0f, 0.0f),
      m_offset(0.0f, 0.0f, 0.0f),
      m_currentRotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_rotateCoef(0.0f),
      m_parentBone(0),
      m_childBone(0),
      m_targetBone(0),
      m_parentIsRoot(false),
      m_angleXLimited(false),
      m_simulated(false),
      m_motionIndepent(false)
{
    memset(m_name, 0, sizeof(m_name));
    m_currentTransform.setIdentity();
    m_transformMoveToOrigin.setIdentity();
}

Bone::~Bone()
{
    memset(m_name, 0, sizeof(m_name));
    m_type = kUnknown;
    m_currentTransform.setIdentity();
    m_transformMoveToOrigin.setIdentity();
    m_originPosition.setZero();
    m_currentPosition.setZero();
    m_offset.setZero();
    m_currentRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_rotateCoef = 0.0f;
    m_parentBone = 0;
    m_childBone = 0;
    m_targetBone = 0;
    m_parentIsRoot = false;
    m_angleXLimited = false;
    m_simulated = false;
    m_motionIndepent = false;
}

Bone *Bone::centerBone(const btAlignedObjectArray<Bone*> *bones)
{
    static const uint8_t centerName[] = { 0x83, 0x5a, 0x83, 0x93, 0x83, 0x5e, 0x81, 0x5b, 0x0 };
    int nbones = bones->size();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones->at(i);
        if (internal::stringEquals(bone->name(), centerName, sizeof(centerName)))
            return bone;
    }
    return bones->at(0);
}

size_t Bone::stride(const uint8_t * /* data */)
{
    return 20 + (sizeof(int16_t) * 3)+ sizeof(uint8_t) + (sizeof(float) * 3);
}

void Bone::read(const uint8_t *data, btAlignedObjectArray<Bone*> *bones, Bone *rootBone)
{
    uint8_t *ptr = const_cast<uint8_t *>(data);
    copyBytesSafe(m_name, ptr, sizeof(m_name));
    ptr += sizeof(m_name);
    int16_t parentBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    int16_t childBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    BoneType type = static_cast<BoneType>(*reinterpret_cast<uint8_t *>(ptr));
    ptr += sizeof(uint8_t);
    int16_t targetBoneID = *reinterpret_cast<int16_t *>(ptr);
    ptr += sizeof(int16_t);
    float pos[3];
    internal::vector3(ptr, pos);

    static const uint8_t kneeName[] = { 0x82, 0xd0, 0x82, 0xb4, 0x0 };
    if (strstr(reinterpret_cast<const char *>(m_name), reinterpret_cast<const char *>(kneeName)))
        m_angleXLimited = true;

    int nbones = bones->size();
    if (parentBoneID != -1 && parentBoneID < nbones) {
        m_parentBone = bones->at(parentBoneID);
        m_parentIsRoot = false;
    }
    else if (nbones >= 0) {
        m_parentBone = rootBone;
        m_parentIsRoot = true;
    }
    else {
        m_parentIsRoot = false;
    }

    if (childBoneID != -1 && childBoneID < nbones)
        m_childBone = bones->at(childBoneID);

    if ((type == kUnderIK || m_type == kUnderRotate) && targetBoneID > 0 && targetBoneID < nbones)
        m_targetBone = bones->at(targetBoneID);
    else if (type == kFollowRotate)
        m_rotateCoef = targetBoneID * 0.01f;

#ifdef VPVL_COORDINATE_OPENGL
    m_originPosition.setValue(pos[0], pos[1], -pos[2]);
#else
    m_originPosition.setValue(pos[0], pos[1], pos[2]);
#endif
    m_currentTransform.setOrigin(m_originPosition);
    m_transformMoveToOrigin.setOrigin(-m_originPosition);
}

void Bone::computeOffset()
{
    m_offset = m_parentBone ? m_originPosition - m_parentBone->m_originPosition : m_originPosition;
}

void Bone::reset()
{
    m_currentPosition.setZero();
    m_currentRotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_currentTransform.setIdentity();
    m_currentTransform.setOrigin(m_originPosition);
}

void Bone::setMotionIndependency()
{
}

void Bone::updateRotation()
{
    btQuaternion q;
    switch (m_type) {
    case kUnderRotate:
        q = m_currentRotation * m_targetBone->m_currentRotation;
        updateTransform(q);
        break;
    case kFollowRotate:
        q = m_currentRotation * internal::kZeroQ.slerp(m_childBone->m_currentRotation, m_rotateCoef);
        updateTransform(q);
        break;
    default:
        break;
    }
}

void Bone::updateTransform()
{
    updateTransform(m_currentRotation);
}

void Bone::updateTransform(const btQuaternion &q)
{
    m_currentTransform.setOrigin(m_currentPosition + m_offset);
    m_currentTransform.setRotation(q);
    if (m_parentBone)
        m_currentTransform = m_parentBone->m_currentTransform * m_currentTransform;
}

void Bone::getSkinTransform(btTransform &tr)
{
    tr = m_currentTransform * m_transformMoveToOrigin;
}

} /* namespace vpvl */
