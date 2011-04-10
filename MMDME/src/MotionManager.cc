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

const float MotionManager::kDefaultLoopAtFrame = 0.0f;

/* MotionPlayer_initialize: initialize MotionPlayer */
static void MotionPlayer_initialize(MotionPlayer *m)
{
    m->name = NULL;

    m->vmd = NULL;

    m->onEnd = 2;
    m->priority = MotionManager::kDefaultPriority;
    m->ignoreStatic = false;
    m->loopAt = MotionManager::kDefaultLoopAtFrame;;
    m->enableSmooth = true;
    m->enableRePos = true;
    m->endingBoneBlendFrames = 20.0f;
    m->endingFaceBlendFrames = 5.0f;
    m->motionBlendRate = 1.0f;

    m->active = true;
    m->endingBoneBlend = 0.0f;
    m->endingFaceBlend = 0.0f;
    m->statusFlag = MOTION_STATUS_RUNNING;

    m->next = NULL;
}

/* MotionManager::purgeMotion: purge inactive motions */
void MotionManager::purgeMotion()
{
    MotionPlayer *tmp1 = NULL, *m = m_playerList;
    while (m) {
        if (!m->active) {
            if (tmp1)
                tmp1->next = m->next;
            else
                m_playerList = m->next;
            MotionPlayer *tmp2 = m->next;
            MMDAIMemoryRelease(m->name);
            delete m;
            m = tmp2;
        } else {
            tmp1 = m;
            m = m->next;
        }
    }
}

/* MotionManager::MotionManager: constructor */
MotionManager::MotionManager(PMDModel *pmd)
    : m_pmd(pmd),
    m_playerList(NULL),
    m_beginningNonControlledBlend(0.0f)
{
}

/* MotionManager::~MotionManager: destructor */
MotionManager::~MotionManager()
{
    release();
}

void MotionManager::release()
{
    MotionPlayer *player = m_playerList;
    while (player) {
        MotionPlayer *tmp = player->next;
        MMDAIMemoryRelease(player->name);
        delete player;
        player = tmp;
    }

    m_pmd = NULL;
    m_playerList = NULL;
    m_beginningNonControlledBlend = 0.0f;
}

/* MotionManager::startMotion start a motion */
bool MotionManager::startMotion(VMD * vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRePos)
{
    assert(vmd != NULL && name != NULL);

    /* purge inactive motion managers */
    purgeMotion();

    /* allocate new motion */
    MotionPlayer *m = new MotionPlayer();
    if (m == NULL) {
        MMDAILogWarnString("cannot allocate memory");
        return false;
    }

    MotionPlayer_initialize(m);

    m->name = MMDAIStringClone(name);
    m->onEnd = once ? 2 : 1; /* if loop is not specified, this motion will be deleted */
    m->ignoreStatic = full ? false : true;
    m->enableSmooth = enableSmooth;
    m->enableRePos = enableRePos;

    startMotionSub(vmd, m);

    /* set reset timer for bones/faces that are not controlled by the given base motion */
    if (!m->ignoreStatic)
        m_beginningNonControlledBlend = 10.0f;

    /* add this new motion to the last of the motion player list, consulting priority */
    if (m_playerList == NULL) {
        m->next = NULL;
        m_playerList = m;
    } else {
        MotionPlayer *tmp2 = m_playerList->next; /* skip the base motion */
        MotionPlayer *tmp1 = m_playerList;
        while (tmp2) {
            if (tmp2->priority > m->priority) {
                /* insert here */
                m->next = tmp2;
                tmp1->next = m;
                break;
            }
            tmp1 = tmp2;
            tmp2 = tmp2->next;
        }
        if (!tmp2) { /* append */
            m->next = NULL;
            tmp1->next = m;
        }
    }

    return true;
}

/* MotionManager::startMotionSub: initialize a motion */
void MotionManager::startMotionSub(VMD * vmd, MotionPlayer * m)
{
    assert(m_pmd != NULL);
    btVector3 offset;

    /* initialize and setup motion controller */
    m->mc.setup(m_pmd, vmd);

    /* reset values */
    m->mc.reset();

    /* base motion does treat the bones with single motion frame at 0th frame as the same as normal bones */
    m->mc.setIgnoreSingleMotion(m->ignoreStatic);

    /* reset work area */
    m->vmd = vmd;
    m->active = true;
    m->endingBoneBlend = 0.0f;
    m->endingFaceBlend = 0.0f;

    /* set model offset */
    if (m->enableSmooth) {
        offset.setZero();
        if (m->mc.hasCenter() && m->enableRePos) {
            /* when the started motion has center motion, the center position of the model will be moved to the current position */
            /* The current global position of the center bone will become the new offset of the root bone, and the local center position will be reset */
            PMDBone *rootBone = m_pmd->getRootBone();
            PMDBone *centerBone = m_pmd->getCenterBone();
            /* calculate relative origin of center bone from model root bone */
            btTransform tr = rootBone->getTransform().inverse();
            btVector3 pos = tr * centerBone->getTransform().getOrigin();
            /* get the translation vector */
            btVector3 centerPos = centerBone->getOriginPosition();
            offset = pos - centerPos;
            offset.setY(0.0f); /* Y axis should be set to zero to place model on ground */
            /* save the current pos/rot for smooth motion changing, resetting center location */
            m->mc.setOverrideFirst(&offset);
            /* add the offset to the root bone */
            btVector3 rootOffset = rootBone->getOffset();
            rootOffset += offset;
            rootBone->setOffset(rootOffset);
            rootBone->update();
        } else {
            /* save the current pos/rot for smooth motion changing */
            m->mc.setOverrideFirst(NULL) ;
        }
    }
}

