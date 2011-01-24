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

#include "PMDBone.h"
#include "PMDInternal.h"

/* PMDBone::initialize: initialize bone */
void PMDBone::initialize()
{
   m_name = NULL;
   m_parentBone = NULL;
   m_childBone = NULL;
   m_type = UNKNOWN;
   m_targetBone = NULL;
   m_originPosition.setZero();
   m_rotateCoef = 0.0f;

   m_offset.setZero();
   m_parentIsRoot = false;
   m_limitAngleX = false;
   m_motionIndependent = false;

   m_trans.setIdentity();
   m_trans.setOrigin(m_originPosition);
   m_transMoveToOrigin.setIdentity();
   m_transMoveToOrigin.setOrigin(-m_originPosition);
   m_simulated = false;

   m_pos.setZero();
   m_rot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

/* PMDBone::clear: free bone */
void PMDBone::clear()
{
   if(m_name)
      free(m_name);

   initialize();
}

/* PMDBone::PMDBone: constructor */
PMDBone::PMDBone()
{
   initialize();
}

/* PMDBone::~PMDBone: destructor */
PMDBone::~PMDBone()
{
   clear();
}

/* PMDBone::setup: initialize and setup bone */
bool PMDBone::setup(PMDFile_Bone *b, PMDBone *boneList, unsigned short maxBones, PMDBone *rootBone)
{
   bool ret = true;
   char name[21];

   clear();

   /* name */
   strncpy(name, b->name, 20);
   name[20] = '\0';
   m_name = strdup(name);

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

   /* reset current transform values */
   m_trans.setOrigin(m_originPosition);

   /* set absolute position->origin transform matrix for skinning */
   m_transMoveToOrigin.setOrigin(-m_originPosition);

   return ret;
}

/* PMDBone::computeOffset: compute offset position */
void PMDBone::computeOffset()
{
   if (m_parentBone)
      m_offset = m_originPosition - m_parentBone->m_originPosition;
   else
      m_offset = m_originPosition;
}

/* PMDBone::reset: reset working pos and rot */
void PMDBone::reset()
{
   m_pos.setZero();
   m_rot = btQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
   /* default transform will be referred while loading rigid bodies in PMD... */
   m_trans.setIdentity();
   m_trans.setOrigin(m_originPosition);
}

/* PMDBone::setMotionIndependency: check if this bone does not be affected by other controller bones */
void PMDBone::setMotionIndependency()
{
   int i;
   const char *names[] = {PMDBONE_ADDITIONALROOTNAME};

   if (! m_parentBone || m_parentIsRoot) {
      /* if no parent bone in the model, return true */
      m_motionIndependent = true;
      return;
   }

   /* some models has additional model root bone or offset bones, they should be treated specially */
   for (i = 0; i < PMDBONE_NADDITIONALROOTNAME; i++) {
      if (strcmp(m_parentBone->m_name, names[i]) == 0) {
         m_motionIndependent = true;
         return;
      }
   }

   m_motionIndependent = false;
}

/* PMDBone::updateRotate: update internal transform, consulting extra rotation */
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

/* PMDBone::update: update internal transform for current position / rotation */
void PMDBone::update()
{
   m_trans.setOrigin(m_pos + m_offset);
   m_trans.setRotation(m_rot);
   if (m_parentBone)
      m_trans = m_parentBone->m_trans * m_trans;
}

/* PMDBone::calcSkinningTrans: get internal transform for skinning */
void PMDBone::calcSkinningTrans(btTransform *b)
{
   *b = m_trans * m_transMoveToOrigin;
}

/* PMDBone;:getName: get bone name */
char *PMDBone::getName()
{
   return m_name;
}

/* PMDBone::getType: get bone type */
unsigned char PMDBone::getType()
{
   return m_type;
}

/* PMDBone::getTransform: get transform */
btTransform *PMDBone::getTransform()
{
   return &m_trans;
}

/* PMDBone::setTransform: set transform */
void PMDBone::setTransform(btTransform *tr)
{
   m_trans = *tr;
}

/* PMDBone::getOriginPosition: get position */
btVector3 *PMDBone::getOriginPosition()
{
   return &m_originPosition;
}

/* PMDBone::isLimitAngleX: return true if this bone can be bended for X axis only at IK process */
bool PMDBone::isLimitAngleX()
{
   return m_limitAngleX;
}

/* PMDBone::hasMotionIndependency: return true if this bone is not affected by other controller bones */
bool PMDBone::hasMotionIndependency()
{
   return m_motionIndependent;
}

/* PMDBone::setSimlatedFlag: set flag whether bone is controlled under phsics or not */
void PMDBone::setSimulatedFlag(bool flag)
{
   m_simulated = flag;
}
/* PMDBone::isSimulated: return true if this bone is controlled under physics */
bool PMDBone::isSimulated()
{
   return m_simulated;
}

/* PMDBone::getOffset: get offset */
btVector3 *PMDBone::getOffset()
{
   return &m_offset;
}

/* PMDBone::setOffset: set offset */
void PMDBone::setOffset(btVector3 *v)
{
   m_offset = *v;
}

/* PMDBone::getParentBone: get parent bone */
PMDBone *PMDBone::getParentBone()
{
   return m_parentBone;
}

/* PMDBone::getCurrentPosition: get current position */
btVector3 *PMDBone::getCurrentPosition()
{
   return &m_pos;
}

/* PMDBone::setCurrentPosition: set current position */
void PMDBone::setCurrentPosition(btVector3 *v)
{
   m_pos = (*v);
}

/* PMDBone::getCurrentRotation: get current rotation */
btQuaternion *PMDBone::getCurrentRotation()
{
   return &m_rot;
}

/* PMDBone::setCurrentRotation: set current rotation */
void PMDBone::setCurrentRotation(btQuaternion *q)
{
   m_rot = (*q);
}

#include "GLee.h"

static void drawCube()
{
   static GLfloat vertices [8][3] = {
      { -0.5f, -0.5f, 0.5f},
      { 0.5f, -0.5f, 0.5f},
      { 0.5f, 0.5f, 0.5f},
      { -0.5f, 0.5f, 0.5f},
      { 0.5f, -0.5f, -0.5f},
      { -0.5f, -0.5f, -0.5f},
      { -0.5f, 0.5f, -0.5f},
      { 0.5f, 0.5f, -0.5f}
   };
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[0]);
   glVertex3fv(vertices[1]);
   glVertex3fv(vertices[2]);
   glVertex3fv(vertices[3]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[4]);
   glVertex3fv(vertices[5]);
   glVertex3fv(vertices[6]);
   glVertex3fv(vertices[7]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[1]);
   glVertex3fv(vertices[4]);
   glVertex3fv(vertices[7]);
   glVertex3fv(vertices[2]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[5]);
   glVertex3fv(vertices[0]);
   glVertex3fv(vertices[3]);
   glVertex3fv(vertices[6]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[3]);
   glVertex3fv(vertices[2]);
   glVertex3fv(vertices[7]);
   glVertex3fv(vertices[6]);
   glEnd();
   glBegin(GL_POLYGON);
   glVertex3fv(vertices[1]);
   glVertex3fv(vertices[0]);
   glVertex3fv(vertices[5]);
   glVertex3fv(vertices[4]);
   glEnd();
}

