/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
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

#pragma pack(push, 1)

struct BoneChunk
{
    uint8_t name[Bone::kNameSize];
    int16_t parentBoneID;
    int16_t childBoneID;
    uint8_t type;
    int16_t targetBoneID;
    float position[3];
};

#pragma pack(pop)

const uint8_t *Bone::centerBoneName()
{
    static const uint8_t centerBoneName[] = { 0x83, 0x5a, 0x83, 0x93, 0x83, 0x5e, 0x81, 0x5b, 0x0 };
    return centerBoneName;
}

Bone *Bone::centerBone(const BoneList *bones)
{
    const uint8_t *name = centerBoneName();
    size_t len = strlen(reinterpret_cast<const char *>(name));
    int nbones = bones->count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = bones->at(i);
        if (internal::stringEquals(bone->name(), name, len))
            return bone;
    }
    return bones->at(0);
}

size_t Bone::stride()
{
    return sizeof(BoneChunk);
}

Bone::Bone()
    : m_id(-1),
      m_type(kUnknown),
      m_originPosition(0.0f, 0.0f, 0.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_offset(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_rotateCoef(0.0f),
      m_parentBone(0),
      m_childBone(0),
      m_targetBone(0),
      m_parentBoneID(-1),
      m_childBoneID(-1),
      m_targetBoneID(-1),
      m_hasParent(false),
      m_parentIsRoot(false),
      m_constraintedXCoordinateForIK(false),
      m_simulated(false),
      m_motionIndepent(false)
{
    internal::zerofill(m_name, sizeof(m_name));
    m_localTransform.setIdentity();
    m_localToOriginTransform.setIdentity();
}

Bone::~Bone()
{
    internal::zerofill(m_name, sizeof(m_name));
    m_id = -1;
    m_type = kUnknown;
    m_localTransform.setIdentity();
    m_localToOriginTransform.setIdentity();
    m_originPosition.setZero();
    m_position.setZero();
    m_offset.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_rotateCoef = 0.0f;
    m_parentBone = 0;
    m_childBone = 0;
    m_targetBone = 0;
    m_parentBoneID = -1;
    m_childBoneID = -1;
    m_targetBoneID = -1;
    m_parentIsRoot = false;
    m_constraintedXCoordinateForIK = false;
    m_simulated = false;
    m_motionIndepent = false;
}

void Bone::read(const uint8_t *data, int16_t id)
{
    BoneChunk chunk;
    internal::copyBytes(reinterpret_cast<uint8_t *>(&chunk), data, sizeof(chunk));
    copyBytesSafe(m_name, chunk.name, sizeof(m_name));
    Type type = static_cast<Type>(chunk.type);
    int16_t targetBoneID = chunk.targetBoneID;

#ifdef VPVL_BUILD_IOS
    float pos[3];
    memcpy(pos, &chunk.position, sizeof(pos));
#else
    float *pos = chunk.position;
#endif

    // Knee bone treats as a special bone to constraint X for IK
    static const uint8_t kneeName[] = { 0x82, 0xd0, 0x82, 0xb4, 0x0 };
    if (strstr(reinterpret_cast<const char *>(m_name), reinterpret_cast<const char *>(kneeName)))
        m_constraintedXCoordinateForIK = true;
    if (type == kFollowRotate)
        m_rotateCoef = targetBoneID * 0.01f;

#ifdef VPVL_COORDINATE_OPENGL
    m_originPosition.setValue(pos[0], pos[1], -pos[2]);
#else
    m_originPosition.setValue(pos[0], pos[1], pos[2]);
#endif
    m_localTransform.setOrigin(m_originPosition);
    m_localToOriginTransform.setOrigin(-m_originPosition);
    m_id = id;
    m_parentBoneID = chunk.parentBoneID;
    m_childBoneID = chunk.childBoneID;
    m_targetBoneID = targetBoneID;
    m_type = type;
}

void Bone::write(uint8_t *data) const
{
    BoneChunk chunk;
    copyBytesSafe(chunk.name, m_name, sizeof(chunk.name));
    chunk.parentBoneID = m_parentBoneID;
    chunk.childBoneID = m_childBoneID;
    chunk.type = m_type;
    chunk.targetBoneID = m_targetBoneID;
    chunk.position[0] = m_position.x();
    chunk.position[1] = m_position.y();
    chunk.position[2] = m_position.z();
    internal::copyBytes(data, reinterpret_cast<const uint8_t *>(&chunk), sizeof(chunk));
}

void Bone::reset()
{
    m_position.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_localTransform.setIdentity();
    m_localTransform.setOrigin(m_originPosition);
}

void Bone::build(BoneList *bones, Bone *rootBone)
{
    const int nbones = bones->count();
    // The bone has a parent bone and in the the current bones
    if (m_parentBoneID != -1 && m_parentBoneID < nbones) {
        m_parentBone = bones->at(m_parentBoneID);
        m_parentIsRoot = false;
        m_hasParent = true;
    }
    // The bone has no parent bone but bones found.
    else if (nbones >= 0) {
        m_parentBone = rootBone;
        m_parentIsRoot = true;
        m_hasParent = false;
    }
    // The bone has no parent bone and no bones found.
    // e.g. The "Center" bone
    else {
        m_parentIsRoot = false;
        m_hasParent = false;
    }
    // The bone has a child bone and in the current bones
    if (m_childBoneID != -1 && m_childBoneID < nbones)
        m_childBone = bones->at(m_childBoneID);
    // The bone has a target bone and in the current bones for IK
    if (m_type == kUnderIK || m_type == kUnderRotate) {
        if (m_targetBoneID >= 0 && m_targetBoneID < bones->count())
            m_targetBone = bones->at(m_targetBoneID);
    }
    m_offset = m_parentBone ? m_originPosition - m_parentBone->m_originPosition : m_originPosition;
    // The parent is root bone
    if (!m_parentBone || m_parentIsRoot) {
        m_motionIndepent = true;
        return;
    }
    // check if the bone is a special root bone
    static const uint8_t allParent[] = { 0x91, 0x53, 0x82, 0xc4, 0x82, 0xcc, 0x90, 0x65, 0x0 };
    static const uint8_t legsOffset[] = { 0x97, 0xbc, 0x91, 0xab, 0x83, 0x49, 0x83, 0x74, 0x83, 0x5a, 0x0 };
    static const uint8_t rightLegOffset[] = { 0x89, 0x45, 0x91, 0xab, 0x83, 0x49, 0x83, 0x74, 0x83, 0x5a, 0x0 };
    static const uint8_t leftLegOffset[] = { 0x8d, 0xb6, 0x91, 0xab, 0x83, 0x49, 0x83, 0x74, 0x83, 0x5a, 0x0 };
    if (internal::stringEquals(m_name, allParent, sizeof(allParent)) ||
            internal::stringEquals(m_name, legsOffset, sizeof(legsOffset)) ||
            internal::stringEquals(m_name, rightLegOffset, sizeof(rightLegOffset)) ||
            internal::stringEquals(m_name, leftLegOffset, sizeof(leftLegOffset))) {
        m_motionIndepent = true;
        return;
    }
    m_motionIndepent = false;
}

void Bone::updateRotation()
{
    Quaternion q;
    switch (m_type) {
    case kUnderRotate:
        q = m_rotation * m_targetBone->m_rotation;
        updateTransform(q);
        break;
    case kFollowRotate:
        q = m_rotation * internal::kZeroQ.slerp(m_childBone->m_rotation, m_rotateCoef);
        updateTransform(q);
        break;
    default:
        break;
    }
}

void Bone::updateTransform()
{
    updateTransform(m_rotation);
}

void Bone::updateTransform(const Quaternion &q)
{
    m_localTransform.setOrigin(m_position + m_offset);
    m_localTransform.setRotation(q);
    if (m_parentBone)
        m_localTransform = m_parentBone->m_localTransform * m_localTransform;
}

void Bone::getSkinTransform(Transform &tr) const
{
    tr = m_localTransform * m_localToOriginTransform;
}

} /* namespace vpvl */
