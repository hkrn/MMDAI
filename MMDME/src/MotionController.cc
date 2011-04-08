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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

const float MotionController::kBoneStartMarginFrame = 20.0f;
const float MotionController::kFaceStartMarginFrame = 6.0f;

void MotionController::calcBoneAt(MotionControllerBoneElement *mc, float frameNow)
{
    BoneMotion *bm = mc->motion;
    float frame = frameNow;
    uint32_t k1 = 0, k2 = 0;
    BoneKeyFrame *keyFrameForInterpolation;

    /* clamp frame to the defined last frame */
    if (frame > bm->keyFrameList[bm->numKeyFrame-1].keyFrame)
        frame = bm->keyFrameList[bm->numKeyFrame-1].keyFrame;

    /* find key frames between which the given frame exists */
    if (frame >= bm->keyFrameList[mc->lastKey].keyFrame) {
        /* start searching from last used key frame */
        for (uint32_t i = mc->lastKey; i < bm->numKeyFrame; i++) {
            if (frame <= bm->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    } else {
        for (uint32_t i = 0; i <= mc->lastKey && i < bm->numKeyFrame; i++) {
            if (frame <= bm->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    }

    /* bounding */
    if (k2 >= bm->numKeyFrame)
        k2 = bm->numKeyFrame - 1;
    if (k2 <= 1)
        k1 = 0;
    else
        k1 = k2 - 1;

    /* store the last key frame for next call */
    mc->lastKey = k1;

    /* get the pos/rot at each key frame */
    float time1 = bm->keyFrameList[k1].keyFrame;
    float time2 = bm->keyFrameList[k2].keyFrame;
    keyFrameForInterpolation = &(bm->keyFrameList[k2]);

    btVector3 pos1, pos2;
    btQuaternion rot1, rot2;
    if (m_overrideFirst && (k1 == 0 || time1 <= m_lastLoopStartFrame)) {
        if (bm->numKeyFrame > 1 && time2 < m_lastLoopStartFrame + 60.0f) {
            /* when this motion has more than two key frames and the next key frame is nearer than 60 frames, replace the pos/rot at first frame of the motion with the snapped ones */
            time1 = m_lastLoopStartFrame;
            pos1 = mc->snapPos;
            rot1 = mc->snapRot;
            pos2 = bm->keyFrameList[k2].pos;
            rot2 = bm->keyFrameList[k2].rot;
        } else if (frameNow - time1 < m_noBoneSmearFrame) {
            /* when this motion has only one key frame at the first frame, or has more than two key frames and the second key frame is further than 60 frames, replace the first frame with snap and go to the original pos/rot specified at the motion within the kBoneStartMarginFrame frames */
            time1 = m_lastLoopStartFrame;
            time2 = m_lastLoopStartFrame + m_noBoneSmearFrame;
            frame = frameNow;
            pos1 = mc->snapPos;
            rot1 = mc->snapRot;
            pos2 = bm->keyFrameList[k1].pos;
            rot2 = bm->keyFrameList[k1].rot;
            keyFrameForInterpolation = &(bm->keyFrameList[k1]);
        } else if (bm->numKeyFrame > 1) {
            /* when this motion has more than two key frames and 60 frames has been passed, we apply motion as if we have a virtual key frame at the 60th frame which has the same value as the original 0th frame */
            time1 = m_lastLoopStartFrame + m_noBoneSmearFrame;
            frame = frameNow;
            pos1 = bm->keyFrameList[k1].pos;
            rot1 = bm->keyFrameList[k1].rot;
            pos2 = bm->keyFrameList[k2].pos;
            rot2 = bm->keyFrameList[k2].rot;
        } else {
            /* when this motion has only one key frame and 60 frames has been passed, just use the original pos/rot */
            pos1 = bm->keyFrameList[k1].pos;
            rot1 = bm->keyFrameList[k1].rot;
        }
    } else {
        pos1 = bm->keyFrameList[k1].pos;
        rot1 = bm->keyFrameList[k1].rot;
        pos2 = bm->keyFrameList[k2].pos;
        rot2 = bm->keyFrameList[k2].rot;
    }

    /* calculate the position and rotation */
    if (time1 != time2) {
        if (frame <= time1) {
            mc->pos = pos1;
            mc->rot = rot1;
        } else if (frame >= time2) {
            mc->pos = pos2;
            mc->rot = rot2;
        } else {
            float x = 0, y = 0, z = 0, ww = 0;
            /* lerp */
            float w = (frame - time1) / (time2 - time1);
            if (keyFrameForInterpolation->linear[0]) {
                x = pos1.x() * (1.0f - w) + pos2.x() * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[0][(int)(w * VMD::kInterpolationTableSize)];
                x = pos1.x() * (1.0f - ww) + pos2.x() * ww;
            }
            if (keyFrameForInterpolation->linear[1]) {
                y = pos1.y() * (1.0f - w) + pos2.y() * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[1][(int)(w * VMD::kInterpolationTableSize)];
                y = pos1.y() * (1.0f - ww) + pos2.y() * ww;
            }
            if (keyFrameForInterpolation->linear[2]) {
                z = pos1.z() * (1.0f - w) + pos2.z() * w;
            } else {
                ww = keyFrameForInterpolation->interpolationTable[2][(int)(w * VMD::kInterpolationTableSize)];
                z = pos1.z() * (1.0f - ww) + pos2.z() * ww;
            }
            mc->pos.setValue(x, y, z);
            if (keyFrameForInterpolation->linear[3]) {
                mc->rot = rot1.slerp(rot2, w);
            } else {
                ww = keyFrameForInterpolation->interpolationTable[3][(int)(w * VMD::kInterpolationTableSize)];
                mc->rot = rot1.slerp(rot2, ww);
            }
        }
    } else {
        /* both keys have the same time, just apply one of them */
        mc->pos = pos1;
        mc->rot = rot1;
    }
}

void MotionController::calcFaceAt(MotionControllerFaceElement *mc, float frameNow)
{
    FaceMotion *fm = mc->motion;
    float frame = frameNow;
    uint32_t k1 = 0, k2 = 0;

    /* clamp frame to the defined last frame */
    if (frame > fm->keyFrameList[fm->numKeyFrame-1].keyFrame)
        frame = fm->keyFrameList[fm->numKeyFrame-1].keyFrame;

    /* find key frames between which the given frame exists */
    if (frame >= fm->keyFrameList[mc->lastKey].keyFrame) {
        /* start searching from last used key frame */
        for (uint32_t i = mc->lastKey; i < fm->numKeyFrame; i++) {
            if (frame <= fm->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    } else {
        for (uint32_t i = 0; i <= mc->lastKey && i < fm->numKeyFrame; i++) {
            if (frame <= fm->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    }

    /* bounding */
    if (k2 >= fm->numKeyFrame)
        k2 = fm->numKeyFrame - 1;
    if (k2 <= 1)
        k1 = 0;
    else
        k1 = k2 - 1;

    /* store the last key frame for next call */
    mc->lastKey = k1;

    /* get the pos/rot at each key frame */
    float time1 = fm->keyFrameList[k1].keyFrame;
    float time2 = fm->keyFrameList[k2].keyFrame;
    float weight1 = 0, weight2 = 0;

    if (m_overrideFirst && (k1 == 0 || time1 <= m_lastLoopStartFrame)) {
        if (fm->numKeyFrame > 1 && time2 < m_lastLoopStartFrame + 60.0f) {
            /* when this motion has more than two key frames and the next key frame is nearer than 60 frames, replace the pos/rot at first frame of the motion with the snapped ones */
            time1 = m_lastLoopStartFrame;
            weight1 = mc->snapWeight;
            weight2 = fm->keyFrameList[k2].weight;
        } else if (frameNow - time1 < m_noFaceSmearFrame) {
            /* when this motion has only one key frame at the first frame, or has more than two key frames and the second key frame is further than 60 frames, replace the first frame with snap and go to the original pos/rot specified at the motion within the kBoneStartMarginFrame frames */
            time1 = m_lastLoopStartFrame;
            time2 = m_lastLoopStartFrame + m_noFaceSmearFrame;
            frame = frameNow;
            weight1 = mc->snapWeight;
            weight2 = fm->keyFrameList[k1].weight;
        } else if (fm->numKeyFrame > 1) {
            /* when this motion has more than two key frames and 60 frames has been passed, we apply motion as if we have a virtual key frame at the 60th frame which has the same value as the original 0th frame */
            time1 = m_lastLoopStartFrame + m_noFaceSmearFrame;
            weight1 = fm->keyFrameList[k1].weight;
            weight2 = fm->keyFrameList[k2].weight;
        } else {
            /* when this motion has only one key frame and 60 frames has been passed, just use the original pos/rot */
            weight1 = fm->keyFrameList[k1].weight;
        }
    } else {
        weight1 = fm->keyFrameList[k1].weight;
        weight2 = fm->keyFrameList[k2].weight;
    }

    /* get value between [time0..time1][weight1..weight2] */
    if (time1 != time2) {
        float w = (frame - time1) / (time2 - time1);
        mc->weight = weight1 * (1.0f - w) + weight2 * w;
    } else {
        mc->weight = weight1;
    }
}

void MotionController::control(float frameNow)
{
    assert(m_boneCtrlList != NULL && m_faceCtrlList != NULL);

    /* update bone positions / rotations at current frame by the correponding motion data */
    /* if blend rate is 1.0, the values will override the current bone pos/rot */
    /* otherwise, the values are blended to the current bone pos/rot */
    for (uint32_t i = 0; i < m_numBoneCtrl; i++) {
        MotionControllerBoneElement *mcb = &(m_boneCtrlList[i]);
        /* if ignore static flag is set and this motion has only one frame (= first), skip it in this controller */
        if (m_ignoreSingleMotion && mcb->motion->numKeyFrame <= 1)
            continue;
        /* calculate bone position / rotation */
        calcBoneAt(mcb, frameNow);
        /* set the calculated position / rotation to the bone */
        if (m_boneBlendRate == 1.0f) {
            /* override */
            mcb->bone->setCurrentPosition(mcb->pos);
            mcb->bone->setCurrentRotation(mcb->rot);
        } else {
            /* lerp */
            btVector3 tmpPos = mcb->bone->getCurrentPosition();
            tmpPos = tmpPos.lerp(mcb->pos, m_boneBlendRate);
            mcb->bone->setCurrentPosition(tmpPos);
            btQuaternion tmpRot = mcb->bone->getCurrentRotation();
            tmpRot = tmpRot.slerp(mcb->rot, m_boneBlendRate);
            mcb->bone->setCurrentRotation(tmpRot);
        }
    }
    /* update face weights by the correponding motion data */
    /* unlike bones, the blend rate is ignored, and values will override the current face weight */
    for (uint32_t i = 0; i < m_numFaceCtrl; i++) {
        MotionControllerFaceElement *mcf = &(m_faceCtrlList[i]);
        /* if ignore static flag is set and this motion has only one frame (= first), skip it in this controller */
        if (m_ignoreSingleMotion && mcf->motion->numKeyFrame <= 1)
            continue;
        /* calculate face weight */
        calcFaceAt(mcf, frameNow);
        /* set the calculated weight to the face */
        if (m_faceBlendRate == 1.0f)
            mcf->face->setWeight(mcf->weight);
        else
            mcf->face->setWeight(mcf->face->getWeight() * (1.0f - m_faceBlendRate) + mcf->weight * m_faceBlendRate);
    }
}

void MotionController::takeSnap(btVector3 *centerPos)
{
    for (uint32_t i = 0; i < m_numBoneCtrl; i++) {
        MotionControllerBoneElement *mcb = &(m_boneCtrlList[i]);
        mcb->snapPos = mcb->bone->getCurrentPosition();
        if (centerPos && mcb->bone->hasMotionIndependency()) {
            /* consider center offset for snapshot */
            mcb->snapPos -= *centerPos;
        }
        mcb->snapRot = mcb->bone->getCurrentRotation();
    }
    for (uint32_t i = 0; i < m_numFaceCtrl; i++) {
        MotionControllerFaceElement *mcf = &(m_faceCtrlList[i]);
        mcf->snapWeight = mcf->face->getWeight();
    }
}

MotionController::MotionController()
    : m_maxFrame(0.0f),
    m_numBoneCtrl(0),
    m_boneCtrlList(NULL),
    m_numFaceCtrl(0),
    m_faceCtrlList(NULL),
    m_hasCenterBoneMotion(false),
    m_boneBlendRate(1.0f),
    m_faceBlendRate(1.0f),
    m_ignoreSingleMotion(false),
    m_currentFrame(0.0),
    m_previousFrame(0.0),
    m_lastLoopStartFrame(0.0f),
    m_noBoneSmearFrame(kBoneStartMarginFrame),
    m_noFaceSmearFrame(kFaceStartMarginFrame),
    m_overrideFirst(false)
{
}

MotionController::~MotionController()
{
    release();
}

void MotionController::release()
{
    MMDAIMemoryRelease(m_boneCtrlList);
    MMDAIMemoryRelease(m_faceCtrlList);

    m_maxFrame = 0.0f;
    m_boneCtrlList = NULL;
    m_numFaceCtrl = 0;
    m_faceCtrlList = NULL;
    m_hasCenterBoneMotion = false;
    m_boneBlendRate = 1.0f;
    m_faceBlendRate = 1.0f;
    m_ignoreSingleMotion = false;
    m_currentFrame = 0.0;
    m_previousFrame = 0.0;
    m_lastLoopStartFrame = 0.0f;
    m_noBoneSmearFrame = kBoneStartMarginFrame;
    m_noFaceSmearFrame = kFaceStartMarginFrame;
    m_overrideFirst = false;
}

void MotionController::setup(PMDModel *pmd, VMD *vmd)
{
    release();

    /* store maximum frame len */
    m_maxFrame = vmd->getMaxFrame();

    /* allocate bone controller */
    m_numBoneCtrl = vmd->countBoneKind();
    uint32_t nbones = pmd->countBones();
    if (m_numBoneCtrl > nbones) /* their maximum will be smaller one between pmd and vmd */
        m_numBoneCtrl = nbones;
    m_boneCtrlList = static_cast<MotionControllerBoneElement *>(MMDAIMemoryAllocate(sizeof(MotionControllerBoneElement) * m_numBoneCtrl));
    if (m_boneCtrlList == NULL) {
        MMDAILogWarnString("cannot allocate memory");
        return;
    }

    /* check all bone definitions in vmd to match the pmd, and store if match */
    m_numBoneCtrl = 0;
    const char *centerBoneName = getCenterBoneName();
    for (BoneMotionLink *bmlink = vmd->getBoneMotionLink(); bmlink; bmlink = bmlink->next) {
        BoneMotion *bm = &(bmlink->boneMotion);
        PMDBone *bone = pmd->getBone(bm->name);
        if (bone != NULL) {
            m_boneCtrlList[m_numBoneCtrl].bone = bone;
            m_boneCtrlList[m_numBoneCtrl].motion = bm;
            m_numBoneCtrl++;
            if (bm->numKeyFrame > 1 && MMDAIStringEquals(bm->name, centerBoneName)) {
                /* This motion has more than 1 key frames for Center Bone, so need re-location */
                m_hasCenterBoneMotion = true;
            }
        }
    }

    /* allocate face controller */
    m_numFaceCtrl = vmd->countFaceKind();
    uint32_t nfaces = pmd->countFaces();
    if (m_numFaceCtrl > nfaces) /* their maximum will be smaller one between pmd and vmd */
        m_numFaceCtrl = nfaces;
    m_faceCtrlList = static_cast<MotionControllerFaceElement *>(MMDAIMemoryAllocate(sizeof(MotionControllerFaceElement) * m_numFaceCtrl));
    if (m_faceCtrlList == NULL) {
        MMDAILogWarnString("cannot allocate memory");
        return;
    }

    /* check all face definitions in vmd to match the pmd, and store if match */
    m_numFaceCtrl = 0;
    for (FaceMotionLink *fmlink = vmd->getFaceMotionLink(); fmlink; fmlink = fmlink->next) {
        FaceMotion *fm = &(fmlink->faceMotion);
        PMDFace *face = pmd->getFace(fm->name);
        if (face != NULL) {
            m_faceCtrlList[m_numFaceCtrl].face = face;
            m_faceCtrlList[m_numFaceCtrl].motion = fm;
            m_numFaceCtrl++;
        }
    }
}

void MotionController::reset()
{
    assert(m_boneCtrlList != NULL && m_faceCtrlList != NULL);

    for (uint32_t i = 0; i < m_numBoneCtrl; i++)
        m_boneCtrlList[i].lastKey = 0;
    for (uint32_t i = 0; i < m_numFaceCtrl; i++)
        m_faceCtrlList[i].lastKey = 0;

    m_currentFrame = 0.0;
    m_previousFrame = 0.0;
    m_lastLoopStartFrame = 0.0f;
    m_noBoneSmearFrame = kBoneStartMarginFrame;
    m_noFaceSmearFrame = kFaceStartMarginFrame;
    m_boneBlendRate = 1.0f;
    m_faceBlendRate = 1.0f;
    m_ignoreSingleMotion = false;
}

bool MotionController::advance(double deltaFrame)
{
    assert(m_boneCtrlList != NULL && m_faceCtrlList != NULL);

    /* apply motion at current frame to bones and faces */
    control((float) m_currentFrame);

    /* advance the current frame count */
    /* store the last frame to m_previousFrame */
    m_previousFrame = m_currentFrame;
    m_currentFrame += deltaFrame;
    if (m_currentFrame >= m_maxFrame) {
        /* we have reached the last key frame of this motion */
        /* clamp the frame to the maximum */
        m_currentFrame = m_maxFrame;
        /* return finished status */
        return true;
    }
    return false;
}

void MotionController::rewind(float targetFrame, float frame)
{
    /* rewind current frame */
    m_currentFrame = m_previousFrame + frame - m_maxFrame + targetFrame;
    m_previousFrame = targetFrame;
    if (m_overrideFirst) {
        /* take a snap shot of current pose to be used as initial status of this motion at frame 0 */
        takeSnap(NULL); /* not move the center position */
        m_lastLoopStartFrame = targetFrame;
        if (m_maxFrame >= kBoneStartMarginFrame) {
            m_noBoneSmearFrame = kBoneStartMarginFrame;
        } else {
            m_noBoneSmearFrame -= m_maxFrame + 1.0f;
            if (m_noBoneSmearFrame < 0.0f)
                m_noBoneSmearFrame = 0.0f;
        }
        if (m_maxFrame >= kFaceStartMarginFrame) {
            m_noFaceSmearFrame = kFaceStartMarginFrame;
        } else {
            m_noFaceSmearFrame -= m_maxFrame + 1.0f;
            if (m_noFaceSmearFrame < 0.0f)
                m_noFaceSmearFrame = 0.0f;
        }
    }
}

void MotionController::setOverrideFirst(const btVector3 &centerPos)
{
    /* take snapshot of current pose, to be used as initial values at frame 0 */
    btVector3 centerPosPtr = centerPos;
    takeSnap(&centerPosPtr);
    /* tell controller that we have snapshot, and should take snap at loop */
    m_overrideFirst = true;
    m_noBoneSmearFrame = kBoneStartMarginFrame;
    m_noFaceSmearFrame = kFaceStartMarginFrame;
}

} /* namespace */