/* PMDBone::renderDebug: render bones for debug */
void PMDBone::renderDebug()
{
   btScalar m[16];
   btVector3 a;
   btVector3 b;

   /* do not draw IK target bones if the IK chain is under simulation */
   if (m_type == IK_TARGET && m_parentBone && m_parentBone->m_simulated) return;

   m_trans.getOpenGLMatrix(m);

   /* draw node */
   glPushMatrix();
   glMultMatrixf(m);
   if (m_type != NO_DISP) { /* do not draw invisible bone nodes */
      if (m_simulated) {
         /* under physics simulation */
         glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
         glScaled(0.1, 0.1, 0.1);
      } else {
         switch (m_type) {
         case IK_DESTINATION:
            glColor4f(0.7f, 0.2f, 0.2f, 1.0f);
            glScaled(0.25, 0.25, 0.25);
            break;
         case UNDER_IK:
            glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
            glScaled(0.15, 0.15, 0.15);
            break;
         case IK_TARGET:
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            glScaled(0.15, 0.15, 0.15);
            break;
         case UNDER_ROTATE:
         case TWIST:
         case FOLLOW_ROTATE:
            glColor4f(0.0f, 0.8f, 0.2f, 1.0f);
            glScaled(0.15, 0.15, 0.15);
            break;
         default:
            if (m_motionIndependent) {
               glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
               glScaled(0.25, 0.25, 0.25);
            } else {
               glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
               glScaled(0.15, 0.15, 0.15);
            }
            break;
         }
      }
      drawCube();
   }
   glPopMatrix();

   if (! m_parentBone) return;

   if (m_type == IK_DESTINATION) return;

   /* draw line from parent */
   glPushMatrix();
   if (m_type == NO_DISP) {
      glColor4f(0.5f, 0.4f, 0.5f, 1.0f);
   } else if (m_simulated) {
      glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
   } else if (m_type == UNDER_IK || m_type == IK_TARGET) {
      glColor4f(0.8f, 0.5f, 0.3f, 1.0f);
   } else {
      glColor4f(0.5f, 0.6f, 1.0f, 1.0f);
   }

   glBegin(GL_LINES);
   a = m_parentBone->m_trans.getOrigin();
   b = m_trans.getOrigin();
   glVertex3f(a.x(), a.y(), a.z());
   glVertex3f(b.x(), b.y(), b.z());
   glEnd();

   glPopMatrix();
}