/* MotionManager::swapMotion: swap a motion, keeping parameters */
bool MotionManager::swapMotion(VMD * vmd, const char * name)
{
    assert(vmd != NULL && name != NULL && m_playerList != NULL);

    /* purge inactive motion managers */
    purgeMotion();

    /* find the motion player to change */
    MotionPlayer *m = NULL;
    for (m = m_playerList; m; m = m->next)
        if (MMDAIStringEquals(m->name, name))
            break;
    if (!m)
        return false; /* not found */

    startMotionSub(vmd, m);

    /* set reset timer for bones/faces that are not controlled by the given base motion */
    if (!m->ignoreStatic)
        m_beginningNonControlledBlend = 10.0f;

    return true;
}

/* MotionManager::deleteMotion: delete a motion */
bool MotionManager::deleteMotion(const char *name)
{
    assert(name != NULL && m_playerList != NULL);

    for (MotionPlayer *m = m_playerList; m; m = m->next) {
        if (m->active && MMDAIStringEquals(m->name, name)) {
            /* enter the ending status, gradually decreasing the blend rate */
            m->endingBoneBlend = m->endingBoneBlendFrames;
            m->endingFaceBlend = m->endingFaceBlendFrames;
            return true;
        }
    }
    return false;
}

/* MotionManager::update: apply all motion players */
bool MotionManager::update(double frame)
{
    if (m_beginningNonControlledBlend > 0.0f) {
        /* if this is the beginning of a base motion, the uncontrolled bone/face will be reset */
        m_beginningNonControlledBlend -= (float) frame;
        if (m_beginningNonControlledBlend < 0.0f)
            m_beginningNonControlledBlend = 0.0f;
        m_pmd->smearAllBonesToDefault(m_beginningNonControlledBlend / 10.0f); /* from 1.0 to 0.0 in 10 frames */
    }

    /* reset status flags */
    for (MotionPlayer *m = m_playerList; m; m = m->next)
        m->statusFlag = MOTION_STATUS_RUNNING;

    /* update the whole motion (the later one will override the other one) */
    for (MotionPlayer *m = m_playerList; m; m = m->next) {
        /* skip deactivated motions */
        if (!m->active)
            continue;
        if (m->endingBoneBlend != 0.0f || m->endingFaceBlend != 0.0f) {
            /* this motion player is in ending status */
            m->mc.setBoneBlendRate(m->motionBlendRate * m->endingBoneBlend / m->endingBoneBlendFrames);
            m->mc.setFaceBlendRate(m->endingFaceBlend / m->endingFaceBlendFrames);
            /* proceed the motion */
            m->mc.advance(frame);
            /* decrement the rest frames */
            m->endingBoneBlend -= (float) frame;
            m->endingFaceBlend -= (float) frame;
            if (m->endingBoneBlend < 0.0f) m->endingBoneBlend = 0.0f;
            if (m->endingFaceBlend < 0.0f) m->endingFaceBlend = 0.0f;
            if (m->endingBoneBlend == 0.0f && m->endingFaceBlend == 0.0f) {
                /* this motion player has reached the final end point */
                /* deactivate this motion player */
                /* this will be purged at next call of purgeMotion() */
                m->active = false;
                m->statusFlag = MOTION_STATUS_DELETED; /* set return flag */
            }
        } else {
            /* this motion is in normal state */
            m->mc.setBoneBlendRate(m->motionBlendRate);
            m->mc.setFaceBlendRate(1.0f); /* does not apply blend rate for face morphs */
            /* proceed the motion */
            if (m->mc.advance(frame)) {
                /* this motion player has reached end */
                switch (m->onEnd) {
                case 0:
                    /* just keep the last pose */
                    break;
                case 1:
                    /* loop to a frame */
                    if (m->mc.getMaxFrame() != 0.0f) { /* avoid infinite event loop when motion is void */
                        m->mc.rewind(m->loopAt, (float) frame);
                        m->statusFlag = MOTION_STATUS_LOOPED;
                    }
                    break;
            case 2:
                    /* enter the ending status, gradually decreasing the blend rate */
                    m->endingBoneBlend = m->endingBoneBlendFrames;
                    m->endingFaceBlend = m->endingFaceBlendFrames;
                    break;
                }
            }
        }
    }

    /* return true when any status change has occurred within this call */
    for (MotionPlayer *m = m_playerList; m; m = m->next)
        if (m->statusFlag != MOTION_STATUS_RUNNING)
            return true;
    return false;
}

} /* namespace */

