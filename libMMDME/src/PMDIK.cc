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

#include "MMDME/PMDIK.h"

const float PMDIK::kPI          = 3.1415926f;
const float PMDIK::kMinDistance = 0.0001f;
const float PMDIK::kMinAngle    = 0.00000001f;
const float PMDIK::kMinAxis     = 0.0000001f;
const float PMDIK::kMinRotSum   = 0.002f;
const float PMDIK::kMinRotation = 0.00001f;

/* PMDIK::initialize: initialize IK */
void PMDIK::initialize()
{
   m_destBone = NULL;
   m_targetBone = NULL;
   m_boneList = NULL;
   m_numBone = 0;
   m_iteration = 0;
   m_angleConstraint = 0.0f;
}

/* PMDIK::clear: free IK */
void PMDIK::clear()
{
   if (m_boneList)
      free(m_boneList);
   initialize();
}

/* PMDIK::PMDIK: constructor */
PMDIK::PMDIK()
{
   initialize();
}

/* PMDIK::~PMDIK: destructor */
PMDIK::~PMDIK()
{
   clear();
}

/* PMDIK::setup: initialize and setup IK  */
void PMDIK::setup(PMDFile_IK *ik, short *ikBoneIDList, PMDBone *boneList)
{
   unsigned char i;

   clear();

   m_destBone = &(boneList[ik->destBoneID]);
   m_targetBone = &(boneList[ik->targetBoneID]);
   m_numBone = ik->numLink;
   if (m_numBone) {
      m_boneList = (PMDBone **) malloc(sizeof(PMDBone *) * m_numBone);
      for (i = 0; i < m_numBone; i++)
         m_boneList[i] = &(boneList[ikBoneIDList[i]]);
   }
   m_iteration = ik->numIteration;
   m_angleConstraint = ik->angleConstraint * kPI;
}

/* PMDIK::solve: try to move targetBone toward destBone, solving constraint among bones in boneList[] and the targetBone */
void PMDIK::solve()
{
   short i;
   unsigned char j;
   unsigned short ite;
   btQuaternion tmpRot;

   btQuaternion origTargetRot;

   btVector3 destPos; /* destination position */
   btVector3 targetPos;

   btTransform tr;
   btVector3 localDestVec;
   btVector3 localTargetVec;

   float angle, dot;

   btVector3 axis;
   btQuaternion rot;

   btScalar x, y, z;
   btScalar cx, cy, cz;
   btMatrix3x3 mat;

   if (m_boneList == NULL)
      return;

   /* get the global destination point */
   destPos = m_destBone->getTransform()->getOrigin();

   /* before begin IK iteration, make sure all the child bones and target bone are up to date update from root to child */
   for (i = m_numBone - 1; i >= 0; i--)
      m_boneList[i]->update();
   m_targetBone->update();

   /* save the current rotation of the target bone. It will be restored at the end of this function */
   origTargetRot = (*(m_targetBone->getCurrentRotation()));

   /* begin IK iteration */
   for (ite = 0; ite < m_iteration; ite++) {
      /* solve each step from leaf bone to root bone */
      for (j = 0; j < m_numBone; j++) {
         /* get current global target bone location */
         targetPos = m_targetBone->getTransform()->getOrigin();
         /* calculate local positions of destination position and target position at current bone */
         tr = m_boneList[j]->getTransform()->inverse();
         localDestVec = tr * destPos;
         localTargetVec = tr * targetPos;
         /* exit if they are close enough */
         if (localDestVec.distance2(localTargetVec) < kMinDistance) {
            ite = m_iteration;
            break;
         }
         /* normalize vectors */
         localDestVec.normalize();
         localTargetVec.normalize();
         /* get angle */
         dot = localDestVec.dot(localTargetVec);
         if (dot > 1.0f) /* assume angle = 0.0f, skip to next bone */
            continue;
         angle = acosf(dot);
         /* if angle is small enough, skip to next bone */
         if (fabsf(angle) < kMinAngle)
            continue;
         /* limit angle per step */
         if (angle < - m_angleConstraint)
            angle = -m_angleConstraint;
         else if (angle > m_angleConstraint)
            angle = m_angleConstraint;
         /* get rotation axis */
         axis = localTargetVec.cross(localDestVec);
         /* if the axis is too small (= direction of destination and target is so close) and this is not a first iteration, skip to next bone */
         if (axis.length2() < kMinAxis && ite > 0)
            continue;
         /* normalize rotation axis */
         axis.normalize();
         /* create quaternion for this step, to rotate the target point to the goal point, from the axis and angle */
         rot = btQuaternion(axis, btScalar(angle));
         /* if this bone has limitation for rotation, consult the limitation */
         if (m_boneList[j]->isLimitAngleX()) {
            if (ite == 0) {
               /* When this is the first iteration, we force rotating to the maximum angle toward limited direction. This will help convergence the whole IK step earlier for most of models, especially for legs. */
               if (angle < 0.0f)
                  angle = - angle;
               rot = btQuaternion(btVector3(1.0f, 0.0f, 0.0f), btScalar(angle));
            } else {
               /* get euler angles of this rotation */
               mat.setRotation(rot);
               mat.getEulerZYX(z, y, x);
               /* get euler angles of current bone rotation (specified by the motion) */
               tmpRot = (*(m_boneList[j]->getCurrentRotation()));
               mat.setRotation(tmpRot);
               mat.getEulerZYX(cz, cy, cx);
               /* y and z should be zero, x should be over 0 */
               if (x + cx > kPI)
                  x = kPI - cx;
               if (kMinRotSum > x + cx)
                  x = kMinRotSum - cx;
               /* apply the rotation limit factor */
               if (x < -m_angleConstraint)
                  x = -m_angleConstraint;
               if (x > m_angleConstraint)
                  x = m_angleConstraint;
               /* if rotation becomes minimal by the limitation, skip to next bone */
               if (fabsf(x) < kMinRotation)
                  continue;
               /* get rotation quaternion from the limited euler angles */
               rot.setEulerZYX(0.0f, 0.0f, x);
            }
            /* apply the limited rotation to current bone */
            tmpRot = (*(m_boneList[j]->getCurrentRotation()));
            tmpRot = rot * tmpRot;
            m_boneList[j]->setCurrentRotation(&tmpRot);
         } else {
            /* apply the rotation to current bone */
            tmpRot = (*(m_boneList[j]->getCurrentRotation()));
            tmpRot *= rot;
            m_boneList[j]->setCurrentRotation(&tmpRot);
         }

         /* update transform matrices for relevant (child) bones */
         for (i = j; i >= 0; i--) m_boneList[i]->update();
         m_targetBone->update();
      }
   }

   /* restore the original rotation of the target bone */
   m_targetBone->setCurrentRotation(&origTargetRot);
   m_targetBone->update();
}

/* PMDIK::isSimulated: check if this IK is under simulation, in case no need to calculate this IK */
bool PMDIK::isSimulated() const
{
   return m_boneList[0]->isSimulated();
}
