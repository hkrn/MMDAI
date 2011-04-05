/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

/* MotionController::calcBoneAt: calculate bone pos/rot at the given frame */
void MotionController::calcBoneAt(MotionControllerBoneElement *mc, float frameNow)
{
    BoneMotion *bm = mc->motion;
    float frame = frameNow;
    unsigned long k1, k2;
    unsigned long i;
    float time1;
    float time2;
    btVector3 pos1, pos2;
    btQuaternion rot1, rot2;
    BoneKeyFrame *keyFrameForInterpolation;
    float x, y, z, ww;
    float w;

    /* clamp frame to the defined last frame */
    if (frame > bm->keyFrameList[bm->numKeyFrame-1].keyFrame)
        frame = bm->keyFrameList[bm->numKeyFrame-1].keyFrame;

    /* find key frames between which the given frame exists */
    if (frame >= bm->keyFrameList[mc->lastKey].keyFrame) {
        /* start searching from last used key frame */
        for (i = mc->lastKey; i < bm->numKeyFrame; i++) {
            if (frame <= bm->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    } else {
        for (i = 0; i <= mc->lastKey && i < bm->numKeyFrame; i++) {
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
    time1 = bm->keyFrameList[k1].keyFrame;
    time2 = bm->keyFrameList[k2].keyFrame;
    keyFrameForInterpolation = &(bm->keyFrameList[k2]);

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
            /* lerp */
            w = (frame - time1) / (time2 - time1);
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

/* MotionController::calcFaceAt: calculate face weight at the given frame */
void MotionController::calcFaceAt(MotionControllerFaceElement *mc, float frameNow)
{
    FaceMotion *fm = mc->motion;
    float frame = frameNow;
    unsigned long k1, k2;
    unsigned long i;
    float time1, time2, weight1, weight2;
    float w;

    /* clamp frame to the defined last frame */
    if (frame > fm->keyFrameList[fm->numKeyFrame-1].keyFrame)
        frame = fm->keyFrameList[fm->numKeyFrame-1].keyFrame;

    /* find key frames between which the given frame exists */
    if (frame >= fm->keyFrameList[mc->lastKey].keyFrame) {
        /* start searching from last used key frame */
        for (i = mc->lastKey; i < fm->numKeyFrame; i++) {
            if (frame <= fm->keyFrameList[i].keyFrame) {
                k2 = i;
                break;
            }
        }
    } else {
        for (i = 0; i <= mc->lastKey && i < fm->numKeyFrame; i++) {
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
    time1 = fm->keyFrameList[k1].keyFrame;
    time2 = fm->keyFrameList[k2].keyFrame;

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
        w = (frame - time1) / (time2 - time1);
        mc->weight = weight1 * (1.0f - w) + weight2 * w;
    } else {
        mc->weight = weight1;
    }
}

/* MotionController::control: set bone position/rotation and face weights according to the motion to the specified frame */
void MotionController::control(float frameNow)
{
    unsigned long i;
    MotionControllerBoneElement *mcb;
    MotionControllerFaceElement *mcf;
    btVector3 tmpPos;
    btQuaternion tmpRot;

    /* update bone positions / rotations at current frame by the correponding motion data */
    /* if blend rate is 1.0, the values will override the current bone pos/rot */
    /* otherwise, the values are blended to the current bone pos/rot */
    for (i = 0; i < m_numBoneCtrl; i++) {
        mcb = &(m_boneCtrlList[i]);
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
            tmpPos = mcb->bone->getCurrentPosition();
            tmpPos = tmpPos.lerp(mcb->pos, m_boneBlendRate);
            mcb->bone->setCurrentPosition(tmpPos);
            tmpRot = mcb->bone->getCurrentRotation();
            tmpRot = tmpRot.slerp(mcb->rot, m_boneBlendRate);
            mcb->bone->setCurrentRotation(tmpRot);
        }
    }
    /* update face weights by the correponding motion data */
    /* unlike bones, the blend rate is ignored, and values will override the current face weight */
    for (i = 0; i < m_numFaceCtrl; i++) {
        mcf = &(m_faceCtrlList[i]);
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

/* MotionController::takeSnap: take a snap shot of current model pose for smooth motion insertion / loop */
void MotionController::takeSnap(const btVector3 &centerPos)
{
    unsigned long i;
    MotionControllerBoneElement *mcb;
    MotionControllerFaceElement *mcf;

    for (i = 0; i < m_numBoneCtrl; i++) {
        mcb = &(m_boneCtrlList[i]);
        mcb->snapPos = mcb->bone->getCurrentPosition();
        if (centerPos && mcb->bone->hasMotionIndependency()) {
            /* consider center offset for snapshot */
            mcb->snapPos -= centerPos;
        }
        mcb->snapRot = mcb->bone->getCurrentRotation();
    }
    for (i = 0; i < m_numFaceCtrl; i++) {
        mcf = &(m_faceCtrlList[i]);
        mcf->snapWeight = mcf->face->getWeight();
    }
}

/* MotionController::initialize: initialize controller */
void MotionController::initialize()
{
    m_maxFrame = 0.0f;
    m_numBoneCtrl = 0;
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
}

/* MotionController::clear: free controller */
void MotionController::clear()
{
    if (m_boneCtrlList)
        MMDAIMemoryRelease(m_boneCtrlList);
    if (m_faceCtrlList)
        MMDAIMemoryRelease(m_faceCtrlList);
    initialize();
}

/* MotionController::MotionController: constructor */
MotionController::MotionController()
{
    initialize();
}

/* MotionController::~MotionController: destructor */
MotionController::~MotionController()
{
    clear();
}

/* MotionController::setup: initialize and set up controller */
void MotionController::setup(PMDModel *pmd, VMD *vmd)
{
    BoneMotionLink *bmlink;
    BoneMotion *bm;
    PMDBone *b;
    FaceMotionLink *fmlink;
    FaceMotion *fm;
    PMDFace *f;

    clear();
    m_hasCenterBoneMotion = false;
    m_overrideFirst = false;

    /* store maximum frame len */
    m_maxFrame = vmd->getMaxFrame();

    /* allocate bone controller */
    m_numBoneCtrl = vmd->countBoneKind();
    if (m_numBoneCtrl > pmd->countBones()) /* their maximum will be smaller one between pmd and vmd */
        m_numBoneCtrl = pmd->countBones();
    m_boneCtrlList = static_cast<MotionControllerBoneElement *>(MMDAIMemoryAllocate(sizeof(MotionControllerBoneElement) * m_numBoneCtrl));
    /* check all bone definitions in vmd to match the pmd, and store if match */
    m_numBoneCtrl = 0;
    const char *centerBoneName = getCenterBoneName();
    for (bmlink = vmd->getBoneMotionLink(); bmlink; bmlink = bmlink->next) {
        bm = &(bmlink->boneMotion);
        if ((b = pmd->getBone(bm->name))) {
            m_boneCtrlList[m_numBoneCtrl].bone = b;
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
    if (m_numFaceCtrl > pmd->countFaces()) /* their maximum will be smaller one between pmd and vmd */
        m_numFaceCtrl = pmd->countFaces();
    m_faceCtrlList = static_cast<MotionControllerFaceElement *>(MMDAIMemoryAllocate(sizeof(MotionControllerFaceElement) * m_numFaceCtrl));
    /* check all face definitions in vmd to match the pmd, and store if match */
    m_numFaceCtrl = 0;
    for (fmlink = vmd->getFaceMotionLink(); fmlink; fmlink = fmlink->next) {
        fm = &(fmlink->faceMotion);
        if ((f = pmd->getFace(fm->name))) {
            m_faceCtrlList[m_numFaceCtrl].face = f;
            m_faceCtrlList[m_numFaceCtrl].motion = fm;
            m_numFaceCtrl++;
        }
    }
}

/* MotionController::reset: reset values */
void MotionController::reset()
{
    unsigned long i;

    for (i = 0; i < m_numBoneCtrl; i++)
        m_boneCtrlList[i].lastKey = 0;
    for (i = 0; i < m_numFaceCtrl; i++)
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

/* MotionController::advance: advance motion controller by the given frame, return true when reached end */
bool MotionController::advance(double deltaFrame)
{
    if (m_boneCtrlList == NULL && m_faceCtrlList == NULL)
        return false;

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

/* MotionController::rewind: rewind motion controller to the given frame */
void MotionController::rewind(float targetFrame, float frame)
{
    /* rewind current frame */
    m_currentFrame = m_previousFrame + frame - m_maxFrame + targetFrame;
    m_previousFrame = targetFrame;
    if (m_overrideFirst) {
        /* take a snap shot of current pose to be used as initial status of this motion at frame 0 */
        takeSnap(btVector3()); /* not move the center position */
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

/* MotionController::setOverrideFirst: should be called at the first frame, to tell controller to take snapshot */
void MotionController::setOverrideFirst(const btVector3 &centerPos)
{
    /* take snapshot of current pose, to be used as initial values at frame 0 */
    takeSnap(centerPos);
    /* tell controller that we have snapshot, and should take snap at loop */
    m_overrideFirst = true;
    m_noBoneSmearFrame = kBoneStartMarginFrame;
    m_noFaceSmearFrame = kFaceStartMarginFrame;
}

} /* namespace */

