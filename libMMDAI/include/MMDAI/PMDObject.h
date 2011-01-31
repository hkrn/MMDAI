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

#ifndef PMDOBJECT_H
#define PMDOBJECT_H

#include <MMDME/BulletPhysics.h>
#include <MMDME/MotionManager.h>
#include <MMDME/PMDBone.h>
#include <MMDME/PMDModel.h>
#include <MMDME/PMDModelLoader.h>
#include <MMDME/VMD.h>

#include "MMDAI/LipSync.h"
#include "MMDAI/TextRenderer.h"

/* PMDObject: object of PMD */
class PMDObject
{
private:

   char *m_alias;               /* alias */
   PMDModel m_pmd;                 /* model */
   MotionManager *m_motionManager; /* motion manager */
   LipSync m_lipSync;              /* lip sync */

   bool m_isEnable; /* true if this model is enabled */

   /* status and work area */
   btVector3 m_lightDir; /* light direction for toon shading */

   /* model configuration */
   PMDObject *m_assignTo;   /* parent model when this is accessory */
   PMDBone *m_baseBone;     /* parent bone when this is accessory */
   btVector3 m_origBasePos; /* offset when position is fixed */

   btVector3 m_offsetPos;      /* root bone offset for accessory or moving */
   btQuaternion m_offsetRot;   /* root bone rotation for accessory or moving */
   bool m_absPosFlag[3];       /* absolute position flag for accessory per each axis */
   float m_moveSpeed;          /* move speed per second. if negative value, warp to m_offsetPos */
   float m_spinSpeed;          /* spin speed per second. if negative value, warp to m_offsetRot */
   bool m_allowToonShading;    /* false if deny toon rendering for accessory or stage */
   bool m_allowMotionFileDrop; /* true if allow motion file drop or all motion command */

   bool m_isMoving;   /* true when model move */
   bool m_isRotating; /* true when model spin */
   bool m_underTurn;  /* true when model turn */

   double m_alphaAppearFrame;    /* number of alpha frame when model appear */
   double m_alphaDisappearFrame; /* number of alpha frame when model disapper */
   double m_displayCommentFrame; /* number of frame to show comment ofs model */

   bool m_needResetKinematic; /* flag for reset Kinematic State when base motion is changed */

   /* PMDObject::initialize: initialize PMDObject */
   void initialize();

   /* PMDOjbect::clear: free PMDObject */
   void clear();

public:

   /* PMDObject::PMDObject: constructor */
   PMDObject();

   /* PMDObject::PMDObject: destructor */
   ~PMDObject();

   /* PMDOjbect::release: free PMDObject */
   void release();

   /* PMDObject::load: load model */
   bool load(PMDModelLoader *loader,
             btVector3 *offsetPos,
             btQuaternion *offsetRot,
             bool forcedPosition,
             PMDBone *assignBone,
             PMDObject *assignObject,
             BulletPhysics *bullet,
             bool useCartoonRendering,
             float cartoonEdgeWidth,
             btVector3 *light);

   /* PMDObject::setMotion: start a motion */
   bool startMotion(VMD *vmd, const char *name, bool full, bool once, bool enableSmooth, bool enableRepos);

   /* PMDObject::swapMotion: swap a motion */
   bool swapMotion(VMD *vmd, const char *targetName);

   /* PMDObject::updateRootBone: update root bone if assigned to a base bone */
   void updateRootBone();

   /* PMDObject::updateMotion: update motions */
   bool updateMotion(double deltaFrame);

   /* PMDObject::updateAfterSimulation: update bone transforms from simulated rigid bodies */
   void updateAfterSimulation(bool physicsEnabled);

   /* PMDObject::updateAlpha: update global model alpha */
   bool updateAlpha(double deltaFrame);

   /* PMDObject::startDisppear: set disapper timer */
   void startDisappear();

   /* PMDModel::setLightForToon: set light direction for ton shading */
   void setLightForToon(btVector3 *v);

   /* PMDObject::updateModel: update model position of root bone */
   bool updateModelRootOffset(float fps);

   /* PMDObject::updateModelRootRotation: update model rotation of root bone */
   bool updateModelRootRotation(float fps);

   /* PMDObject::getAlias: get alias name */
   const char *getAlias();

   /* PMDObject::setAlias: set alias name */
   void setAlias(const char *alias);

   /* PMDObject::getPMDModel: get PMDModel */
   PMDModel *getPMDModel();

   /* PMDObject::getMotionManager: get MotionManager */
   MotionManager *getMotionManager();

   /* PMDObject::resetMotionManager: reset MotionManager */
   void resetMotionManager();

   /* PMDObject::getLipSync: get LipSync */
   LipSync *getLipSync();

   /* PMDObject::getPosition: get root bone offset */
   void getPosition(btVector3 &pos);

   /* PMDObject::setPosition: set root bone offset */
   void setPosition(btVector3 &pos);

   /* PMDObject::getRotation: get root bone rotation */
   void getRotation(btQuaternion &rot);

   /* PMDObject::setRotation: set root bone rotation */
   void setRotation(btQuaternion &rot);

   /* PMDObject::setMoveSpeed: set move speed per second */
   void setMoveSpeed(float speed);

   /* PMDObject::setSpinSpeed: set spin seed per second */
   void setSpinSpeed(float speed);

   /* PMDObject::isMoving: return true when model move */
   bool isMoving();

   /* PMDObject::isRotating: return true when model spin */
   bool isRotating();

   /* PMDObject::isTruning: return true when model turn */
   bool isTurning();

   /* PMDObject::setTurnFlag: set turnning flag */
   void setTurningFlag(bool flag);

   /* PMDObject::isEnable: get enable flag */
   bool isEnable();

   /* PMDObject::setEnableFlag: set enable flag */
   void setEnableFlag(bool flag);

   /* PMDObject::allowMotionFileDrop: return true if motion file drop is allowed */
   bool allowMotionFileDrop();

   /* PMDObject::getAssignedModel: get parent model */
   PMDObject *getAssignedModel();

   /* renderComment: render model comment */
   void renderComment(TextRenderer *text);

   /* renderDebug: render model debug */
   void renderDebug(TextRenderer *text);

   /* renderError: render model error */
   void renderError(TextRenderer *text);
 };

#endif // PMDOBJECT_H
