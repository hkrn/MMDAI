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

#ifndef MMDME_PMDBONE_H_
#define MMDME_PMDBONE_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/PMDFile.h"

#define PMDBONE_KNEENAME "ひざ"

#define PMDBONE_ADDITIONALROOTNAME  "全ての親", "両足オフセ", "右足オフセ", "左足オフセ"
#define PMDBONE_NADDITIONALROOTNAME 4

/* PMDBone: bone of PMD */
class PMDBone
{
private:

   /* defined data */
   char *m_name;               /* bone name */
   PMDBone *m_parentBone;      /* parent bone (NULL = none) */
   PMDBone *m_childBone;       /* child bone (NULL = none) or co-rotate bone if type == 9 */
   unsigned char m_type;       /* bone type (PMD_BONE_TYPE) */
   PMDBone *m_targetBone;      /* bone ID by which this bone if affected: IK bone (type 4), under_rotate bone (type 5) */
   btVector3 m_originPosition; /* position from origin, defined in model (absolute) */
   float m_rotateCoef;         /* Effect coef. if type == corotate */

   /* definitions extracted at startup */
   btVector3 m_offset;       /* Offset position from parent bone */
   bool m_parentIsRoot;      /* true if parent is root bone, otherwise false */
   bool m_limitAngleX;       /* true if this bone can be bended for X axis only at IK process */
   bool m_motionIndependent; /* true if this bone is not affected by other controller bones */

   /* work area */
   btTransform m_trans;             /* current transform matrix, computed from m_pos and m_rot */
   btTransform m_transMoveToOrigin; /* transform to move position to origin, for skinning */
   bool m_simulated;                /* true if this bone is controlled under physics */
   btVector3 m_pos;                 /* current position from parent bone, given by motion */
   btQuaternion m_rot;              /* current rotation, given by motion */

private:

   /* initialize: initialize bone */
   void initialize();

   /* clear: free bone */
   void clear();

public:

   /* PMDBone: constructor */
   PMDBone();

   /* ~PMDBone: destructor */
   ~PMDBone();

   /* setup: initialize and setup bone */
   bool setup(PMDFile_Bone *b, PMDBone *boneList, unsigned short maxBones, PMDBone *rootBone);

   /* computeOffset: compute offset position */
   void computeOffset();

   /* reset: reset working pos and rot */
   void reset();

   /* setMotionIndependency: check if this bone does not be affected by other controller bones */
   void setMotionIndependency();

   /* updateRotate: update internal transform, consulting extra rotation */
   void updateRotate();

   /* update: update internal transform for current position / rotation */
   void update();

   /* calcSkinningTrans: get internal transform for skinning */
   void calcSkinningTrans(btTransform *b);

   /* getName: get bone name */
   char *getName();

   /* getType: get bone type */
   unsigned char getType();

   /* getTransform: get transform */
   btTransform *getTransform();

   /* setTransform: set transform */
   void setTransform(btTransform *tr);

   /* getOriginPosition: get origin position */
   btVector3 *getOriginPosition();

   /* isLimitAngleX: return true if this bone can be bended for X axis only at IK process */
   bool isLimitAngleX();

   /* hasMotionIndependency: return true if this bone is not affected by other controller bones */
   bool hasMotionIndependency();

   /* setSimlatedFlag: set flag whether bone is controlled under phsics or not */
   void setSimulatedFlag(bool flag);

   /* isSimulated: return true if this bone is controlled under physics */
   bool isSimulated();

   /* getOffset: get offset */
   btVector3 *getOffset();

   /* setOffset: set offset */
   void setOffset(btVector3 *v);

   /* getParentBone: get parent bone */
   PMDBone *getParentBone();

   /* getCurrentPosition: get current position */
   btVector3 *getCurrentPosition();

   /* setCurrentPosition: set current position */
   void setCurrentPosition(btVector3 *v);

   /* getCurrentRotation: get current rotation */
   btQuaternion *getCurrentRotation();

   /* setCurrentRotation: set current rotation */
   void setCurrentRotation(btQuaternion *q);

   void renderDebug();
};

#endif
