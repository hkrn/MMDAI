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

#ifndef MMDME_PMDRIGIDBODY_H_
#define MMDME_PMDRIGIDBODY_H_

#include <btBulletDynamicsCommon.h>

#include "MMDME/Common.h"
#include "MMDME/PMDBone.h"
#include "MMDME/PMDFile.h"

namespace MMDAI {

/* PMDRigidBody: rigid body */
class PMDRigidBody
{
private:

   btCollisionShape *m_shape;    /* collision shape */
   btRigidBody *m_body;          /* rigid body */
   btMotionState *m_motionState; /* motion state */
   unsigned short m_groupID;     /* collition group ID */
   unsigned short m_groupMask;   /* collision group mask */

   unsigned char m_type;                  /* control type: 0: kinematics, 1: simulated, 2: simulated+aligned */
   PMDBone *m_bone;                       /* associated bone */
   bool m_noBone;                         /* true if this bone will be affected from/to the movement of assigned bone */
   btTransform m_trans;                   /* local transform of position and rotation, local to associated bone */
   btTransform m_transInv;                /* inverse of m_trans */
   btMotionState *m_kinematicMotionState; /* kinematic motion state for static moving */

   btDiscreteDynamicsWorld *m_world; /* pointer to the simulation world where this rigid body exists */

   /* initialize: initialize PMDRigidBody */
   void initialize();

   /* clear: free PMDRigidBody */
   void clear();

   MMDME_DISABLE_COPY_AND_ASSIGN(PMDRigidBody);

public:

   /* PMDRigidBody: constructor */
   PMDRigidBody();

   /* ~PMDRigidBody: destructor */
   ~PMDRigidBody();

   /* setup: initialize and setup PMDRigidBody */
   bool setup(PMDFile_RigidBody *rb, PMDBone *bone);

   /* joinWorld: add the body to simulation world */
   void joinWorld(btDiscreteDynamicsWorld *btWorld);

   /* applyTransformToBone: apply the current rigid body transform to bone after simulation (for type 1 and 2) */
   void applyTransformToBone();

   /* setKinematic: switch between Default and Kinematic body for non-simulated movement */
   void setKinematic(bool flag);

   /* getBody: get rigid body */
   btRigidBody *getBody() const;
};

} /* namespace */

#endif

