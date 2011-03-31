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

#ifndef MMDAI_PMDOBJECT_H_
#define MMDAI_PMDOBJECT_H_

#include <MMDME/BulletPhysics.h>
#include <MMDME/Common.h>

#include "MMDAI/LipSync.h"

#define PMDOBJECT_MINSPINDIFF   0.000001f

namespace MMDAI {

class LipSyncLoader;
class MotionManager;
class PMDBone;
class PMDModel;
class PMDModelLoader;
class SceneRenderEngine;
class VMD;

class PMDObject
{
public:
    PMDObject(SceneRenderEngine *engine);
    ~PMDObject();

    void release();
    bool load(PMDModelLoader *modelLoader,
              LipSyncLoader *lipSyncLoader,
              btVector3 *offsetPos,
              btQuaternion *offsetRot,
              bool forcedPosition,
              PMDBone *assignBone,
              PMDObject *assignObject,
              BulletPhysics *bullet,
              bool useCartoonRendering,
              float cartoonEdgeWidth,
              btVector3 *light);
    bool startMotion(VMD *vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRepos);
    bool swapMotion(VMD *vmd, const char *targetName);
    void updateRootBone();
    bool updateMotion(double deltaFrame);
    void updateAfterSimulation(bool physicsEnabled);
    bool updateAlpha(double deltaFrame);
    void startDisappear();
    void setLightForToon(btVector3 *v);
    bool updateModelRootOffset(float fps);
    bool updateModelRootRotation(float fps);
    const char *getAlias() const;
    void setAlias(const char *alias);
    PMDModel *getPMDModel();
    MotionManager *getMotionManager() const;
    void resetMotionManager();
    bool createLipSyncMotion(const char *str, unsigned char **data, size_t *size);
    void getCurrentPosition(btVector3 &pos);
    void getTargetPosition(btVector3 &pos);
    void setPosition(btVector3 &pos);
    void getCurrentRotation(btQuaternion &rot);
    void getTargetRotation(btQuaternion &rot);
    void setRotation(btQuaternion &rot);
    void setMoveSpeed(float speed);
    void setSpinSpeed(float speed);
    bool isMoving() const;
    bool isRotating() const;
    bool isTurning() const;
    void setTurningFlag(bool flag);
    bool isEnable() const;
    void setEnableFlag(bool flag);
    bool allowMotionFileDrop() const;
    PMDObject *getAssignedModel() const;
    void renderDebug();

private:
    void initialize();
    void clear();

    char *m_alias;
    PMDModel *m_model;
    MotionManager *m_motionManager;
    LipSync *m_globalLipSync;
    LipSync m_localLipSync;
    SceneRenderEngine *m_engine;
    bool m_isEnable;
    btVector3 m_lightDir;
    PMDObject *m_assignTo;
    PMDBone *m_baseBone;
    btVector3 m_origBasePos;
    btVector3 m_offsetPos;
    btQuaternion m_offsetRot;
    bool m_absPosFlag[3];
    float m_moveSpeed;
    float m_spinSpeed;
    bool m_allowToonShading;
    bool m_allowMotionFileDrop;
    bool m_isMoving;
    bool m_isRotating;
    bool m_underTurn;
    double m_alphaAppearFrame;
    double m_alphaDisappearFrame;
    double m_displayCommentFrame;
    bool m_needResetKinematic;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDObject);
};

} /* namespace */

#endif // PMDOBJECT_H

