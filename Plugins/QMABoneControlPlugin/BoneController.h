#ifndef BONECONTROLLER_H
#define BONECONTROLLER_H

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

#include "btBulletDynamicsCommon.h"

#include "MMDFiles.h"

#define MAXBONEPERCONTROL 10

/* BoneControlDef: bone control definition */
struct BoneControlDef {
  char name[21];                        /* name */
  char boneName[MAXBONEPERCONTROL][21]; /* target bone name */
  unsigned short boneNum;               /* number of target name */
  float rateOn;                         /* speed rate when switch is on */
  float rateOff;                        /* speed rate when switch is off */
  btVector3 baseVector;                 /* normalized base vector */
  btVector3 LowerAngularLimit;          /* lower angular limit */
  btVector3 UpperAngularLimit;          /* upper angular limit */
  btVector3 adjustPosition;             /* offset to adjust target position */

  BoneControlDef() {
    name[0] = 0;
  }
};

/* BoneController: control bone */
struct BoneController {
  BoneControlDef *def;
  unsigned short boneNum;
  PMDBone *bone[MAXBONEPERCONTROL];
  PMDBone **childBoneList;
  unsigned short childBoneNum;
  bool enabled;
  btQuaternion rot[MAXBONEPERCONTROL];
  float fadingRate;

  BoneController()
    : def(NULL),
    boneNum(0),
    childBoneList(NULL),
    childBoneNum(0),
    enabled(false),
    fadingRate(0.0)
  {
  }

  ~BoneController() {
    if (childBoneList) free(childBoneList);
  }

  void initialize(PMDModel *pmd, BoneControlDef *defArg) {
    if (defArg) def = defArg;

    /* get list of child bones to be updated */
    if (childBoneList) {
      free(childBoneList);
      childBoneList = NULL;
    }

    if (! def) {
      boneNum = 0;
      childBoneNum = 0;
      reset();
      return;
    }

    int n = 0;
    /* look for target bones */
    for (int i = 0; i < def->boneNum; i++) {
      PMDBone *b = pmd->getBone(def->boneName[i]);
      if (b) bone[n++] = b;
    }
    boneNum = n;

    childBoneNum = pmd->getNumBone();
    if (childBoneNum > 0) {
      childBoneList = (PMDBone **) malloc(sizeof(PMDBone *) * childBoneNum);
      int ret = pmd->getChildBoneList(bone, boneNum, childBoneList, childBoneNum);
      if (ret < 0) {
        free(childBoneList);
        childBoneList = NULL;
        childBoneNum = 0;
      } else {
        childBoneNum = ret;
        /* remove simulated bones */
        int n = 0;
        for (int i = 0; i < childBoneNum; i++) {
          if (! childBoneList[i]->isSimulated()) {
            if (n != i) childBoneList[n] = childBoneList[i];
            n++;
          }
        }
        childBoneNum = n;
      }
    }

    reset();
  }
  void reset() {
    enabled = false;
    fadingRate = 0.0f;
  }
  void enable() {
    enabled = true;
    for (int i = 0; i < boneNum; i++)
      rot[i] = (*(bone[i]->getCurrentRotation()));
  }
  void disable() {
    enabled = false;
    fadingRate = 1.0f;
  }
  void update(btVector3 *lookAtPos, float deltaFrame) {
    if (enabled) {
      /* increase rate */
      float rate = def->rateOn * deltaFrame;
      if (rate > 1.0f) rate = 1.0f;
      if (rate < 0.0f) rate = 0.0f;
      /* set offset of target position */
      btVector3 v = *lookAtPos + def->adjustPosition;
      for (int i = 0; i < boneNum; i++) {
        /* calculate rotation to target position */
        btVector3 localDest = bone[i]->getTransform()->inverse() * v;
        localDest.normalize();
        float dot = def->baseVector.dot(localDest);
        if (dot > 1.0f) continue;
        float angle = acosf(dot);
        btVector3 axis = def->baseVector.cross(localDest);
        if (axis.length2() < 0.0000001f) continue;
        axis.normalize();
        btQuaternion targetRot = btQuaternion(axis, btScalar(angle));
        /* set limit of rotation */
        btScalar x, y, z;
        btMatrix3x3 mat;
        mat.setRotation(targetRot);
        mat.getEulerZYX(z, y, x);
        if (x > def->UpperAngularLimit.x()) x = def->UpperAngularLimit.x();
        if (y > def->UpperAngularLimit.y()) y = def->UpperAngularLimit.y();
        if (z > def->UpperAngularLimit.z()) z = def->UpperAngularLimit.z();
        if (x < def->LowerAngularLimit.x()) x = def->LowerAngularLimit.x();
        if (y < def->LowerAngularLimit.y()) y = def->LowerAngularLimit.y();
        if (z < def->LowerAngularLimit.z()) z = def->LowerAngularLimit.z();
        targetRot.setEulerZYX(z, y, x);
        /* slerp from current rotation to target rotation */
        rot[i] = rot[i].slerp(targetRot, rate);
        /* set result to current rotation */
        bone[i]->setCurrentRotation(&rot[i]);
      }
      /* update bone transform matrices */
      for (int i = 0; i < boneNum; i++) bone[i]->update();
      for (int i = 0; i < childBoneNum; i++) childBoneList[i]->update();
    } else {
      btQuaternion tmpRot;
      /* spin target bone slowly */
      if (fadingRate > 0.0f) {
        /* decrement rate */
        fadingRate -= def->rateOff * deltaFrame;
        if (fadingRate < 0.0f) fadingRate = 0.0f;
        /* rate multiplication for bone rotation */
        for (int i = 0; i < boneNum; i++) {
          tmpRot = (*(bone[i]->getCurrentRotation()));
          rot[i] = tmpRot.slerp(rot[i], fadingRate);
          bone[i]->setCurrentRotation(&rot[i]);
        }
        /* update bone transform matrices */
        for (int i = 0; i < boneNum; i++) bone[i]->update();
        for (int i = 0; i < childBoneNum; i++) childBoneList[i]->update();
      }
    }
  }
};

#endif // BONECONTROLLER_H
