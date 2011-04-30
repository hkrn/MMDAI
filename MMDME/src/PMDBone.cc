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

/* headers */

#include "MMDME/MMDME.h"

namespace MMDAI {

PMDBone::PMDBone()
    : m_name(NULL),
    m_parentBone(NULL),
    m_childBone(NULL),
    m_type(UNKNOWN),
    m_targetBone(NULL),
    m_rotateCoef(0.0f),
    m_parentIsRoot(false),
    m_limitAngleX(false),
    m_motionIndependent(false),
    m_simulated(false),
    m_rot(0.0f, 0.0f, 0.0f, 1.0f)

{
    m_originPosition.setZero();
    m_offset.setZero();
    m_trans.setIdentity();
    m_trans.setOrigin(m_originPosition);
    m_transMoveToOrigin.setIdentity();
    m_transMoveToOrigin.setOrigin(-m_originPosition);
    m_pos.setZero();
}

PMDBone::~PMDBone()
{
    release();
}

void PMDBone::release()
{
    MMDAIMemoryRelease(m_name);
    m_name = NULL;
    m_parentBone = NULL;
    m_childBone = NULL;
    m_type = UNKNOWN;
    m_targetBone = NULL;
    m_rotateCoef = 0.0f;
    m_parentIsRoot = false;
    m_limitAngleX = false;
    m_motionIndependent = false;
    m_simulated = false;
    m_rot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    m_originPosition.setZero();
    m_offset.setZero();
    m_trans.setIdentity();
    m_trans.setOrigin(m_originPosition);
    m_transMoveToOrigin.setIdentity();
    m_transMoveToOrigin.setOrigin(-m_originPosition);
    m_pos.setZero();
}

bool PMDBone::setup(const PMDFile_Bone *b, PMDBone *boneList, const uint16_t maxBones, PMDBone *rootBone)
{
    bool ret = true;
    char name[21];

    release();

    /* name */
    MMDAIStringCopySafe(name, b->name, sizeof(name));
    m_name = MMDAIStringClone(name);

    /* mark if this bone should be treated as angle-constrained bone in IK process */
    if (strstr(m_name, PMDBONE_KNEENAME))
        m_limitAngleX = true;
    else
        m_limitAngleX = false;

    /* parent bone */
    if (b->parentBoneID != -1) {
        /* has parent bone */
        if (b->parentBoneID >= maxBones) {
            ret = false;
        } else {
            m_parentBone = &(boneList[b->parentBoneID]);
            m_parentIsRoot = false;
        }
    } else {
        /* no parent bone */
        if (rootBone) {
            /* set model root bone as parent */
            m_parentBone = rootBone;
            m_parentIsRoot = true;
        } else {
            /* no parent, just use it */
            m_parentIsRoot = false;
        }
    }

    /* child bone */
    if (b->childBoneID != -1) {
        if (b->childBoneID >= maxBones)
            ret = false;
        else
            m_childBone = &(boneList[b->childBoneID]);
    }

    /* type */
    m_type = b->type;

    /* target bone to which this bone is subject to */
    if (m_type == UNDER_IK || m_type == UNDER_ROTATE) {
        m_targetBone = &(boneList[b->targetBoneID]);
        if (b->targetBoneID >= maxBones)
            ret = false;
        else
            m_targetBone = &(boneList[b->targetBoneID]);
    }

    /* store the value of targetBoneID as co-rotate coef if kind == FOLLOW_ROTATE */
    if (m_type == FOLLOW_ROTATE)
        m_rotateCoef = (float) b->targetBoneID * 0.01f;

    /* store absolute bone positions */
    /* reverse Z value on bone position */
#ifdef MMDFILES_CONVERTCOORDINATESYSTEM
    m_originPosition = btVector3(b->pos[0], b->pos[1], -b->pos[2]);
#else
    m_originPosition = btVector3(b->pos[0], b->pos[1], b->pos[2]);
#endif
    MMDAILogDebugSJIS("name=\"%s\", parentID=%d, childID=%d, type=%d, position=(%.2f, %.2f, %.2f)",
                      m_name, b->parentBoneID, b->childBoneID, m_type, b->pos[0], b->pos[1], b->pos[2]);

    /* reset current transform values */
    m_trans.setOrigin(m_originPosition);

    /* set absolute position->origin transform matrix for skinning */
    m_transMoveToOrigin.setOrigin(-m_originPosition);

    return ret;
}

void PMDBone::computeOffset()
{
    if (m_parentBone)
        m_offset = m_originPosition - m_parentBone->m_originPosition;
    else
        m_offset = m_originPosition;
}

void PMDBone::reset()
{
    m_pos.setZero();
    m_rot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
    /* default transform will be referred while loading rigid bodies in PMD... */
    m_trans.setIdentity();
    m_trans.setOrigin(m_originPosition);
}

void PMDBone::setMotionIndependency()
{
    const char *names[] = {PMDBONE_ADDITIONALROOTNAME};

    if (! m_parentBone || m_parentIsRoot) {
        /* if no parent bone in the model, return true */
        m_motionIndependent = true;
        return;
    }

    /* some models has additional model root bone or offset bones, they should be treated specially */
    for (int i = 0; i < PMDBONE_NADDITIONALROOTNAME; i++) {
        if (MMDAIStringEquals(m_parentBone->m_name, names[i])) {
            m_motionIndependent = true;
            return;
        }
    }

    m_motionIndependent = false;
}

void PMDBone::updateRotate()
{
    btQuaternion r;
    const btQuaternion norot(0.0f, 0.0f, 0.0f, 1.0f);

    if (m_type == UNDER_ROTATE) {
        /* for under-rotate bone, further apply the rotation of target bone */
        r = m_rot * m_targetBone->m_rot;
        m_trans.setOrigin(m_pos + m_offset);
        m_trans.setRotation(r);
        if (m_parentBone)
            m_trans = m_parentBone->m_trans * m_trans;
    } else if (m_type == FOLLOW_ROTATE) {
        /* for co-rotate bone, further apply the rotation of child bone scaled by the rotation weight */
        r = m_rot * norot.slerp(m_childBone->m_rot, m_rotateCoef);
        m_trans.setOrigin(m_pos + m_offset);
        m_trans.setRotation(r);
        if (m_parentBone)
            m_trans = m_parentBone->m_trans * m_trans;
    }
}

void PMDBone::update()
{
    m_trans.setOrigin(m_pos + m_offset);
    m_trans.setRotation(m_rot);
    if (m_parentBone)
        m_trans = m_parentBone->m_trans * m_trans;
}

void PMDBone::calcSkinningTrans(btTransform *b)
{
    *b = m_trans * m_transMoveToOrigin;
}

} /* namespace */

